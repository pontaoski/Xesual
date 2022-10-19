#pragma once

#include "PhysicalMemoryManagement.h"
#include "SMP.h"
#include <cstdint>

namespace LocalAPIC
{

inline int lapicID()
{
    volatile auto lapic = (uint32_t*)PhysicalMemoryManagement::toVirtual(SMP::getAPICBase()).asPtr();
    return lapic[0x0020/4] >> 24;
}

void setLAPICAddress();
void initLAPIC();

}