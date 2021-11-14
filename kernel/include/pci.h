#pragma once

#include <stdint.h>

#include "acpi.h"

typedef struct {
    uint16_t vendor_id;             // identifies the particular device
    uint16_t device_id;             // identifies the manufacturer of the device
    uint16_t command;               // provides control over device's ability to generate & respond to PCI cycles (0 = disconnected)
    uint16_t status;                // register used to record status information for PCI bus relaterd events
    uint8_t revision_id;            // revision identifier
    uint8_t prog_iface;             // read-only register that specifies a register-level programming iface the device has
    uint8_t subclass;               // read-only register that specifies the specific function the device performs
    uint8_t class_code;             // read-only register that specifies the type of function the device performs
    uint8_t cache_line_size;        // specifies the system cache line size in 32-bit units. (unsupported value == 0)
    uint8_t latency_timer;          // specifies the laency timer in units of PCI bus clocks
    uint8_t header_type;            // 0x0 = general device, 0x1 = PCI-to-PCI bridge, 0x2 = CardBus bridge
    uint8_t bist;                   // represents the status and allows control of the device built-in self-test.
} __attribute__((packed)) pci_device_hdr_t;

typedef struct {
    pci_device_hdr_t header;
    uint32_t base_address0;
    uint32_t base_address1;
    uint32_t base_address2;
    uint32_t base_address3;
    uint32_t base_address4;
    uint32_t base_address5;
    uint32_t cardbus_cis_ptr;           // CardBus CIS Pointer: Points to the Card Information Structure and is used by devices that share silicon between CardBus and PCI.
    uint16_t subsystem_vendor_id;
    uint16_t subsystem_id;
    uint32_t expansion_rom_address;
    uint8_t capabilities_ptr;           // Capabilities Pointer: Points (i.e. an offset into this function's configuration space) to a linked list of new capabilities implemented by the device. Used if bit 4 of the status register (Capabilities List bit) is set to 1. The bottom two bits are reserved and should be masked before the Pointer is used to access the Configuration Space.
    uint16_t reserved0;
    uint8_t reserved1;
    uint32_t reserved2;
    uint8_t interrupt_line;             // Interrupt Line: Specifies which input of the system interrupt controllers the device's interrupt pin is connected to and is implemented by any device that makes use of an interrupt pin. For the x86 architecture this register corresponds to the PIC IRQ numbers 0-15 (and not I/O APIC IRQ numbers) and a value of 0xFF defines no connection.
    uint8_t interrupt_pin;              // Specifies which interrupt pin the device uses. Where a value of 0x1 is INTA#, 0x2 is INTB#, 0x3 is INTC#, 0x4 is INTD#, and 0x0 means the device does not use an interrupt pin.
    uint8_t min_grant;                  // Min Grant: A read-only register that specifies the burst period length, in 1/4 microsecond units, that the device needs (assuming a 33 MHz clock rate).
    uint8_t max_latency;                // Max Latency: A read-only register that specifies how often the device needs access to the PCI bus (in 1/4 microsecond units).
} __attribute__((packed)) pci_general_device_t;

// Header Type 0x1 (PCI-to-PCI bridge)
typedef struct {
    pci_device_hdr_t header;
    uint32_t base_address0;             
    uint32_t base_address1;             
    uint8_t primary_bus_num;            
    uint8_t secondary_bus_num;          
    uint8_t subordinate_bus_num;        
    uint8_t secondary_latency_timer;    
    uint8_t io_base;
    uint8_t io_limit;
    uint16_t secondary_status;
    uint16_t memory_base;
    uint16_t memory_limit;
    uint16_t prefetchable_memory_base;
    uint16_t prefetchable_memory_limit;
    uint32_t prefetchable_base_upper;
    uint32_t prefetchable_limit_upper;
    uint16_t io_base_upper;
    uint16_t io_limit_upper;
    uint8_t capability_ptr;
    uint16_t reserved0;
    uint8_t reserved1;
    uint32_t expansion_rom_address;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t bridge_control;
} __attribute__((packed)) pci_to_pci_bridge_t;

typedef struct {
    pci_device_hdr_t header;
    uint32_t cardbus_socket_base_address;
    uint8_t capabilities_offset;
    uint8_t reserved0;
    uint16_t secondary_status;
    uint8_t pci_bus_number;
    uint8_t cardbus_bus_number;
    uint8_t subordinate_bus_number;
    uint8_t cardbus_latency_timer;
    uint32_t memory_base_address0;
    uint32_t memory_limit0;
    uint32_t memory_base_address1;
    uint32_t memory_limit1;
    uint32_t io_base_address0;
    uint32_t io_limit0;
    uint32_t io_base_address1;
    uint32_t io_limit1;
    uint8_t interrupt_line;
    uint8_t interrupt_pin;
    uint16_t bridge_control;
    uint16_t subsystem_device_id;
    uint16_t subsystem_vendor_id;
    uint32_t legacy_mode_base_address;
} __attribute__((packed)) pci_to_cardbus_bridge_t;

void pci_enumerate(acpi_mcfg_header_t *mcfg);
const char* pci_device_class(uint8_t class_code);
const char* pci_vendor_name(uint16_t vendor_id);
const char* pci_device_name(uint16_t vendor_id, uint16_t device_id);
const char* pci_subclass_name(uint8_t class_code, uint8_t subclass_code);
const char* pci_program_iface(uint8_t class_code, uint8_t subclass_code, uint8_t program_iface);