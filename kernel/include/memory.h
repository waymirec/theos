#pragma once

#include "types.h"
#include "efimem.h"

#include <stdint.h>
#include <stddef.h>

uint64_t system_memory_size(memory_info_t *memory_info);
void* memset(void *dest, char value, size_t len);
void* memzero(void *dest, size_t len);
int memcmp(const void *s1, const void *s2, size_t n);
void* memcpy(void *dest, const void *src, size_t len);
void* memrcpy(void *dest, const void *src, size_t len);