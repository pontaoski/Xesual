#include "VirtualMemoryManagement.h"
#include "PhysicalMemoryManagement.h"
#include "MiscFunctions.h"

namespace VirtualMemoryManagement
{

using namespace PhysicalMemoryManagement;

PageTableRef KernelPageTable = nullptr;


bool allocatePageForTableEntry(PageTableEntry* entry)
{
    auto frame = allocatePage();
    if (!frame.nonNull())
        return false;

    entry->hl.physicalAddress = toPhysical(frame).Address >> PageShift;
    entry->hl.pageSize = 0;
    entry->hl.isPresent = 1;
    entry->hl.isWritable = 1;
    entry->hl.isUserAccessible = 1;
    entry->hl.cacheDisabled = 0;
    entry->hl.writeThrough = 0;
    entry->hl.executionDisabled = 0;

    return true;
}

void freePageForTableEntry(PageTableEntry* entry)
{
    freePage(entry->hl.physicalAddress << PageShift);
    entry->hl.physicalAddress = 0;
    entry->hl.isPresent = 0;
}

PageTableRef allocateRootPageTable()
{
    auto frame = allocatePage();
    if (!frame.nonNull())
        return nullptr;

    memset(frame.asPtr(), 0, PageSize);
    return (PageTableRef)(frame.asPtr());
}

bool allocatePageTableForTableEntry(PageTableEntry* entry)
{
    if (!allocatePageForTableEntry(entry))
        return false;

    memset((void*)((uint64_t)entry->hl.physicalAddress << (uint64_t)PageShift), 0, PageSize);

    return true;
}

void map(PageTableRef root, uint64_t virtualAddress, uint64_t toPhysical)
{
    uintptr_t pageAddress = (virtualAddress & 0xFFFFffffFFFFUL) >> PageShift;

    unsigned int level4Index = (pageAddress >> 27) & 0x1FF;
    unsigned int level3Index = (pageAddress >> 18) & 0x1FF;
    unsigned int level2Index = (pageAddress >> 9)  & 0x1FF;
    unsigned int level1Index = (pageAddress) & 0x1FF;

    constexpr auto pe2v = [](PageTableEntry p) {
        return (PageTableRef)((p.hl.physicalAddress << PageShift) + HigherHalfDirectMapOffset);
    };

    if (!root[level4Index].hl.isPresent) {
        allocatePageTableForTableEntry(&root[level4Index]);
    }

    auto level3Address = pe2v(root[level4Index]);
    if (!level3Address[level3Index].hl.isPresent) {
        allocatePageTableForTableEntry(&level3Address[level3Index]);
    }
    auto level2Address = pe2v(level3Address[level3Index]);
    if (!level2Address[level2Index].hl.isPresent) {
        allocatePageTableForTableEntry(&level2Address[level2Index]);
    }
    auto level1Address = pe2v(level2Address[level2Index]);

    level1Address[level1Index].hl.physicalAddress = (toPhysical >> PageShift);
    level1Address[level1Index].hl.isPresent = true;
    level1Address[level1Index].hl.isWritable = true;
    level1Address[level1Index].hl.isUserAccessible = true;
}

PageTableRef shallowCopy(PageTableRef root)
{
    auto addr = allocatePage();
    memcpy(addr.asPtr(), root, PageSize);
    return (PageTableRef)addr.asPtr();
}

}