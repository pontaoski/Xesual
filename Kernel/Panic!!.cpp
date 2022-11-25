#include "Panic!!.h"
#include "printf.h"

void dumpRegisters(Traps::InterruptRegisters* registers)
{
	logfn("r15=%#llx, r14=%#llx, r13=%#llx, r12=%#llx\n", registers->r15, registers->r14, registers->r13, registers->r12);
	logfn("r11=%#llx, r10=%#llx, r9=%#llx, r8=%#llx\n", registers->r11, registers->r10, registers->r9, registers->r8);
	logfn("rbp=%#llx, rdi=%#llx, rsi=%#llx, rdx=%#llx, rcx=%#llx, rbx=%#llx, rax=%#llx\n", registers->rbp, registers->rdi, registers->rsi, registers->rdx, registers->rcx, registers->rbx, registers->rax);
	logfn("interruptNumber=%#llx, errorCode=%#llx\n", registers->interruptNumber, registers->errorCode);
	logfn("rip=%#llx, cs=%#llx, rflags=%#llx, rsp=%#llx, ss=%#llx\n", registers->rip, registers->cs, registers->rflags, registers->rsp, registers->ss);
}

void PanicExclamationExclamation(const char *why, Traps::InterruptRegisters* registers)
{
    logfn("fatal: %s\n", why);
    dumpRegisters(registers);
    asm volatile ("cli");
    while (true) {
        asm volatile ("hlt");
    }
}