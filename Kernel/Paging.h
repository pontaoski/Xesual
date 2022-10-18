#pragma once

#include <cstdint>

union PageTableEntry {
    struct {
        uint64_t isPresent : 1;
        uint64_t isWritable : 1;
        uint64_t isUserAccessible : 1;
        uint64_t writeThrough : 1;
        uint64_t cacheDisabled : 1;
        uint64_t hasBeenAccessed : 1;
        uint64_t _available1 : 1;
        uint64_t pageSize : 1;
        /// if isGlobal is true, this won't be evicted from TLB on cr3 reload
        uint64_t isGlobal : 1;
        uint64_t _available2 : 3;
        uint64_t physicalAddress : 28;
        uint64_t _reserved : 12;
        uint64_t _available3 : 11;
        uint64_t executionDisabled : 1;
    } hl;
    uint64_t raw;
};

using PageTableRef = PageTableEntry*;
