#include "Traps.h"
#include "GDT.h"
#include "LocalAPIC.h"
#include "Panic!!.h"
#include "ProcessManagement.h"
#include "TrapNumbers.h"
#include "printf.h"
#include <cstddef>

extern "C"
{

enum Exceptions: uint64_t {
	DoubleFault = 8,
	GeneralProtectionFault = 13,
	PageFault = 14,
};

Traps::InterruptRegisters* TrapHandler(Traps::InterruptRegisters* registers)
{
	switch (registers->interruptNumber) {
	case SyscallInterrupt:
		return registers;
	case LocalAPIC::IRQ0 + LocalAPIC::IRQTimer:
		LocalAPIC::acknowledgeInterrupt();
		break;
	case Exceptions::DoubleFault: {
		PanicExclamationExclamation("Double fault", registers);
		break;
	case Exceptions::GeneralProtectionFault:
		if (ProcessManagement::thisCPU()->currentProcess == nullptr || registers->cs == offsetof(GDT::Table, kernelCode)) {
			PanicExclamationExclamation("GPF in kernel code", registers);
		}
		// TODO: kill userspace
		break;
	case Exceptions::PageFault:
		if (ProcessManagement::thisCPU()->currentProcess == nullptr || registers->cs == offsetof(GDT::Table, kernelCode)) {
			PanicExclamationExclamation("Page fault in kernel code", registers);
		}
		// TODO: kill userspace
		break;
	}
	default:
		logfn("unhandled interrupt %d/%llx\n", registers->interruptNumber, registers->interruptNumber);
	}
	return registers;
}

Traps::SyscallRegisters* KernelSyscallHandler(Traps::SyscallRegisters* registers)
{
	logfn("syscall! %llx", registers->rax);
	while (true) {
		asm volatile ("cli");
		asm volatile ("hlt");
	}
	return registers;
}

}

namespace Traps
{

static IDTEntry IDTEntries[256];

void IDTEntry::setGate(InterruptHandler handler, uint16_t _selector, IDTEntryFlags gateFlags, bool canBeCalledByUserspace)
{
	auto base = (uintptr_t)handler;
	baseLow = base & 0xFFFF;
	baseMid = (base >> 16) & 0xFFFF;
	baseHigh = (base >> 32) & 0xFFFFFFFF;
	selector = _selector;
	zero = 0;
	padding = 0;
	flags = gateFlags;

	if (canBeCalledByUserspace)
		flags.bits |= 0x60;
}

InterruptHandler IDTEntry::getGate()
{
	uintptr_t res = baseLow | ((uintptr_t)baseMid << 16) | ((uintptr_t)baseHigh << 32);
	return (InterruptHandler)res;
}

void setupIDTTable()
{
	// fifth entry in limine GDT: https://github.com/limine-bootloader/limine/blob/trunk/PROTOCOL.md
	// 64-bit code selector
	const int CodeSelector = offsetof(GDT::Table, kernelCode);

	IDTEntries[0].setGate(_isr0,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[1].setGate(_isr1,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[2].setGate(_isr2,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[3].setGate(_isr3,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[4].setGate(_isr4,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[5].setGate(_isr5,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[6].setGate(_isr6,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[7].setGate(_isr7,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[8].setGate(_isr8,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[9].setGate(_isr9,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[10].setGate(_isr10, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[11].setGate(_isr11, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[12].setGate(_isr12, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[13].setGate(_isr13, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[14].setGate(_isr14, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[15].setGate(_isr15, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[16].setGate(_isr16, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[17].setGate(_isr17, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[18].setGate(_isr18, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[19].setGate(_isr19, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[20].setGate(_isr20, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[21].setGate(_isr21, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[22].setGate(_isr22, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[23].setGate(_isr23, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[24].setGate(_isr24, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[25].setGate(_isr25, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[26].setGate(_isr26, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[27].setGate(_isr27, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[28].setGate(_isr28, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[29].setGate(_isr29, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[30].setGate(_isr30, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[31].setGate(_isr31, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[32].setGate(_irq0,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[33].setGate(_irq1,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[34].setGate(_irq2,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[35].setGate(_irq3,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[36].setGate(_irq4,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[37].setGate(_irq5,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[38].setGate(_irq6,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[39].setGate(_irq7,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[40].setGate(_irq8,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[41].setGate(_irq9,  CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[42].setGate(_irq10, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[43].setGate(_irq11, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[44].setGate(_irq12, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[45].setGate(_irq13, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[46].setGate(_irq14, CodeSelector, IDTEntryFlags::interruptGate());
	IDTEntries[47].setGate(_irq15, CodeSelector, IDTEntryFlags::interruptGate());

	IDTEntries[123].setGate(_isr123, CodeSelector, IDTEntryFlags::interruptGate(), false);
	IDTEntries[124].setGate(_isr124, CodeSelector, IDTEntryFlags::interruptGate(), false);
	IDTEntries[125].setGate(_isr125, CodeSelector, IDTEntryFlags::interruptGate(), false);
	IDTEntries[126].setGate(_isr126, CodeSelector, IDTEntryFlags::interruptGate(), false);
	IDTEntries[SyscallInterrupt].setGate(ISRForNumber(SyscallInterrupt), CodeSelector, IDTEntryFlags::interruptGate(), true);
}

void loadIDTTable()
{
    volatile struct IDTPointer {
        uint16_t limit;
        uint64_t base;
    } __attribute__ ((__packed__)) idtPointer;
    idtPointer.limit = sizeof(IDTEntries);
    idtPointer.base = (uintptr_t)&IDTEntries;

    asm volatile (
        "lidt %0"
        : : "m"(idtPointer)
    );
}

}
