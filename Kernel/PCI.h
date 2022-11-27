#pragma once

#include <cstdint>

namespace PCI
{

enum Field {
    VendorID = 0x00,
    DeviceID = 0x02,
    Command = 0x04,
    Status = 0x06,
    RevisionID = 0x08,

    ProgIF = 0x09,
    Subclass = 0x0A,
    Class = 0x0B,
    CacheLineSize = 0x0C,
    LatencyTimer = 0x0D,
    HeaderType = 0x0E,
    Bist = 0x0F,
    Bar0 = 0x10,
    Bar1 = 0x14,
    Bar2 = 0x18,
    Bar3 = 0x1C,
    Bar4 = 0x20,
    Bar5 = 0x24,

    InterruptLine = 0x3C,
    InterruptPin = 0x3D,

    SecondaryBus = 0x19,
};

enum HeaderType {
    HeaderDevice = 0,
    HeaderBridge = 1,
    HeaderCardBus = 2,
};

enum Type {
    Bridge = 0x604,
    Sata = 0x106,
};

constexpr uint16_t AddressPort = 0xCF8;
constexpr uint16_t ValuePort = 0xCFC;
constexpr uint32_t PCINull = 0xFFFF;

constexpr inline uint32_t busOf(uint32_t device)
{
    return (uint8_t)(device >> 16);
}

constexpr inline uint32_t slotOf(uint32_t device)
{
    return (uint8_t)(device >> 8);
}

constexpr inline uint32_t funcOf(uint32_t device)
{
    return (uint8_t)(device);
}

constexpr inline uint32_t addressOf(uint32_t device, int field)
{
    return 0x80000000
        | (busOf(device) << 16)
        | (slotOf(device) << 11)
        | (funcOf(device) << 8)
        | ((field) & 0xFC);
}

constexpr inline uint32_t box(int bus, int slot, int func)
{
    return (uint32_t)((bus<<16) | (slot<<8) | func);
}

inline void outl(uint16_t port, uint32_t data)
{
    asm volatile ("outl %%eax, %%dx" :: "dN"(port), "a"(data));
}

inline uint32_t inl(uint16_t port)
{
    uint32_t ret;
    asm volatile ("inl %%dx, %%eax" : "=a"(ret) : "dN"(port));
    return ret;
}

inline uint16_t ins(uint16_t port)
{
    uint16_t ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

inline void writeField(uint32_t device, int field, int size, uint32_t value)
{
    (void)size;

    outl(AddressPort, addressOf(device, field));
    outl(ValuePort, value);
}

template<int size>
uint32_t readField(uint32_t device, int field)
{
    outl(AddressPort, addressOf(device, field));
    if constexpr (size == 1) {
        return inb(ValuePort + (field & 0b11));
    } else if constexpr (size == 2) {
        return ins(ValuePort + (field & 0b10));
    } else if constexpr (size == 4) {
        return inl(ValuePort);
    } else {
        return 0xFFFF;
    }
}

using ScanCallback = void(*)(uint32_t device, uint16_t vendorID, uint16_t deviceID, void* context);

void scan(ScanCallback cb, void* context, int type = -1);

}