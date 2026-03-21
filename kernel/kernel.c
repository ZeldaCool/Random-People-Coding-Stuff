#include "kernel.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "mem.h"

void _entry()
{
    uint8_t term_color = vga_entry_color(VGA_COLOR_WHITE, VGA_COLOR_BLUE);
    vga_clear(term_color);
    //vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    printf("----- COMMUNITY OS v0.1 -----\n", term_color);
    printf("Built by random people on the internet.\n", term_color);
    
    // input loop for testing


    // AMERICAN US QWERTY KEYBOARD
    char DOWNCASE[128] = {
        0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,
        'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\',
        'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, ' '
        // Rest unprintable
    };
    char UPPERCASE[128] = {
        0, 27, '1', '2', '3', '4','5','6','7','8','9','0','-','=','\b','\t',
        'Q','W','E','R','T','Y','U','I','O','P','{','}', '\n', 0,
        'A','S','D','F','G','H','J','K','L',':','"', '~', 0, '|',
        'Z','X','C','V','B','N','M','<','>','?', 0, '*', 0, ' '
        // Rest unprintable
    };

    set_layout(DOWNCASE, UPPERCASE);

    while (true) {
        scancode_t sc = ps2_kb_wfi();
        putchar(scancode_to_ascii(sc), term_color);
    }

    /* Hangs forever */
    //for (;;) {}

    // ^ do NOT do that. it will use too much of the CPU. do this instead
    asm volatile ("hlt");
}
