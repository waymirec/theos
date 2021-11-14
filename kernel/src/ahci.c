#include "ahci.h"
#include "pagetable_manager.h"
#include "kernel.h"

#include "terminal.h"
#include "string.h"

static void __probe_ports(hba_mem_t *);
static AHCI_DEVICE_TYPE __check_device_type(hba_port_t *);

char buf[128];

void ahci_init(ahci_driver_t *driver, pci_device_hdr_t *pci_base_address)
{
    driver->pci_base_address = pci_base_address;
    driver->abar = (hba_mem_t *)((uint64_t)((pci_general_device_t *)pci_base_address)->base_address5);
    pagetable_map(g_pml4, (void *)(driver->abar), (void *)(driver->abar));
    
    terminal_nprintln(2, "PCI BASE: ", uint64_to_hex((uint64_t)pci_base_address, buf));
    __probe_ports(driver->abar);
}

static void __probe_ports(hba_mem_t *abar)
{
    uint32_t pi = abar->pi;
    for (int i = 0; i < 32; i++, pi >> 1) {
        if (pi & (1 << i)) {
            int dt = __check_device_type(&abar->ports[i]);
            switch (dt) {
                case AHCI_DEVICE_TYPE_SATA:
                    terminal_nprintln(2, "SATA drive found at port ", int_to_string(i, buf));
                    break;
                case AHCI_DEVICE_TYPE_SATAPI:
                    terminal_nprintln(2, "SATAPI drive found at port ", int_to_string(i, buf));
                    break;
                case AHCI_DEVICE_TYPE_SEMB:
                    terminal_nprintln(2, "SEMB drive found at port ", int_to_string(i, buf));
                    break;
                case AHCI_DEVICE_TYPE_PM:
                    terminal_nprintln(2, "PM drive found at port ", int_to_string(i, buf));
                    break;
                default:
                    terminal_nprintln(2, "No drive found at port ", int_to_string(i, buf));
                    break;
            }
        }
    }
}

static AHCI_DEVICE_TYPE __check_device_type(hba_port_t *port)
{
    uint32_t ssts = port->ssts;

    uint8_t ipm = (ssts >> 8) & 0x0F;
    uint8_t det = ssts & 0x0F;

    if (det != HBA_PORT_DET_PRESENT) return AHCI_DEVICE_TYPE_NULL;
    if (ipm != HBA_PORT_IPM_ACTIVE) return AHCI_DEVICE_TYPE_NULL;

    switch(port->sig) {
        case SATA_SIG_ATAPI:
            return AHCI_DEVICE_TYPE_SATAPI;
        case SATA_SIG_SEMB:
            return AHCI_DEVICE_TYPE_SEMB;
        case SATA_SIG_PM:
            return AHCI_DEVICE_TYPE_PM;
        default:
            return AHCI_DEVICE_TYPE_SATA;
    }
}