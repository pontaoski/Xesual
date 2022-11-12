#include "ProcessManagement.h"
#include "MiscFunctions.h"
#include "Spinlock.h"

namespace ProcessManagement
{

uint32_t CPUCount = 0;
CPUState* CPUs = nullptr;

ProcessInfo ProcessTable[64];
Spinlock ProcessTableSpinlock;

void init() {
    ProcessTableSpinlock.init();
    memset(ProcessTable, 0, sizeof(ProcessInfo) * 64);
}

void schedule() {
    auto cpu = thisCPU();

    for (;;) {
        asm volatile ("sti");

        ProcessTableSpinlock.acquire();
        for (auto p = ProcessTable; p < &ProcessTable[64]; p++) {
            if (p->state == ProcessInfo::Runnable)
                continue;

            cpu->currentProcess = p;
            p->state = ProcessInfo::Running;

            cpu->currentProcess = nullptr;
        }
        ProcessTableSpinlock.release();
    }
}

}