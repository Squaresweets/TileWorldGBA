/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "pico/stdlib.h"
#include "hardware/flash.h"
#include <string.h>

char *p = (char *)(XIP_BASE+ (256 * 1024));

#define PICO_DEFAULT_LED_PIN 25
int main() {
    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    //if (strcmp(p,"Hello World!")) {
    if (*p == 0x48) {
        gpio_put(LED_PIN, 1);
    }
    while(1);
}
