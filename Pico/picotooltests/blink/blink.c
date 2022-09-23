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

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

    uint32_t memAddress = (((uint32_t)&__flash_binary_end)&0xFFFFFF00)+0x00000100;
    char *p = (char *)memAddress;
	char delim[] = "\n";
    char *ptr = strtok(p, delim);
    sleep_ms(5000);
	while(ptr != NULL)
	{
		printf("'%s'\n", ptr);
		ptr = strtok(NULL, delim);
	}
    while(1);

    char *ssid;
    strcpy(ptr, ssid);
    ptr = strtok(NULL, delim);
    char *password;
    strcpy(ptr, password);

    while(1)
    {
        printf("ssid: %s\n", ssid);
        printf("password: %s\n", password);
        sleep_ms(1000);
    }
    while(1);
}
