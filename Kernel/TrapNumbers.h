#pragma once

#define ClockInterrupt 123
#define BadTLBInterrupt 124
#define HaltInterrupt 125
#define NoOpInterrupt 126
#define SyscallInterrupt 128
#define HaltInterrupt2 2

#define LocalAPICAddress _ZN9LocalAPIC5lapicE
#define CONCAT(a, b) a ## b
#define ISRForNumber(num) CONCAT(_isr, num)
