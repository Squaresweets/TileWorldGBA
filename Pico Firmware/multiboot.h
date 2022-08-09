#ifndef MULTIBOOT_H
#define MULTIBOOT_H
#include "pico/stdlib.h"

int multiboot(uint8_t* rom, uint64_t len);

#endif