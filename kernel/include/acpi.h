#pragma once

#include <stdint.h>

typedef struct {
    char signature[8];              // 8-byte (not null terminated) string: "RSD PTR "
    uint8_t checksum;
    char oem_id[6];                 // OEM-supplied string that identifies the OEM
    uint8_t revision;               // ACPI version. 0 = v1.0, 2 = v2.0 - v6.1
    uint32_t rsdt_address;          // 32-bit physical address of the RSDT table
    uint32_t length;                // size of the entire table
    uint64_t xsdt_address;          // 64-bit physical address of the XSDT table
    uint8_t extended_checksum;
    uint8_t reserved[3];            // 3 bytes ignored in reading and must not be written
} __attribute__((packed)) rsdp_descriptor_t;

typedef struct {
    char signature[4];
    uint32_t length;                // total size of the table, inclusive of the header
    uint8_t revision;
    uint8_t checksum;               // 8-bit checksum field of the whole table, inclusive of the header.
    char oem_id[6];
    char otem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_sdt_header_t;

typedef struct {
    acpi_sdt_header_t sdt_header;
    uint8_t reserved[8];
} __attribute__((packed)) acpi_mcfg_header_t;

typedef struct {
    uint64_t base_address;          // base address of enhanced configuration mechanism
    uint16_t segment_group;         // PCI segment group number
    uint8_t start_bus;              // start PCI bus number decoded by this host bridge
    uint8_t end_bus;                // end PCI bus number decoded by this host bridge
    uint32_t reserved;              // reserved
} __attribute__((packed)) acpi_mcfg_device_t;

void * acpi_find_table(acpi_sdt_header_t *header, char *signature);