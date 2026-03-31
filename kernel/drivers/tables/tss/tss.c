#include <stdint.h>
#include "tss.h"
#include "../gdt/gdt.h"

tss_t tss;

void tss_gate() {
    const uintptr_t tss_base = (uintptr_t)&tss;
    const uintptr_t tss_limit = sizeof(tss) - 1;

    // TSS - low
    gdt_entries[5].limit_low    = tss_limit;                                      // Limit (15:0)
    gdt_entries[5].base_middle  = (uint16_t)tss_base;                             // Base (15:0)
    gdt_entries[5].flags1   = (uint16_t)(0xE900 + ((tss_base >> 16) & 0xFF)); // P + DPL 3 + TSS + base (23:16)
    gdt_entries[5].flags2   = (uint16_t)((tss_base >> 16) & 0xFF00);          // Base (31:24)

    // TSS - high
    gdt_entries[6].limit    = (uint16_t)(tss_base >> 32);                     // Base (47:32)
    gdt_entries[6].base     = (uint16_t)(tss_base >> 48);                     // Base (63:32)
    gdt_entries[6].flags1   = 0x0000;
    gdt_entries[6].flags2   = 0x0000;
}