/// This file implements the _putchar function that our nested printf library relies on.
/// For debugging purposes.

#include <cstdint>

static inline void outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

extern "C" void _putchar(char character)
{
    if (character == '\n') {
        outb(0x3F8, '\r');
    }
    outb(0x3F8, character);

}