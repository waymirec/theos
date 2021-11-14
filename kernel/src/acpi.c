#include "acpi.h"

#define ACPI_STD_HEADER_SIZE_BYTES 8

void * acpi_find_table(acpi_sdt_header_t *header, char *signature)
{
    int count = (header->length - sizeof(acpi_sdt_header_t)) / 8;
    for (int i = 0; i < count; i++) {
        acpi_sdt_header_t *hdr =  (acpi_sdt_header_t *)*(uint64_t *)((uint64_t)header + sizeof(acpi_sdt_header_t) + (i * ACPI_STD_HEADER_SIZE_BYTES));
        if (hdr->signature[0] != signature[0]) continue;
        if (hdr->signature[1] != signature[1]) continue;
        if (hdr->signature[2] != signature[2]) continue;
        if (hdr->signature[3] != signature[3]) continue;
        return hdr;
    }
    return 0;
}