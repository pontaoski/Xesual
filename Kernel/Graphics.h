#pragma once

#include <cstdint>

namespace Graphics
{

enum DispiIOPort {
    Index = 0x01CE,
    Data = 0x01CF,
};

enum DispiIndex {
    ID = 0x0,
    XRes = 0x1,
    YRes = 0x2,
    BPP = 0x3,
    Enable = 0x4,
    Bank = 0x5,
    VirtWidth = 0x6,
    VirtHeight = 0x7,
    XOffset = 0x8,
    YOffset = 0x9,
    VideoMemory64k = 0xa,
};

enum DispiID {
    Zero = 0xB0C0,
    One = 0xB0C1,
    Two = 0xB0C2,
    Three = 0xB0C3,
    Four = 0xB0C4,
    Five = 0xB0C5,
};

enum DispiFlags {
    Disabled = 0x00,
    Enabled = 0x01,
    GetCaps = 0x02,
    EightBitDAC = 0x20,
    LFBEnabled = 0x40,
    NoClearMem = 0x80,
};

class BochsDisplayDevice {
    uint32_t m_pciID;
public:
    explicit BochsDisplayDevice(uint32_t pciID);

    void* framebufferAddress() const;
    uint16_t read(DispiIndex idx) const;
    void write(DispiIndex, uint16_t val);
};

};
