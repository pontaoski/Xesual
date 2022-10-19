#pragma once

#include <cstdint>
#include <cstddef>

inline void* memset(void* b, int val, std::size_t count) {
    asm volatile ("cld; rep stosb" : "+c" (count), "+D" (b) : "a" (val) : "memory");
    return b;
}

inline void cpuGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
   asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}
 
inline void cpuSetMSR(uint32_t msr, uint32_t lo, uint32_t hi)
{
   asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}
