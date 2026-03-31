#include "isr.h"
#include "../../../terminal/terminal.h"

void isr_handler(registers_t regs)
{
    print("recieved interrupt: intno: ");
    print_hex(regs.int_no);
    print(" errno: "); print_hex(regs.err_code);
    print("\nSegments: CS: "); print_int(regs.cs); print(" SS: "); print_int(regs.ss); print(" DS: "); print_int(regs.ds);
    print("\n");
} 