#pragma once

#include "LocalAPIC.h"
#include "PhysicalMemoryManagement.h"
#include "SMP.h"
#include "GDT.h"
#include "TrapNumbers.h"
#include "Traps.h"
#include <cstddef>
#include <cstdint>

namespace ProcessManagement
{

struct ProcessInfo;

constexpr const int StackSize = 16 * PhysicalMemoryManagement::PageSize;

struct CPUState {
    int apicID;
    int32_t cliCount;
    bool wereInterruptsEnabled;
    ProcessInfo* currentProcess;
    void* interruptStack;
    GDT::Table table;

    /// used for preserving in syscalls
    uint64_t userStack;

    void setKernelGSBase();
    void setSyscallRIP();
    void setStarRegister();
};

static_assert(UserStackOffset == offsetof(CPUState, userStack));
static_assert(KernelInterruptStackOffset == offsetof(CPUState, interruptStack));

struct ProcessInfo {
    enum State {
        Unused = 0,
        Runnable,
        Running,
    };

    PageTableRef pageTable;
    State state;
    Traps::SyscallRegisters registers;
};

extern uint32_t CPUCount;
extern CPUState* CPUs;

void init();
void schedule();
void runFirstProcess();

inline CPUState* thisCPU()
{
    for (auto i = 0U; i < CPUCount; i++) {
        if (CPUs[i].apicID == LocalAPIC::lapicID()) {
            return &CPUs[i];
        }
    }
    return nullptr;
}

inline uint64_t eflags(void)
{
  uint64_t eflags;
  asm volatile("pushfq; popq %0" : "=r" (eflags));
  return eflags;
}

inline void pushCLI()
{
    auto flags = eflags();
    asm volatile("cli");

    if (thisCPU()->cliCount == 0)
        thisCPU()->wereInterruptsEnabled = flags & 0x200;

    thisCPU()->cliCount++;
}

inline void popCLI()
{
    thisCPU()->cliCount--;
    if (thisCPU()->cliCount == 0 && thisCPU()->wereInterruptsEnabled)
        asm volatile("sti");
}

}
