/* By Ember2819 */

ENTRY(_entry)

SECTIONS
{
    . = 0x00100000;

    _kernel_start = .;

    .text ALIGN(4K) : {
        *(.text.entry)
        *(.text*)
    }

    .rodata ALIGN(4K) : { *(.rodata*) }
    .data   ALIGN(4K) : { *(.data*)   }
    .bss    ALIGN(4K) : {
        *(.bss*)
        *(COMMON)
    }

    _kernel_end = .;
}
