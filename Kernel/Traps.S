#include "TrapNumbers.h"

.macro GenerateIRQStub index byte
    .global _irq\index

    _irq\index:
        pushq $0x00
        pushq $\byte
        jmp TrapCommon
.endm

.macro GenerateISRStub_NoErrorCode index
    .global _isr\index

    _isr\index:
        pushq $0x00
        pushq $\index
        jmp TrapCommon
.endm

.macro GenerateISRStub_WithErrorCode index
    .global _isr\index

    _isr\index:
        pushq $\index
        jmp TrapCommon
.endm

.macro Symbol prefix name
    .global \prefix\name
    \prefix\name:
.endm

GenerateISRStub_NoErrorCode 0
GenerateISRStub_NoErrorCode 1
GenerateISRStub_NoErrorCode 3
GenerateISRStub_NoErrorCode 4
GenerateISRStub_NoErrorCode 5
GenerateISRStub_NoErrorCode 6
GenerateISRStub_NoErrorCode 7
GenerateISRStub_WithErrorCode 8
GenerateISRStub_NoErrorCode 9
GenerateISRStub_WithErrorCode 10
GenerateISRStub_WithErrorCode 11
GenerateISRStub_WithErrorCode 12
GenerateISRStub_WithErrorCode 13
GenerateISRStub_WithErrorCode 14
GenerateISRStub_NoErrorCode 15
GenerateISRStub_NoErrorCode 16
GenerateISRStub_NoErrorCode 17
GenerateISRStub_NoErrorCode 18
GenerateISRStub_NoErrorCode 19
GenerateISRStub_NoErrorCode 20
GenerateISRStub_NoErrorCode 21
GenerateISRStub_NoErrorCode 22
GenerateISRStub_NoErrorCode 23
GenerateISRStub_NoErrorCode 24
GenerateISRStub_NoErrorCode 25
GenerateISRStub_NoErrorCode 26
GenerateISRStub_NoErrorCode 27
GenerateISRStub_NoErrorCode 28
GenerateISRStub_NoErrorCode 29
GenerateISRStub_NoErrorCode 30
GenerateISRStub_NoErrorCode 31

GenerateIRQStub 0, 32
GenerateIRQStub 1, 33
GenerateIRQStub 2, 34
GenerateIRQStub 3, 35
GenerateIRQStub 4, 36
GenerateIRQStub 5, 37
GenerateIRQStub 6, 38
GenerateIRQStub 7, 39
GenerateIRQStub 8, 40
GenerateIRQStub 9, 41
GenerateIRQStub 10, 42
GenerateIRQStub 11, 43
GenerateIRQStub 12, 44
GenerateIRQStub 13, 45
GenerateIRQStub 14, 46
GenerateIRQStub 15, 47

GenerateISRStub_NoErrorCode SyscallInterrupt

Symbol _isr, ClockInterrupt
    pushq %r12
    mov (LocalAPICAddress), %r12
    add $0xb0, %r12
    popq %r12

    cmpq $8, 8(%rsp)
    je 1f

    pushq $0x00
    pushq ClockInterrupt

    jmp TrapCommon

1:
    iretq


Symbol _isr, BadTLBInterrupt
    pushq %r12
    mov %cr3, %r12
    mov %r12, %cr3
    mov (LocalAPICAddress), %r12
    add $0xb0, %r12
    movl $0, (%r12)
    popq %r12
    iretq

Symbol _isr, NoOpInterrupt
    pushq %r12
    mov (LocalAPICAddress), %r12
    add $0xb0, %r12
    movl $0, (%r12)
    popq %r12
    iretq


Symbol _isr, HaltInterrupt
    cli
1:
    hlt
    jmp 1b

Symbol _isr, HaltInterrupt2
    cli
1:
    hlt
    jmp 1b

.global TrapCommon
TrapCommon:
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    cld

    mov %rsp, %rdi
    call TrapHandler
    mov %rax, %rsp

.global TrapRet
TrapRet:

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    add $16, %rsp

    iretq

.global SyscallHandler
SyscallHandler:
    // rcx: rip of userspace
    // r11: rflags of userspace
    // rax: return code

    // rsp isn't saved for us, so we need to put it in a place we made in the CPUState structure
    // for it
    // switch to kernel gsbase (pointer to CPUState for this cpu)
    swapgs

    // load the address of thisCPU offset 0 to rax
    mov %rsp, %gs:UserStackOffset
    // load kernel stack offset into rsp
    mov %gs:KernelInterruptStackOffset, %rsp

    // now we can push
    push %rax
    push %rbx
    push %rcx
    push %rdx
    push %rsi
    push %rdi
    push %rbp
    push %r8
    push %r9
    push %r10
    push %r11
    push %r12
    push %r13
    push %r14
    push %r15

    call KernelSyscallHandler

    pop %r15
    pop %r14
    pop %r13
    pop %r12
    pop %r11
    pop %r10
    pop %r9
    pop %r8
    pop %rbp
    pop %rdi
    pop %rsi
    pop %rdx
    pop %rcx
    pop %rbx
    pop %rax

    // put user stack pointer back where it belongs
    mov %gs:UserStackOffset, %rsp

    swapgs
    sysret
