#pragma once

#include "types.h"

int memcmp(const void *s1, const void *s2, size_t n);
void * memcpy(void *dest, void *src, size_t len);
char * strcpy(char *dest, char *src);
char* uint_to_string(uint64_t value, char *buffer);
char* int_to_string(int64_t value, char *buffer);
char* uint64_to_hex(uint64_t value, char *buffer);
char* uint32_to_hex(uint32_t value, char *buffer);
char* uint16_to_hex(uint16_t value, char *buffer);
char* uint8_to_hex(uint8_t value, char *buffer);
char* int_to_hex(int64_t value, char *buffer);
char* double_to_string(double value, uint8_t precision, char *buffer);
size_t strlen(char *string);
size_t strrev(char *string);
void reverse(void *buffer, size_t len);

