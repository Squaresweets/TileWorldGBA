#ifndef SIO_H
#define SIO_H
#include "pico/stdlib.h"
#include "hardware/pio.h"

uint32_t rw4(uint32_t data);
extern pio_spi_inst_t* sio_spi;
#endif