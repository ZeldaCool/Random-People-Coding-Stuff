//#include "kernel.h"
#include "drivers/tables/idt/idt.h"
#include "drivers/tables/idt/idt.h"
#include "drivers/tables/irq/irq.h"
#include "drivers/tables/isr/isr.h"
#include "drivers/tables/timer/timer.h"
#include "drivers/vga.h"
#include "drivers/keyboard.h"
#include "drivers/drives.h" // clangd mark this as an error
#include "layouts/kb_layouts.h"
#include "terminal/terminal.h"
#include "commands.h"       // Included by Ember2819: Adds commands
#include "colors.h"         // Added by MorganPG1 to centralise colors into one file
#include "users/users.h"    // ember2819: user & permission system
#include "drivers/tables/gdt/gdt.h"
#include "paging/paging.h" //ember2819
#include <stdint.h>

void process_input(unsigned char *buffer) {
    run_command(buffer, TERM_COLOR);
}

static void kmain();

__attribute__((section(".text.entry")))
void _entry() {
    // Initialise display
    vga_clear(TERM_COLOR);
    printc("----- GeckoOS v1.1 -----\n", TERM_COLOR);
    printc("Built by random people on the internet.\n", TERM_COLOR);

    // Setup keyboard layouts
    set_layout(LAYOUTS[0]);

    init_gdt();
    init_idt();
    irq_install();
    timer_install();
    timer_phase(50);

    initialise_paging(); //ember2819: identity-maps kernel and enables paging


    drives_init();
    users_init();
    printc("User system initialised. Default accounts: root / guest\n", VGA_COLOR_LIGHT_GREY);

    kmain();
}

static void kmain()
{
    get_kdrive(0);

    do_login_prompt();

    while (1) {
        // Build the prompt: "username> "
        user_t *u = users_current();
        if (u) {
            uint8_t pcolor = (u->ring == RING_ADMIN) ? VGA_COLOR_LIGHT_RED : PROMPT_COLOR;
            printc(u->name, pcolor);
            printc("> ", pcolor);
        } else {
            // Shouldn't reach here, but be safe
            printc("> ", PROMPT_COLOR);
        }

        unsigned char buff[512];
        input(buff, 512, TERM_COLOR);
        process_input(buff);
    }
}
