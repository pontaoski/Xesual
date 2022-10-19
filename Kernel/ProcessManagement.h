#pragma once

#include "LocalAPIC.h"
#include "SMP.h"
#include <cstdint>

namespace ProcessManagement
{

struct CPUState {
    int apicID;
    int32_t cliCount;
};

extern uint32_t CPUCount;
extern CPUState* CPUs;

inline CPUState* thisCPU()
{
    for (auto i = 0U; i < CPUCount; i++) {
        if (CPUs[i].apicID == LocalAPIC::lapicID()) {
            return &CPUs[i];
        }
    }
    return nullptr;
}

inline void pushCLI()
{
    asm volatile("cli");
    thisCPU()->cliCount++;
}

inline void popCLI()
{
    // TODO: validate in cli
    // TODO: validate cliCount >= 0
    thisCPU()->cliCount--;
    if (thisCPU()->cliCount == 0)
        asm volatile("sti");
}

}
