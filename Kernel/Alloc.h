#pragma once

#include <cstdint>

namespace Alloc
{

void init();
void* allocate(uint64_t size);
void free(void* mem);

}
