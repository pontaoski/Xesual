/// This file implements the _putchar function that our nested printf library relies on.
/// For debugging purposes.

#include "Spinlock.h"
#include "MiscFunctions.h"
#include <cstdint>

extern "C" void _putchar(char character)
{
    if (character == '\n') {
        outb(0x3F8, '\r');
    }
    outb(0x3F8, character);
}

Spinlock logLock;

extern "C" void initLogLock()
{
    logLock.init();
}

extern "C" void lockLog()
{
    logLock.acquire();
}

extern "C" void unlockLog()
{
    logLock.release();
}
