#pragma once

#include "PhysicalMemoryManagement.h"
#include "MiscFunctions.h"

namespace SMP
{

inline void setAPICBase(PhysicalMemoryManagement::PAddress physAddr = PhysicalMemoryManagement::PAddress{0xFEE00000})
{
    uint32_t edx = (physAddr.Address >> 32) & 0x0f;
    uint32_t eax = (physAddr.Address & 0xfffff0000) | 0x800;

    cpuSetMSR(0x1B, eax, edx);
}

inline PhysicalMemoryManagement::PAddress getAPICBase()
{
    uint32_t eax, edx;
    cpuGetMSR(0x1B, &eax, &edx);

    return PhysicalMemoryManagement::PAddress{(eax & 0xfffff000) | ((edx & 0x0fUL) << 32)};
}

}
