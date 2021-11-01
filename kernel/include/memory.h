#pragma once

#include "types.h"
#include "efimem.h"

uint64_t system_memory_size(memory_info_t *memory_info);
void* memset(void *dest, char value, size_t len);
void* memzero(void *dest, size_t len);