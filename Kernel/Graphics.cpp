#include "Graphics.h"
#include "PCI.h"
#include "PhysicalMemoryManagement.h"

namespace Graphics
{

BochsDisplayDevice::BochsDisplayDevice(uint32_t pciID) : m_pciID(pciID)
{
}

inline uint16_t* addrOf(uint32_t dev, DispiIndex idx)
{
    const auto mmioField = PCI::readField<4>(dev, PCI::Bar2);
    const auto mmioAddr = PhysicalMemoryManagement::physicalToVirtual(mmioField);
    const auto bochsAddr = reinterpret_cast<uint16_t*>(mmioAddr + 0x500);
    return reinterpret_cast<uint16_t*>((uintptr_t)bochsAddr + (idx<<1));
}

void* BochsDisplayDevice::framebufferAddress() const
{
    const auto bar = PCI::readField<4>(m_pciID, PCI::Bar0);
    return (void*)PhysicalMemoryManagement::physicalToVirtual(bar);
}

uint16_t BochsDisplayDevice::read(DispiIndex idx) const
{
    return *addrOf(m_pciID, idx);
}

void BochsDisplayDevice::write(DispiIndex idx, uint16_t val)
{
    *addrOf(m_pciID, idx) = val;
}

}