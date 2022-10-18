#include "PhysicalMemoryManagement.h"
#include "Paging.h"
#include "Spinlock.h"
#include "MiscFunctions.h"
#include "printf.h"

namespace PhysicalMemoryManagement
{

uint64_t HigherHalfDirectMapOffset = 0;

struct MemoryChunk {
    MemoryChunk* next;
};

struct {
    Spinlock lock;
    bool usingLock;
    MemoryChunk* freeList;
} Memory;

void init()
{
    Memory.lock.init();
    Memory.usingLock = false;
    Memory.freeList = nullptr;
}

void freeRange(void* start, void* end)
{
    for (
        uintptr_t p = (uintptr_t)start;
        p + PageSize < (uintptr_t)end;
        p += PageSize
    ) {
        freePage(p);
    }
}

void freePage(uintptr_t v)
{
    /// Overwrite the page with gibberish for security reasons,
    /// as well as to make it clear that the page is unused.
    memset((void*)v, 1, PageSize);

    if (Memory.usingLock)
        Memory.lock.acquire();

    MemoryChunk* chunk = (MemoryChunk*)v;
    chunk->next = Memory.freeList;
    Memory.freeList = chunk;

    if (Memory.usingLock)
        Memory.lock.release();
}

HHVAddress allocatePage()
{
    if (Memory.usingLock)
        Memory.lock.acquire();

    MemoryChunk* chunk = (MemoryChunk*)Memory.freeList;
    if (chunk)
        Memory.freeList = chunk->next;

    if (Memory.usingLock)
        Memory.lock.release();

    return HHVAddress{(uint64_t)chunk};
}

}
