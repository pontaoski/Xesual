#include "MiscFunctions.h"

extern "C"
{

void* memset(void* b, int val, std::size_t count) {
    asm volatile ("cld; rep stosb" : "+c" (count), "+D" (b) : "a" (val) : "memory");
    return b;
}

void* memcpy(void* dstptr, const void* srcptr, size_t size) {
	unsigned char* dst = (unsigned char*) dstptr;
	const unsigned char* src = (const unsigned char*) srcptr;
	for (size_t i = 0; i < size; i++)
		dst[i] = src[i];
	return dstptr;
}

}