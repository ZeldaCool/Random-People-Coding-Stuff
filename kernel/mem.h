// bonk enjoyer (dorito girl)

#ifndef _MEM_H
#define _MEM_H

#include <stdint.h>

void* memcpy(void* dest, const void* src, unsigned long n);

//Ember2819
void* memset(void* dest, int val, unsigned long n);
uint32_t kmalloc_int(uint32_t sz, int align, uint32_t *phys);
void* kmalloc(uint32_t size);

//Pumpkicks
int strlen(char* ptr);

typedef unsigned int size_t;
typedef int ssize_t;

#endif
