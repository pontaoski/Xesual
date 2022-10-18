#include "Spinlock.h"

void Spinlock::init()
{
    held = 0;
}

void Spinlock::acquire()
{
    while (__sync_lock_test_and_set(&held, 1) != 0);

    __sync_synchronize();
}

void Spinlock::release()
{
    __sync_synchronize();

    asm volatile("movl $0, %0" : "+m" (held) : );
}
