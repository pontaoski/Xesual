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
    MemoryChunk* prev;
    uint64_t numPages;
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

void thereIsNowMoreThanOneProcessor()
{
    Memory.usingLock = true;
}

void freeRange(void* start, void* end)
{
    freePages((uintptr_t)start, ((uintptr_t)end-(uintptr_t)start)/PageSize);
}

void freePage(uintptr_t v)
{
    freePages(v, 1);
}

void freePages(uintptr_t base, uint64_t numPages)
{
    if (Memory.usingLock)
        Memory.lock.acquire();

    auto self = (MemoryChunk*)base;
    memset((void*)base, 0, PageSize * numPages);
    self->numPages = numPages;

    if (Memory.freeList == nullptr) {
        Memory.freeList = self;
        if (Memory.usingLock)
            Memory.lock.release();
        return;
    }

    MemoryChunk* prev = nullptr;
    MemoryChunk* next = (MemoryChunk*)Memory.freeList;
    while (next != nullptr) {
        if ((uintptr_t)next > base) {
            break;
        }
        prev = next;
        next = next->next;
    }

    self->next = next;
    if (self->next != nullptr) {
        self->next->prev = self;
        if (base + (PageSize * numPages) == (uintptr_t)self->next) {
            self->numPages += self->next->numPages;
            self->next = self->next->next;
        }
    }

    self->prev = prev;
    if (self->prev != nullptr) {
        self->prev->next = self;
        if ((uintptr_t)self->prev + (PageSize * self->prev->numPages) == base) {
            prev->numPages += numPages;
            self = prev;
        }
    }

    if (Memory.usingLock)
        Memory.lock.release();
}

HHVAddress allocatePages(uint64_t numPages)
{
    if (Memory.usingLock)
        Memory.lock.acquire();

    auto chunk = (MemoryChunk*)Memory.freeList;
    while (chunk != nullptr) {
        if (chunk->numPages >= numPages) {
            chunk->numPages -= numPages;

            if (chunk->numPages == 0) {
                if (chunk->prev)
                    chunk->prev->next = chunk->next;
                if (chunk->next)
                    chunk->next->prev = chunk->prev;
                if (chunk == Memory.freeList)
                    Memory.freeList = chunk->next;
            }

            if (Memory.usingLock)
                Memory.lock.release();
            return HHVAddress{(uint64_t)chunk + (uint64_t)PageSize*chunk->numPages};
        }
    }

    if (Memory.usingLock)
        Memory.lock.release();

    return HHVAddress{0};
}

HHVAddress allocatePage()
{
    return allocatePages(1);
}

}
