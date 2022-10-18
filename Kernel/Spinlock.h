#pragma once

#include <cstdint>

struct Spinlock
{
    uint16_t held;

    void init();
    void acquire();
    void release();
};
