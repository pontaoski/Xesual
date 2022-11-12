#pragma once

#include "TrapNumbers.h"
#include <cstdint>

namespace Traps
{

void setupIDTTable();
void loadIDTTable();

struct Registers {
	uint64_t r15, r14, r13, r12;
	uint64_t r11, r10, r9, r8;
	uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;

	uint64_t interruptNumber, errorCode;

	uint64_t rip, cs, rflags, rsp, ss;
};

extern "C"
{

using InterruptHandler = Traps::Registers*(*)(Traps::Registers*);

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

extern Traps::Registers* _isr0(Traps::Registers*);
extern Traps::Registers* _isr1(Traps::Registers*);
extern Traps::Registers* _isr2(Traps::Registers*);
extern Traps::Registers* _isr3(Traps::Registers*);
extern Traps::Registers* _isr4(Traps::Registers*);
extern Traps::Registers* _isr5(Traps::Registers*);
extern Traps::Registers* _isr6(Traps::Registers*);
extern Traps::Registers* _isr7(Traps::Registers*);
extern Traps::Registers* _isr8(Traps::Registers*);
extern Traps::Registers* _isr9(Traps::Registers*);
extern Traps::Registers* _isr10(Traps::Registers*);
extern Traps::Registers* _isr11(Traps::Registers*);
extern Traps::Registers* _isr12(Traps::Registers*);
extern Traps::Registers* _isr13(Traps::Registers*);
extern Traps::Registers* _isr14(Traps::Registers*);
extern Traps::Registers* _isr15(Traps::Registers*);
extern Traps::Registers* _isr16(Traps::Registers*);
extern Traps::Registers* _isr17(Traps::Registers*);
extern Traps::Registers* _isr18(Traps::Registers*);
extern Traps::Registers* _isr19(Traps::Registers*);
extern Traps::Registers* _isr20(Traps::Registers*);
extern Traps::Registers* _isr21(Traps::Registers*);
extern Traps::Registers* _isr22(Traps::Registers*);
extern Traps::Registers* _isr23(Traps::Registers*);
extern Traps::Registers* _isr24(Traps::Registers*);
extern Traps::Registers* _isr25(Traps::Registers*);
extern Traps::Registers* _isr26(Traps::Registers*);
extern Traps::Registers* _isr27(Traps::Registers*);
extern Traps::Registers* _isr28(Traps::Registers*);
extern Traps::Registers* _isr29(Traps::Registers*);
extern Traps::Registers* _isr30(Traps::Registers*);
extern Traps::Registers* _isr31(Traps::Registers*);

extern Traps::Registers* _irq0(Traps::Registers*);
extern Traps::Registers* _irq1(Traps::Registers*);
extern Traps::Registers* _irq2(Traps::Registers*);
extern Traps::Registers* _irq3(Traps::Registers*);
extern Traps::Registers* _irq4(Traps::Registers*);
extern Traps::Registers* _irq5(Traps::Registers*);
extern Traps::Registers* _irq6(Traps::Registers*);
extern Traps::Registers* _irq7(Traps::Registers*);
extern Traps::Registers* _irq8(Traps::Registers*);
extern Traps::Registers* _irq9(Traps::Registers*);
extern Traps::Registers* _irq10(Traps::Registers*);
extern Traps::Registers* _irq11(Traps::Registers*);
extern Traps::Registers* _irq12(Traps::Registers*);
extern Traps::Registers* _irq13(Traps::Registers*);
extern Traps::Registers* _irq14(Traps::Registers*);
extern Traps::Registers* _irq15(Traps::Registers*);

extern Traps::Registers* _isr123(Traps::Registers*);
extern Traps::Registers* _isr124(Traps::Registers*);
extern Traps::Registers* _isr125(Traps::Registers*);
extern Traps::Registers* _isr126(Traps::Registers*);
extern Traps::Registers* ISRForNumber(SyscallInterrupt) (Traps::Registers*);

}
