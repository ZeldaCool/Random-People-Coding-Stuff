#ifndef  KB_LAYOUTS_H
#define KB_LAYOUTS_H

typedef struct {
    char lower[128];
    char upper[128];
} KeyboardLayout;

extern KeyboardLayout LAYOUTS[];

#endif