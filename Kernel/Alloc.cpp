#include "Alloc.h"
#include "MiscFunctions.h"
#include "Spinlock.h"

namespace Alloc
{

struct FreeChunk {
    FreeChunk* next;
    FreeChunk* prev;
    uint64_t numBytes;
};

struct UnfreeChunk {
    uint64_t numBytes;
};

struct {
    Spinlock lock;
    FreeChunk* freeList;
} Memory;

void init()
{
    Memory.lock.init();
}

void* allocate(uint64_t size)
{
    Memory.lock.acquire();
    auto chunk = (FreeChunk*)Memory.freeList;

    const auto actualSize = size + sizeof(UnfreeChunk);

    while (chunk != nullptr) {
        if (chunk->numBytes >= actualSize) {
            chunk->numBytes -= actualSize;

            if (chunk->numBytes == 0) {
                if (chunk->prev)
                    chunk->prev->next = chunk->next;
                if (chunk->next)
                    chunk->next->prev = chunk->prev;
                if (chunk == Memory.freeList)
                    Memory.freeList = chunk->next;
            }

            Memory.lock.release();
            return (void*)((uint64_t)chunk + chunk->numBytes + sizeof(UnfreeChunk));
        }
        chunk = chunk->next;
    }
    return nullptr;
}

void free(void* mem)
{
    Memory.lock.acquire();

    auto base = (uint64_t)mem - sizeof(UnfreeChunk);
    auto numBytes = ((UnfreeChunk*)base)->numBytes;
    auto self = (FreeChunk*)base;
    memset((void*)self, 0, numBytes);
    self->numBytes = numBytes;

    if (Memory.freeList == nullptr) {
        Memory.freeList = self;
        Memory.lock.release();
        return;
    }

    FreeChunk* prev = nullptr;
    FreeChunk* next = (FreeChunk*)Memory.freeList;
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
        if (base + numBytes == (uintptr_t)self->next) {
            self->numBytes += self->next->numBytes;
            self->next = self->next->next;
        }
    }

    self->prev = prev;
    if (self->prev != nullptr) {
        self->prev->next = self;
        if ((uintptr_t)self->prev + self->prev->numBytes == base) {
            prev->numBytes += numBytes;
            self = prev;
        }
    }

    Memory.lock.release();
}

}

