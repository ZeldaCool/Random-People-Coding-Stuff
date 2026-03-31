#ifndef PANIC_H
#define PANIC_H

#include "../terminal/terminal.h"

static inline void PANIC(char* str) {
    
    while (1) HALT();
}

#endif