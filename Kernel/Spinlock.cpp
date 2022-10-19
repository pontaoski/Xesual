#include "Spinlock.h"
#include "ProcessManagement.h"

void Spinlock::init()
{
    held = 0;
}

void Spinlock::acquire()
{
    // if we got interrupted here, bad shit could happen
    ProcessManagement::pushCLI();

    while (__sync_lock_test_and_set(&held, 1) != 0);

    __sync_synchronize();
    ProcessManagement::popCLI();
}

void Spinlock::release()
{
    // if we got interrupted here, bad shit could happen
    ProcessManagement::pushCLI();
    __sync_synchronize();

    asm volatile("movl $0, %0" : "+m" (held) : );
    ProcessManagement::popCLI();
}
