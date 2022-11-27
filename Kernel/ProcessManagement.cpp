#include "ProcessManagement.h"
#include "GDT.h"
#include "MiscFunctions.h"
#include "PhysicalMemoryManagement.h"
#include "Spinlock.h"
#include "Traps.h"
#include "VirtualMemoryManagement.h"
#include <cstddef>


extern "C" {

extern "C" void TrapRet();
extern void SyscallHandler();

void __attribute__((section (".userspace"))) __attribute__((naked)) MyFirstUserspace()
{
    asm volatile (R"(
        nop
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

void __attribute__((naked)) Pause(KernelContext** yieldFrom, KernelContext* yieldTo)
{
    asm volatile (R"(
        pushq %r15
        pushq %r14
        pushq %r13
        pushq %r12
        pushq %rbp
        pushq %rbx

        movq %rsp, (%rdi)
        movq %rsi, %rsp

        popq %rbx
        popq %rbp
        popq %r12
        popq %r13
        popq %r14
        popq %r15

        retq
    )");
}

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
        mov $0xc0000080, %%rcx
        rdmsr
        or $1, %%rax
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

void schedule()
{
    auto cpu = thisCPU();
    cpu->currentProcess = nullptr;

    while (true) {
        asm volatile ("sti");

        ProcessTableSpinlock.acquire();
        for (auto p = ProcessTable; p < &ProcessTable[64]; p = p++) {
            if (p->state != ProcessInfo::Runnable)
                continue;
            logfn("found a process to run! :)\n");

            cpu->currentProcess = p;
            asm volatile ("movq %0, %%cr3" :: "r" (
                toPhysical(PhysicalMemoryManagement::HHVAddress{(uint64_t)p->pageTable})
            ));
            p->state = ProcessInfo::Running;

            logfn("sweeetch! :)\n");
            Pause(&cpu->schedulerTask, p->kernelTask);
        }
        ProcessTableSpinlock.release();
    }
}

// first yield to a process's kernel task will go here
void ProcessEntryPoint()
{
    ProcessTableSpinlock.release();
}

ProcessInfo* allocateProcess()
{
    ProcessTableSpinlock.acquire();

    ProcessInfo* p = nullptr;
    bool found = false;
    for (p = ProcessTable; p < &ProcessTable[64]; p++)
        if (p->state == ProcessInfo::Unused) {
            found = true;
            break;
        }

    if (!found) {
        ProcessTableSpinlock.release();
        return nullptr;
    }

    p->pageTable = VirtualMemoryManagement::shallowCopy(VirtualMemoryManagement::KernelPageTable);

    p->state = ProcessInfo::Fresh;
    ProcessTableSpinlock.release();

    p->kernelStack = PhysicalMemoryManagement::allocatePage().asPtr();
    if (p->kernelStack == nullptr) {
        p->state = ProcessInfo::Unused;
        return nullptr;
    }

    char* stackPointer = (char*)p->kernelStack + PhysicalMemoryManagement::PageSize;

    stackPointer -= sizeof(Traps::InterruptRegisters);
    p->trapFrame = (Traps::InterruptRegisters*)stackPointer;

    stackPointer -= sizeof(uintptr_t);
    *(uintptr_t*)stackPointer = (uintptr_t)TrapRet;

    stackPointer -= sizeof(*p->kernelTask);
    p->kernelTask = (KernelContext*)stackPointer;
    memset(p->kernelTask, 0, sizeof(*p->kernelTask));
    p->kernelTask->rip = (uintptr_t)ProcessEntryPoint;

    return p;
}

void createFirstProcess()
{
    auto process = allocateProcess();

    auto page = PhysicalMemoryManagement::allocatePage();
    memcpy(page.asPtr(), (void*)MyFirstUserspace, (uintptr_t)MyFirstUserspaceEnd-(uintptr_t)MyFirstUserspace);
    const int Entrypoint = 0x1000;

    VirtualMemoryManagement::map(process->pageTable, Entrypoint, PhysicalMemoryManagement::toPhysical(page).Address);

    memset(process->trapFrame, 0, sizeof(Traps::InterruptRegisters));
    process->trapFrame->cs = offsetof(GDT::Table, userCode64) | GDT::Ring3;
    process->trapFrame->ss = offsetof(GDT::Table, userData64) | GDT::Ring3;
    process->trapFrame->rflags = 0x200; // enable interrupt
    process->trapFrame->rsp = 0; // TODO: stack
    process->trapFrame->rip = Entrypoint;

    ProcessTableSpinlock.acquire();
    process->state = ProcessInfo::Runnable;
    ProcessTableSpinlock.release();
}

}