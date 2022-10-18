#pragma once

#include <cstddef>

inline void* memset(void* b, int val, std::size_t count) {
    asm volatile ("cld; rep stosb" : "+c" (count), "+D" (b) : "a" (val) : "memory");
    return b;
}
