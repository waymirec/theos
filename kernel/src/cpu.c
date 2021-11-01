#include "cpu.h"

// Request for CPU identification
static inline void cpuid(int code, uint32_t *a, uint32_t *d)
{
    asm volatile ( "cpuid" : "=a"(*a), "=d"(*d) : "0"(code) : "ebx", "ecx" );
}

// Read the current value of the CPU's time-stamp counter and store into EDX:EAX
static inline uint64_t rdtsc(void)
{
    uint32_t low, high;
    asm volatile ( "rdtsc" : "=a"(low), "=d"(high));
    return ((uint64_t)high << 32) | low;
}

// Read the value in CR0
static inline unsigned long read_cr0(void)
{
    unsigned long ret;
    asm volatile ( "mov %%cr0, %0" : "=r"(ret) );
    return ret;
}

// Invalidates the TLB for one specific virtual address
static inline void invlpg(void * m)
{
    /* Clobber memory to avoid optimizer re-ordering access before invlpg, which may cause nasty bugs. */
    asm volatile ( "invlpg (%0)" : : "b"(m) : "memory" );
}

// Write a 64-bit value to a MSR.
static inline void wrmsr(uint64_t msr, uint64_t value)
{
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    asm volatile ( "wrmsr" : : "c"(msr), "a"(low), "d"(high) );
}

// Read a 64-bit value from a MSR
static inline uint64_t rdmsr(uint32_t msr)
{
    uint32_t low, high;
    asm volatile ( "rdmsr" : "=a"(low), "=d"(high) : "c"(msr) );
    return ((uint64_t)high << 32) | low;
}