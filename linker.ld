/* By Ember2819, your welcome dorito girl. Took me way too long to figure out how to make this... */

ENTRY(_entry)

SECTIONS
{
    . = 0x00100000;

    _kernel_start = .;

    .text ALIGN (4K) : AT (ADDR (.text) - 0xC0000000) {
        *(.text.entry)
        *(.*text*)
    }
    
    .rodata ALIGN (4K) : AT (ADDR (.rodata) - 0xC0000000) { *(.rodata) }
    .data ALIGN (4K) : AT (ADDR (.data) - 0xC0000000) { *(.data)   }
    .bss ALIGN (4K) : AT (ADDR (.bss) - 0xC0000000) {
        *(.bss)
        *(COMMON)
        *(.bootstrap_stack)
    }
    
    _kernel_end = .;
}
