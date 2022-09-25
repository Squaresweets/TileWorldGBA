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

#define PICO_DEFAULT_LED_PIN 25
int main() {
    stdio_init_all();

    //Get the address where the binary ends, and then get the next section
    char *p = (char *)((((uint32_t)&__flash_binary_end)&0xFFFFFF00)+0x00000100);
    //Convert this to a char[] (since a char* doesn't seem to work with strtok)
    char str[strlen(p)]; strcpy(str, p);
    //Get ssid and password
    char *ssid = strtok(str, "\r\n");
    char *password = strtok(NULL, "\r\n");
    while(1)
    {
        printf("ssid:%s, password:%s\n", ssid, password);
        sleep_ms(1000);
    }
    return 0;
}
