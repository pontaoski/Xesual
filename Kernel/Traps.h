#pragma once

#include "TrapNumbers.h"
#include <cstdint>

namespace Traps
{

void setupIDTTable();
void loadIDTTable();

struct InterruptRegisters {
	uint64_t r15, r14, r13, r12;
	uint64_t r11, r10, r9, r8;
	uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

	uint64_t interruptNumber, errorCode;

	uint64_t rip, cs, rflags, rsp, ss;
};

struct SyscallRegisters {
	uint64_t r15, r14, r13, r12;
	uint64_t r11, r10, r9, r8;
	uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
};

extern "C"
{

using InterruptHandler = Traps::InterruptRegisters*(*)(Traps::InterruptRegisters*);

}

union IDTEntryFlags {
    struct {
        uint8_t type: 4;
        uint8_t storageSegment: 1;
        uint8_t privilegeLevel: 2;
        uint8_t present: 1;
    } __attribute__ ((__packed__)) highLevel;
    uint8_t bits;

    static IDTEntryFlags interruptGate()
    {
        return IDTEntryFlags{ .bits = 0x8E };
    }
    static IDTEntryFlags trapGate()
    {
        return IDTEntryFlags{ .bits = 0x8F };
    }
};

struct IDTEntry {
    uint16_t baseLow;
    uint16_t selector;

    uint8_t zero;

    IDTEntryFlags flags;

    uint16_t baseMid;
    uint32_t baseHigh;
    uint32_t padding;

    void setGate(InterruptHandler handler, uint16_t _selector, IDTEntryFlags gateFlags, bool canBeCalledByUserspace = false);
    InterruptHandler getGate();
} __attribute__ ((__packed__));

}

extern "C"
{

extern Traps::InterruptRegisters* _isr0(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr1(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr2(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr3(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr4(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr5(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr6(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr7(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr8(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr9(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr10(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr11(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr12(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr13(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr14(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr15(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr16(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr17(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr18(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr19(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr20(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr21(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr22(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr23(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr24(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr25(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr26(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr27(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr28(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr29(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr30(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr31(Traps::InterruptRegisters*);

extern Traps::InterruptRegisters* _irq0(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq1(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq2(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq3(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq4(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq5(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq6(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq7(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq8(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq9(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq10(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq11(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq12(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq13(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq14(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _irq15(Traps::InterruptRegisters*);

extern Traps::InterruptRegisters* _isr123(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr124(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr125(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* _isr126(Traps::InterruptRegisters*);
extern Traps::InterruptRegisters* ISRForNumber(SyscallInterrupt) (Traps::InterruptRegisters*);

}
