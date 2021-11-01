#pragma once

#include <stdint.h>

static inline void cpuid(int code, uint32_t *a, uint32_t *d);
static inline uint64_t rdtsc(void);
static inline unsigned long read_cr0(void);
static inline void invlpg(void * m);
static inline void wrmsr(uint64_t msr, uint64_t value);
static inline uint64_t rdmsr(uint32_t msr);