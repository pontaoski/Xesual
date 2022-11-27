#include "PCI.h"

namespace PCI
{

uint16_t findType(uint32_t dev)
{
    return (readField<1>(dev, Field::Class)<<8) | readField<1>(dev, Field::Subclass);
}

template<typename CB>
void scanHit(const CB& f, uint32_t dev)
{
    auto vid = readField<2>(dev, Field::VendorID);
    auto did = readField<2>(dev, Field::DeviceID);

    f(dev, vid, did);
}

template<typename CB>
void scanFunc(const CB& f, int type, int bus, int slot, int func)
{
    auto dev = box(bus, slot, func);
    if (type == -1 || type == findType(dev)) {
        scanHit(f, dev);
    }
    if (findType(dev) == Type::Bridge) {
        scanBus(f, type, readField<1>(dev, Field::SecondaryBus));
    }
}

template<typename CB>
void scanSlot(const CB& f, int type, int bus, int slot)
{
    auto dev = box(bus, slot, 0);
    if (readField<2>(dev, Field::VendorID) == PCINull) {
        return;
    }
    scanFunc(f, type, bus, slot, 0);
    if (readField<1>(dev, Field::HeaderType) == 0) {
        return;
    }
    for (int func = 1; func < 8; func++) {
        auto dev = box(bus, slot, func);
        if (readField<2>(dev, Field::VendorID) != PCINull) {
            scanFunc(f, type, bus, slot, func);
        }
    }
}

template<typename CB>
void scanBus(const CB& f, int type, int bus)
{
    for (int slot = 0; slot < 32; ++slot) {
        scanSlot(f, type, bus, slot);
    }
}

template<typename CB>
void scan(const CB& f, int type = -1)
{
    if ((readField<1>(0, Field::HeaderType) & 0x80) == 0) {
        scanBus(f, type, 0);
        return;
    }

    auto hit = false;
    for (int func = 0; func < 8; ++func) {
        auto dev = box(0, 0, func);
        if (readField<2>(dev, Field::VendorID) != PCINull) {
            hit = true;
            scanBus(f, type, func);
        } else {
            break;
        }
    }

    if (!hit) {
        for (int bus = 0; bus < 256; ++bus) {
            for (int slot = 0; slot < 32; ++slot) {
                scanSlot(f, type, bus, slot);
            }
        }
    }
}

void scan(ScanCallback cb, void* context, int type)
{
    scan([context, cb](uint32_t device, uint16_t vid, uint16_t did) {
        cb(device, vid, did, context);
    }, type);
}

}
