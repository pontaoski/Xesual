#include "ProcessManagement.h"
#include "MiscFunctions.h"
#include "Spinlock.h"

extern "C" void SyscallHandler();

namespace ProcessManagement
{

uint32_t CPUCount = 0;
CPUState* CPUs = nullptr;

ProcessInfo ProcessTable[64];
Spinlock ProcessTableSpinlock;

void CPUState::setKernelGSBase()
{
    uintptr_t base = (uintptr_t)this;

    cpuSetMSR(0xC0000101, base, base >> 32);
    cpuSetMSR(0xC0000102, base, base >> 32);
	asm volatile ("swapgs");
}

void CPUState::setSyscallRIP()
{
    uintptr_t base = (uintptr_t)SyscallHandler;

    // enable using the syscall instruction
    asm volatile (R"(
        mov 0xc0000080, %%rcx
        rdmsr
        or 1, %%rax
        wrmsr
    )" ::: "rcx", "rax");
    cpuSetMSR(0xC0000082, base, base >> 32);
}

void init() {
    ProcessTableSpinlock.init();
    memset(ProcessTable, 0, sizeof(ProcessInfo) * 64);
}

void schedule() {

}

}