#pragma once

#include "Paging.h"
#include <cstdint>

template<bool B, class T = void>
struct enable_if {};
 
template<class T>
struct enable_if<true, T> { typedef T type; };

namespace PhysicalMemoryManagement
{

    static constexpr uint64_t PageShift = 12;
    static constexpr uint64_t PageSize = 1UL << PageShift;
    extern uint64_t HigherHalfDirectMapOffset;


    enum class AddressKind {
        DirectPhysical,
        HigherHalfVirtualMappedToPhysical,
    };

    template<AddressKind T>
    struct Address
    {
        uint64_t Address;

        inline constexpr bool nonNull() const
        {
            return Address != 0;
        }
        inline constexpr void* asPtr() const
        {
            return (void*)Address;
        }
    };
    using HHVAddress = Address<AddressKind::HigherHalfVirtualMappedToPhysical>;
    using PAddress = Address<AddressKind::DirectPhysical>;

    inline PAddress toPhysical(HHVAddress addr)
    {
        return PAddress{addr.Address - HigherHalfDirectMapOffset};
    }
    inline HHVAddress toVirtual(PAddress addr)
    {
        return HHVAddress{addr.Address + HigherHalfDirectMapOffset};
    }

    void init();
    void thereIsNowMoreThanOneProcessor();

    void freeRange(void* start, void* end);
    void freePage(uintptr_t v);
    HHVAddress allocatePage();

    inline uint64_t physicalToVirtual(uint64_t addr) { return addr + HigherHalfDirectMapOffset; };
    inline uint64_t virtualToPhysical(uint64_t addr) { return addr - HigherHalfDirectMapOffset; };

};
