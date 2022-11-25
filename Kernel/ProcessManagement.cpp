#include "ProcessManagement.h"
#include "MiscFunctions.h"
#include "PhysicalMemoryManagement.h"
#include "Spinlock.h"
#include "VirtualMemoryManagement.h"
#include <cstddef>


extern "C" {

extern void SyscallHandler();

void __attribute__((section (".userspace"))) __attribute__((naked)) MyFirstUserspace()
{
    asm volatile (R"(
        mov $1, %rax
        syscall
    )");
}

extern char MyFirstUserspaceEnd[];

}

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

void CPUState::setStarRegister()
{
    const int StarRegister = 0xC0000081;
    const uint32_t KernelSelector = offsetof(GDT::Table, kernelCode) | GDT::Ring0;
    const uint32_t UserSelector = offsetof(GDT::Table, userCode32) | GDT::Ring3;

    cpuSetMSR(StarRegister, 0, KernelSelector | (UserSelector >> 16));
}

void init() {
    ProcessTableSpinlock.init();
    memset(ProcessTable, 0, sizeof(ProcessInfo) * 64);
}

// void schedule() {
//     pushCLI();
//     ProcessTableSpinlock.acquire();

//     for (auto& process : ProcessTable) {
//         if (process.state != ProcessInfo::Runnable)
//             continue;

//         process.state = ProcessInfo::Running;
//     }

//     ProcessTableSpinlock.release();
//     popCLI();
// }

void runFirstProcess()
{
    auto page = PhysicalMemoryManagement::allocatePage();
    memcpy(page.asPtr(), (void*)MyFirstUserspace, (uintptr_t)MyFirstUserspaceEnd-(uintptr_t)MyFirstUserspace);

    ProcessManagement::ProcessInfo pi;
    pi.pageTable = VirtualMemoryManagement::shallowCopy(VirtualMemoryManagement::KernelPageTable);
    VirtualMemoryManagement::map(pi.pageTable, 0x0, PhysicalMemoryManagement::toPhysical(page).Address);
    pi.state = ProcessManagement::ProcessInfo::Running;
    ProcessTable[0] = pi;
    thisCPU()->currentProcess = &ProcessTable[0];

    asm volatile("movq %0, %%cr3" :: "r" (
        toPhysical(PhysicalMemoryManagement::HHVAddress{(uint64_t)pi.pageTable})
    ));
    asm volatile (R"(
        movq $0, %%r11
        movq %0, %%rcx
        sysret
    )" :: "r"(uint64_t(0x0)));
}

}