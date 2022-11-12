#pragma once

#include "PhysicalMemoryManagement.h"
#include "SMP.h"
#include <cstdint>

namespace LocalAPIC
{

enum TrapNumbers: uint32_t {
// Processor-defined:
    Division = 0, // divide error
    Debug = 1, // debug exception
    NonMaskable = 2, // non-maskable interrupt
    Breakpoint = 3, // breakpoint
    Overflow = 4, // overflow
    Bounds = 5, // bounds check
    IllegalOpcode = 6, // illegal opcode
    DeviceNotAvailable = 7, // device not available
    DoubleFault = 8, // double fault
    CoprocessorReserved = 9,      // reserved (not used since 486)
    InvalidTaskSwitchSegment = 10,      // invalid task switch segment
    SegmentNotPresent = 11,      // segment not present
    StackException = 12,      // stack exception
    GeneralProtectionFault = 13,      // general protection fault
    PageFault = 14,      // page fault
    Reserved = 15,      // reserved
    FloatingPointError = 16,      // floating point error
    AlignmentCheck = 17,      // aligment check
    MachineCheck = 18,      // machine check
    SIMDFloatingPointError = 19,      // SIMD floating point error

    Syscall = 64,      // system call
    Catchall = 500,      // catchall

    // IRQ 0 corresponds to int T_IRQ
    IRQ0 = 32,

    IRQTimer = 0,
    IRQKeyboard = 1,
    IRQCOM1 = 4,
    IRQIDE = 14,
    IRQError = 19,
    IRQSpurious = 31,
};

inline int lapicID()
{
    volatile auto lapic = (uint32_t*)PhysicalMemoryManagement::toVirtual(SMP::getAPICBase()).asPtr();
    return lapic[0x0020/4] >> 24;
}

void setLAPICAddress();
void initLAPIC();
void acknowledgeInterrupt();

}