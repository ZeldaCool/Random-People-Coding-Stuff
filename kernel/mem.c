#include <stdint.h>

void* memcpy(void* dest, const void* src, unsigned long n) {
    // n = Number of bytes

    unsigned char* d = dest;
    const unsigned char* s = src;

    // Iterate n times, copying the byte in s into the same index in d
    for (unsigned long i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}

// [Ember2819: BEGIN - memset implementation]
void* memset(void* dest, int val, unsigned long n) {
    unsigned char* d = dest;
    for (unsigned long i = 0; i < n; i++) {
        d[i] = (unsigned char)val;
    }
    return dest;
}
// [Ember2819: END]

// Pumpkicks
int strlen(char* ptr) {
    int i = 0;
    while (ptr[i]) i++;
    return i;
}

//replace with real allocator later but should be fine for now
//kotofyt: it is not
extern uint32_t _kernel_end;
uint32_t placement_address = (uint32_t)&_kernel_end;

uint32_t kmalloc_int(uint32_t sz, int align, uint32_t *phys)
{
    if (align == 1 && (placement_address & 0xFFFFF000)) // If the address is not already page-aligned
    {
        // Align it.
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000;
    }
    if (phys)
    {
        *phys = placement_address;
    }
    uint32_t tmp = placement_address;
    placement_address += sz;
    return tmp;
}
void* kmalloc(uint32_t size) {
    return (void*)kmalloc_int(size, 0, 0);
}