#include "LocalAPIC.h"
#include "PhysicalMemoryManagement.h"
#include "SMP.h"

namespace LocalAPIC
{

volatile uint32_t* lapic;

// offsets are divided by 4 to be able to index that up there
enum LAPICRegisters: uint32_t {
    ID = (0x0020/4),
    Version = (0x0030/4),
    TaskPriority = (0x0080/4),
    EndOfInterrupt = (0x00B0/4),
    SpuriousInterruptVector = (0x00F0/4),
    ErrorStatus = (0x0280/4),
    LowInterruptCommand = (0x0300/4),
    HighInterruptCommand = (0x0310/4),
    // local vector tables
    LVT0Timer = (0x0320/4),
    LVTPerfCount = (0x0340/4),
    LVT1Interrupt0 = (0x0350/4),
    LVT2Interrupt1 = (0x0360/4),
    LVT3Error = (0x0370/4),
    TimerInitialCount = (0x0380/4),
    TimerCurrentCount = (0x0390/4),
    TimerDivideConfiguration = (0x03E0/4),
};

static constexpr uint32_t SVREnable = 0x00000100;
static constexpr uint32_t TimerDivideBy1 = 0x0000000B;
static constexpr uint32_t TimerPeriodic = 0x00020000;
static constexpr uint32_t ErrorMasked = 0x00010000;

enum InterruptCommand: uint32_t {
    Initialize = 0x00000500,
    StartupIPI = 0x00000600,
    DeliveryStatus = 0x00001000,
    Assert = 0x00004000,
    Deassert = 0x00000000,
    LevelTriggered = 0x00008000,
    Broadcast = 0x00080000,
    Busy = 0x00001000,
    Fixed = 0x00000000,
};

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

static void writeLAPIC(uint32_t index, uint32_t value)
{
    lapic[index] = value;
    // force a read to ensure the write is done
    lapic[LAPICRegisters::ID];
}

void setLAPICAddress()
{
    lapic = (uint32_t*)PhysicalMemoryManagement::toVirtual(SMP::getAPICBase()).asPtr();
}

void initLAPIC()
{
    writeLAPIC(SpuriousInterruptVector, SVREnable | (IRQ0 + IRQSpurious));
    writeLAPIC(TimerDivideConfiguration, TimerDivideBy1);
    writeLAPIC(LVT0Timer, TimerPeriodic | (IRQ0 + IRQTimer));
    writeLAPIC(TimerInitialCount, 10000000);

    writeLAPIC(LVT1Interrupt0, ErrorMasked);
    writeLAPIC(LVT2Interrupt1, ErrorMasked);

    if (((lapic[Version] >> 16) & 0xFF) >= 4)
        writeLAPIC(LVTPerfCount, ErrorMasked);

    writeLAPIC(LVT3Error, IRQ0 + IRQError);

    // clearing this requires a double write
    writeLAPIC(ErrorStatus, 0);
    writeLAPIC(ErrorStatus, 0);

    // acknowledge any outstanding inputs
    writeLAPIC(EndOfInterrupt, 0);

    writeLAPIC(HighInterruptCommand, 0);
    writeLAPIC(LowInterruptCommand, Broadcast | Initialize | LevelTriggered);

    while(lapic[LowInterruptCommand] & DeliveryStatus);

    // enable interrupts from LAPIC (but not on us!)
    writeLAPIC(TaskPriority, 0);
}

}
