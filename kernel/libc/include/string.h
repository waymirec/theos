#ifndef _STRING_H
#define _STRING_H 1

#include <sys/cdefs.h>

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memrcpy(void *dest, const void *src, size_t len);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
void* memzero(void *dest, size_t len);
size_t strlen(const char*);
size_t strcpy(void *dstptr, const void *srcptr);

#ifdef __cplusplus
}
#endif

#endif
