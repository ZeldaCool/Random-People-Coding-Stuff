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
        // Zero the new table
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

    // Enable paging in CR0
    uint32_t cr0;
    asm volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile("mov %0, %%cr0" :: "r"(cr0));
}

//ember2819: page fault handler — prints fault info and halts
void page_fault(registers_t regs)
{
    uint32_t fault_addr;
    asm volatile("mov %%cr2, %0" : "=r"(fault_addr));

    // The error code tells us what kind of fault
    int present  = !(regs.err_code & 0x1); // page not present
    int rw       =   regs.err_code & 0x2;  // write attempted
    int us       =   regs.err_code & 0x4;  // user mode
    int reserved =   regs.err_code & 0x8;  // reserved bits overwritten
    
    // We don't have formatted print here so just halt with PANIC
    (void)present; (void)rw; (void)us; (void)reserved; (void)fault_addr;
    PANIC("Page fault!");
}

//ember2819
void initialise_paging()
{
    // Assume 16 MB of physical RAM for now
    uint32_t mem_end_page = 0x1000000;
    nframes = mem_end_page / 0x1000;

    // Allocate the frames bitset (one bit per frame)
    frames = (uint32_t *)kmalloc(INDEX_FROM_BIT(nframes) * sizeof(uint32_t));
    uint8_t *fp = (uint8_t *)frames;
    for (uint32_t i = 0; i < INDEX_FROM_BIT(nframes) * sizeof(uint32_t); i++) fp[i] = 0;

    // Allocate a page directory
    uint32_t phys;
    kernel_directory = (page_directory_t *)kmalloc_int(sizeof(page_directory_t), 1, &phys);
    uint8_t *dp = (uint8_t *)kernel_directory;
    for (uint32_t i = 0; i < sizeof(page_directory_t); i++) dp[i] = 0;
    kernel_directory->physicalAddr = phys;

    // Identity-map everything from 0 up to placement_address so the kernel
    // can keep running at the same addresses after paging is enabled
    uint32_t i = 0;
    while (i < placement_address + 0x1000) {
        alloc_frame(get_page(i, 1, kernel_directory), 1, 0); // kernel, read-only
        i += 0x1000;
    }

    // Register the page fault handler (ISR 14)
    // (isr_register is declared in isr.h — call it if your IRQ system supports it)
    // isr_register(14, page_fault);  // uncomment once syscall/ISR dispatch is wired

    switch_page_directory(kernel_directory);
}
