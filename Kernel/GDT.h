#pragma once

#include "printf.h"
#include <cstddef>
#include <cstdint>
#include <inttypes.h>

namespace GDT
{

enum DescriptorPrivilegeLevel: uint8_t {
    NullDPL = 0,
    Ring0 = 0,
    Ring1 = 1,
    Ring2 = 2,
    Ring3 = 3,
};

enum ReadableWriteable {
    NullRW = 0,
    CodeDenyRead = 0,
    CodeAllowRead = 1,
    DataDenyWrite = 0,
    DataAllowWrite = 1,
};

enum Executable {
    NullSeg = 0,
    DataSeg = 0,
    CodeSeg = 1,
};

struct UserAccessByte {
    bool accessed: 1;
    ReadableWriteable readableWriteable: 1;
    bool directionConforming: 1;
    Executable executable: 1;
    bool isUserSegment: 1;
    DescriptorPrivilegeLevel descriptorPrivilegeLevel: 2;
    bool present: 1;

    constexpr UserAccessByte(ReadableWriteable rw, Executable executable, DescriptorPrivilegeLevel level)
        : accessed(0), readableWriteable(rw), directionConforming(false)
        , executable(executable), isUserSegment(true), descriptorPrivilegeLevel(level)
        , present(true)
    {
    }
    constexpr UserAccessByte()
        : accessed(0), readableWriteable(NullRW), directionConforming(false)
        , executable(NullSeg), isUserSegment(0), descriptorPrivilegeLevel(NullDPL), present(false)
    {
    }
} __attribute__((packed));

enum Type {
    LDT = 0x2,
    AvailableTSS = 0x9,
    BusyTSS = 0xB
};

struct SystemAccessByte {
    Type type: 4;
    bool isUserSegment: 1;
    DescriptorPrivilegeLevel descriptorPrivilegeLevel: 2;
    bool present: 1;

    constexpr SystemAccessByte(Type type, DescriptorPrivilegeLevel dpl)
        : type(type), isUserSegment(false), descriptorPrivilegeLevel(dpl), present(true)
    {
    }
} __attribute__((packed));

enum Granularity: uint8_t {
    NullGran = 0,
    Byte = 0,
    Page = 1,
};
enum Size: uint8_t {
    NullSize = 0,
    Bit16 = 0,
    Bit32 = 1,
};

struct Flags {
    uint8_t reserved: 1;
    bool isLongModeCode: 1;
    Size size: 1;
    Granularity granularity: 1;

    constexpr Flags(bool isLongModeCode, Size size, Granularity granularity)
        : reserved(0), isLongModeCode(isLongModeCode), size(size), granularity(granularity)
    {
    }
    constexpr Flags(Size size, Granularity granularity)
        : reserved(0), isLongModeCode(false), size(size), granularity(granularity)
    {
    }
    constexpr Flags()
        : reserved(0), isLongModeCode(0), size(NullSize), granularity(NullGran)
    {
    }
    Flags(uint8_t bits)
    {
        *reinterpret_cast<uint8_t*>(this) = bits;
    }

    constexpr static Flags longModeCode()
    {
        return Flags(true, NullSize, Byte);
    }
    operator uint8_t()
    {
        return *reinterpret_cast<uint8_t*>(this);
    }
} __attribute__((packed));

struct UserEntry {
    uint16_t limitLow;
    uint16_t baseLow;

    uint8_t baseMid;
    UserAccessByte access;

    uint8_t limitHigh: 4;
    uint8_t flags: 4;
    uint8_t baseHigh;

    constexpr UserEntry(uint32_t limit, uint32_t base, UserAccessByte access, Flags flags)
        : access(access), flags(flags)
    {
        limitLow = limit & 0xFFFF;
        limitHigh = (limit >> 16) & 0xFF;

        baseLow = base & 0xFFFF;
        baseMid = (base >> 16) & 0xFF;
        baseHigh = (base >> 24) & 0xFF;
    }
    void become(uint64_t x)
    {
        *reinterpret_cast<uint64_t*>(this) = x;
    }
    void dump()
    {
        logfn("%" PRIx64 "\n", *this);
        logfn("limitLow: %" PRIx16 "\n", this->limitLow);
        logfn("baseLow: %" PRIx16 "\n", this->baseLow);
        logfn("baseMid: %" PRIx8 "\n", this->baseMid);
        logfn("access: %" PRIx8 "\n", this->access);
        logfn("\t %" PRIx8 "\n", this->access.accessed);
        logfn("\t %" PRIx8 "\n", this->access.readableWriteable);
        logfn("\t %" PRIx8 "\n", this->access.directionConforming);
        logfn("\t %" PRIx8 "\n", this->access.executable);
        logfn("\t %" PRIx8 "\n", this->access.isUserSegment);
        logfn("\t %" PRIx8 "\n", this->access.descriptorPrivilegeLevel);
        logfn("\t %" PRIx8 "\n", this->access.present);
        logfn("limitHigh: %" PRIx8 "\n", this->limitHigh);
        logfn("flags: %" PRIx8 "\n", this->flags);
        logfn("\t %" PRIx8 "\n", Flags(this->flags).reserved);
        logfn("\t %" PRIx8 "\n", Flags(this->flags).isLongModeCode);
        logfn("\t %" PRIx8 "\n", Flags(this->flags).size);
        logfn("\t %" PRIx8 "\n", Flags(this->flags).granularity);
        logfn("baseHigh: %" PRIx8 "\n", this->baseHigh);
    }
} __attribute__((packed));

struct SystemEntry {
    // low
    uint16_t limitLow;
    uint16_t baseLow;

    uint8_t baseMid;
    SystemAccessByte access;

    uint8_t limitHigh: 4;
    uint8_t flags: 4;
    uint8_t baseHigh;

    // high
    uint32_t baseHigher;
    uint32_t reserved;

    constexpr SystemEntry(uint16_t limit, uint64_t base, SystemAccessByte access, Flags granularity)
        : access(access), flags(granularity), reserved(0)
    {
        limitLow = limit & 0xFFFF;
        limitHigh = (limit >> 16) & 0xFF;

        baseLow = base & 0xFFFF;
        baseMid = (base >> 16) & 0xFF;
        baseHigh = (base >> 24) & 0xFF;
        baseHigher = (base >> 32) & 0xFFFFFFFF;
    }
} __attribute__((packed));

struct TSSEntry {
    uint32_t reserved0;
    uint64_t stackPointers[3];
    uint64_t reserved1;
    uint64_t interruptStackTable[7];
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomapBase;
} __attribute__ ((packed));

struct Table {
    UserEntry nullEntry;
    UserEntry kernelCode;
    UserEntry kernelData;
    UserEntry userCode32;
    UserEntry userData32;
    UserEntry userCode64;
    UserEntry userData64;
    SystemEntry theEntryForTheTSS;
    TSSEntry theTSSEntryItself;

    void init(uintptr_t stackLocation)
    {
        nullEntry = UserEntry(0, 0, UserAccessByte(), Flags());
        kernelCode = UserEntry(0xffffffff, 0, UserAccessByte(CodeAllowRead, CodeSeg, Ring0), Flags::longModeCode());
        kernelData = UserEntry(0xffffffff, 0, UserAccessByte(DataAllowWrite, DataSeg, Ring0), Flags(Bit32, Byte));
        userCode32 = UserEntry(0xffffffff, 0, UserAccessByte(CodeAllowRead, CodeSeg, Ring3), Flags(Bit32, Byte));
        userData32 = UserEntry(0xffffffff, 0, UserAccessByte(DataAllowWrite, DataSeg, Ring3), Flags(Bit32, Byte));
        userCode64 = UserEntry(0xffffffff, 0, UserAccessByte(CodeAllowRead, CodeSeg, Ring3), Flags::longModeCode());
        userData64 = UserEntry(0xffffffff, 0, UserAccessByte(DataAllowWrite, DataSeg, Ring3), Flags(Bit32, Byte));
        theEntryForTheTSS = SystemEntry(sizeof(theTSSEntryItself), (uint64_t)&theTSSEntryItself, SystemAccessByte(AvailableTSS, Ring0), Flags());
    }
    void install()
    {
        struct {
            uint16_t limit;
            uintptr_t base;
        } __attribute__((packed)) ptr;

        ptr.limit = offsetof(Table, theTSSEntryItself)-1;
        ptr.base = (uintptr_t)this;

        // load gdt
        asm volatile (
            "mov %0, %%rdi\n"
            "lgdt (%%rdi)\n"

            : : "r"(&ptr)
        );

        // load code segment registers
        asm volatile (R"(
            push %0
            lea (.farJumpDestination), %%rax
            push %%rax
            lretq
        
        .farJumpDestination:
        )" :: "r"(offsetof(Table, kernelCode)));

        // load data segment registers
        asm volatile (
            "mov %0, %%rax\n"
            "mov %%rax, %%ds\n"
            "mov %%rax, %%es\n"
            "mov %%rax, %%ss\n"

            : : "r"(offsetof(Table, kernelData))
        );

        // load the tss
        asm volatile (
            "mov $0x3b, %ax\n"
            "ltr %ax\n"
        );
    }
} __attribute__((packed)) __attribute__((aligned(0x10)));

}