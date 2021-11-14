#include "pci.h"
#include "kernel.h"
#include "ahci.h"
#include "heap.h"

#include "terminal.h"
#include "string.h"

#define BUS_DEVICE_CNT 32
#define DEVICE_FUNS_CNT 8

static const char *_device_classes[] = {
    "Unclassified",
    "Mass Storage Controller",
    "Network Controller",
    "Display Controller",
    "Multimedia Controller",
    "Memory Controller",
    "Bridge Device",
    "Simple Communication Controller",
    "Base System Peripheral",
    "Input Device Controller",
    "Docking Station", 
    "Processor",
    "Serial Bus Controller",
    "Wireless Controller",
    "Intelligent Controller",
    "Satellite Communication Controller",
    "Encryption Controller",
    "Signal Processing Controller",
    "Processing Accelerator",
    "Non Essential Instrumentation"
};
static const size_t _device_classes_cnt = sizeof(_device_classes) / sizeof(char *);

static char buf[128];

static void __enumerate_bus(uint64_t, uint64_t);
static void __enumerate_device(uint64_t, uint64_t);
static void __enumerate_function(uint64_t, uint64_t);
static const char* __mass_storage_controller_subclass(uint8_t);
static const char* __serial_bus_controller_subclass(uint8_t);
static const char* __bridge_device_subclass(uint8_t);

//todo: replace all of this clunkiness with a proper lookup mechanism, referencing pci.ids

void pci_enumerate(acpi_mcfg_header_t *mcfg)
{
    int entries = ((mcfg->sdt_header.length) - sizeof(acpi_mcfg_header_t)) / sizeof(acpi_mcfg_device_t);
    for (int i  = 0; i < entries; i++) {
        acpi_mcfg_device_t *cfg =  (acpi_mcfg_device_t *)((uint64_t)mcfg + sizeof(acpi_mcfg_header_t) + (sizeof(acpi_mcfg_device_t) * i));
        for (uint64_t bus = cfg->start_bus; bus < cfg->end_bus; bus++) {
            __enumerate_bus(cfg->base_address, bus);
        }
    }
}

const char* pci_device_class(uint8_t class_code)
{
    if (class_code >= _device_classes_cnt) return NULL;
    return _device_classes[class_code];
}

const char* pci_vendor_name(uint16_t vendor_id)
{
    switch (vendor_id) {
        case 0x8086:
            return "Intel Corp";
        case 0x1022:
            return "AMD";
        case 0x10DE:
            return "NVIDIA Corporation";
        default:
        {
            return uint16_to_hex(vendor_id, buf);
        }
    }
}

const char* pci_device_name(uint16_t vendor_id, uint16_t device_id)
{
    switch(vendor_id) {
        case 0x8086:  // Intel
        {
            switch (device_id) {
                case 0x29C0:
                    return "Express DRAM Controller";
                case 0x2918:
                    return "LPC Interface Controller";
                case 0x2922:
                    return "6 port SATA Controller [AHCI mode]";
                case 0x2930:
                    return "SMBus Controler";
            }
        }
    }
    return uint16_to_hex(device_id, buf);
}

const char* pci_subclass_name(uint8_t class_code, uint8_t subclass_code)
{
    switch(class_code) {
        case 0x01:
            return __mass_storage_controller_subclass(subclass_code);
        case 0x03:
            switch (subclass_code) {
                case 0x00:
                    return "VGA Compatible Controller";
            }
        case 0x06:
            return __bridge_device_subclass(subclass_code);
        case 0x0C:
            return __serial_bus_controller_subclass(subclass_code);
    }

    return uint8_to_hex(subclass_code, buf);
}

const char* pci_program_iface(uint8_t class_code, uint8_t subclass_code, uint8_t program_iface)
{
    switch (class_code) {
        case 0x01:
            switch (subclass_code) {
                case 0x06:
                    switch (program_iface) {
                        case 0x00:
                            return "Vendor Specific Interface";
                        case 0x01:
                            return "AHCI 1.0";
                        case 0x02:
                            return "Serial Storage Bus";
                    }
            }
        case 0x03:
            switch (subclass_code) {
                case 0x00:
                    switch (program_iface) {
                        case 0x00:
                            return "VGA Controller";
                        case 0x01:
                            return "8514-Compatible Controller";
                    }
            }
        case 0x0C:
            switch (subclass_code) {
                case 0x03:
                    switch (program_iface) {
                        case 0x00:
                            return "UHCI Controller";
                        case 0x10:
                            return "OHCI Controller";
                        case 0x20:
                            return "EHCI (USB2) Controller";
                        case 0x30:
                            return "XHCI (USB3) Controller";
                        case 0x80:
                            return "Unspecified";
                        case 0xFE:
                            return "USB Device (Not a Host Controller)";
                    }
            }
    }

    return uint8_to_hex(program_iface, buf);
}

static void __enumerate_bus(uint64_t base_address, uint64_t bus)
{
    uint64_t bus_address = base_address + (bus << 20);
    pagetable_map(g_pml4, (void *)bus_address, (void *)bus_address);
    pci_device_hdr_t *dev_hdr = (pci_device_hdr_t *)bus_address;
    if (dev_hdr->device_id == 0) return; // device not valid
    if (dev_hdr->device_id == 0xFFFF) return; // device not valid

    for (uint64_t device = 0; device < BUS_DEVICE_CNT; device++) {
        __enumerate_device(bus_address, device);
    }   
}

static void __enumerate_device(uint64_t bus_address, uint64_t device)
{
    uint64_t device_address = bus_address + (device << 15);
    pagetable_map(g_pml4, (void *)device_address, (void *)device_address);
    pci_device_hdr_t *dev_hdr = (pci_device_hdr_t *)device_address;
    if (dev_hdr->device_id == 0) return;
    if (dev_hdr->device_id == 0xFFFF) return;

    for (uint64_t function = 0; function < DEVICE_FUNS_CNT; function++) {
        __enumerate_function(device_address, function);
    }
}

static void __enumerate_function(uint64_t device_address, uint64_t function)
{
    uint64_t function_address = device_address + (function << 12);
    pagetable_map(g_pml4, (void *)function_address, (void *)function_address);
    pci_device_hdr_t *dev_hdr = (pci_device_hdr_t *)function_address;
    if (dev_hdr->device_id == 0) return;
    if (dev_hdr->device_id == 0xFFFF) return;

    char buf[128];
    terminal_print(pci_vendor_name(dev_hdr->vendor_id));
    terminal_nprint(2, " :: ", pci_device_name(dev_hdr->vendor_id, dev_hdr->device_id));
    terminal_nprint(2, " :: ", pci_device_class(dev_hdr->class_code));
    terminal_nprint(2, " :: ", pci_subclass_name(dev_hdr->class_code, dev_hdr->subclass));
    terminal_nprintln(2, " :: ", pci_program_iface(dev_hdr->class_code, dev_hdr->subclass, dev_hdr->prog_iface));

    if (dev_hdr->class_code == 0x01 && dev_hdr->subclass == 0x06 && dev_hdr->prog_iface == 0x01) {
        ahci_driver_t *ahci_driver = heap_alloc(sizeof(ahci_driver_t));
        ahci_init(ahci_driver, dev_hdr);
    }
}

static const char* __mass_storage_controller_subclass(uint8_t code)
{
    switch (code) {
        case 0x00:
            return "SCSI Bus Controller";
        case 0x01:
            return "IDE Controller";
        case 0x03:
            return "IPI Bus Controller";
        case 0x04:
            return "RAID Controller";
        case 0x05:
            return "ATA Controller";
        case 0x06:
            return "Serial ATA";
        case 0x07:
            return "Serial Attached SCSI";
        case 0x08:
            return "Non-Volatile Memory Controller";
        case 0x80:
            return "Other";
        default:
            return uint8_to_hex(code, buf);
    }
}

static const char* __serial_bus_controller_subclass(uint8_t code)
{
    switch (code) {
        case 0x00:
            return "FireWire (IEEE 1394) Controller";
        case 0x01:
            return "ACCESS Bus";
        case 0x02:
            return "SSA";
        case 0x03:
            return "USB Controller";
        case 0x04:
            return "Fibre Channel";
        case 0x05:
            return "SMBus";
        case 0x06:
            return "Infiniband";
        case 0x07:
            return "IPMI Interface";
        case 0x08:
            return "SERCOS Interface (IEC 61491)";
        case 0x09:
            return "CANBus";
        case 0x80:
            return "SerialBusController - Other";
        default:
            return uint8_to_hex(code, buf);
    }
}

static const char* __bridge_device_subclass(uint8_t code)
{
    switch(code) {
        case 0x00:
            return "Host Bridge";
        case 0x01:
            return "ISA Bridge";
        case 0x02:
            return "EISA Bridge";
        case 0x03:
            return "MCA Bridge";
        case 0x04:
            return "PCI-to-PCI Bridge";
        case 0x05:
            return "PCMCIA Bridge";
        case 0x06:
            return "NuBus Bridge";
        case 0x07:
            return "NuBus Bridge";
        case 0x08:
            return "CardBus Bridge";
        case 0x09:
            return "RACEway Bridge";
        case 0x0A:
            return "PCI-to-PCI Bridge";
        case 0x80:
            return "Other";
        default:
            return uint8_to_hex(code, buf);
    }
}
