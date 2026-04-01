#include <stdint.h>
#include "paging.h"
#include "../mem.h"
#include "../drivers/panic.h"

uint32_t *frames;
uint32_t nframes;

extern uint32_t placement_address;

#define INDEX_FROM_BIT(a)  ((a) / 32)
#define OFFSET_FROM_BIT(a) ((a) % 32)

static void set_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr / 0x1000;
    frames[INDEX_FROM_BIT(frame)] |= (1u << OFFSET_FROM_BIT(frame));
}

static void clear_frame(uint32_t frame_addr)
{
    uint32_t frame = frame_addr / 0x1000;
    frames[INDEX_FROM_BIT(frame)] &= ~(1u << OFFSET_FROM_BIT(frame));
}

static uint32_t first_frame()
{
    for (uint32_t i = 0; i < INDEX_FROM_BIT(nframes); i++) {
        if (frames[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                if (!(frames[i] & (1u << j)))
                    return i * 32 + j;
            }
        }
    }
    return (uint32_t)-1;
}

void alloc_frame(page_t *page, int is_kernel, int is_writeable)
{
    if (page->frame != 0) return;

    uint32_t idx = first_frame();
    if (idx == (uint32_t)-1)
        PANIC("No free frames!");

    set_frame(idx * 0x1000);
    page->present = 1;
    page->rw      = is_writeable ? 1 : 0;
    page->user    = is_kernel   ? 0 : 1;
    page->frame   = idx;
}

void free_frame(page_t *page)
{
    if (!page->frame) return;
    clear_frame(page->frame * 0x1000);
    page->frame = 0;
}

page_directory_t *kernel_directory = 0;

page_t *get_page(uint32_t address, int make, page_directory_t *dir)
{
    address /= 0x1000;
    uint32_t table_idx = address / 1024;

    if (dir->tables[table_idx])
        return &dir->tables[table_idx]->pages[address % 1024];

    if (make) {
        uint32_t tmp;
        dir->tables[table_idx] = (page_table_t *)kmalloc_int(sizeof(page_table_t), 1, &tmp);
        uint8_t *p = (uint8_t *)dir->tables[table_idx];
        for (uint32_t i = 0; i < sizeof(page_table_t); i++) p[i] = 0;
        dir->tablesPhysical[table_idx] = tmp | 0x7; // PRESENT | RW | USER
        return &dir->tables[table_idx]->pages[address % 1024];
    }

    return 0;
}

void switch_page_directory(page_directory_t *dir)
{
    uint32_t phys = dir->physicalAddr;
    asm volatile("mov %0, %%cr3" :: "r"(phys));

    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

void page_fault(registers_t regs)
{
    uint32_t fault_addr;
    asm volatile("mov %%cr2, %0" : "=r"(fault_addr));
    (void)regs; (void)fault_addr;
    PANIC("Page fault!");
}

void initialise_paging()
{
    uint32_t mem_end_page = 0x1000000; // 16 MB
    nframes = mem_end_page / 0x1000;

    // Allocate + zero the frame bitset
    frames = (uint32_t *)kmalloc(INDEX_FROM_BIT(nframes) * sizeof(uint32_t));
    uint8_t *fp = (uint8_t *)frames;
    for (uint32_t i = 0; i < INDEX_FROM_BIT(nframes) * sizeof(uint32_t); i++) fp[i] = 0;

    // Allocate + zero the kernel page directory
    uint32_t phys;
    kernel_directory = (page_directory_t *)kmalloc_int(sizeof(page_directory_t), 1, &phys);
    uint8_t *dp = (uint8_t *)kernel_directory;
    for (uint32_t i = 0; i < sizeof(page_directory_t); i++) dp[i] = 0;
    kernel_directory->physicalAddr = phys;

    // Snapshot placement_address AFTER the setup allocations above.
    // get_page(make=1) calls kmalloc_int internally during the mapping loop,
    // allocating page tables — if we check placement_address live those
    // allocations can fall outside the mapped region and triple-fault.
    // Padding 1MB gives room for all page tables the loop will create.
    //ember2819
    uint32_t map_end = placement_address + 0x100000;

    // Identity-map 0 → map_end so the kernel keeps running at the same
    // physical addresses after the PG bit is set
    for (uint32_t i = 0; i < map_end; i += 0x1000) {
        alloc_frame(get_page(i, 1, kernel_directory), 1, 0);
    }

    switch_page_directory(kernel_directory);
}
