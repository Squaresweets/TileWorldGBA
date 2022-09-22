/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include <string.h>
#include <stdio.h>

extern uint32_t __flash_binary_end;
//char *p = (char *)(XIP_BASE+ (256 * 1024));
char *p = (char *)(0x10004800);

#define PICO_DEFAULT_LED_PIN 25
int main() {
    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint32_t memAddress = (((uint32_t)&__flash_binary_end)&0xFFFFFF00)+0x00000100;
    char *p = (char *)memAddress;

    //if (strcmp(p,"Hello World!")) {
    if (*p == 0x65) {
        gpio_put(LED_PIN, 1);
    }
    while(1);
}
