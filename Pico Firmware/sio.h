#ifndef SIO_H
#define SIO_H
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "pio/pio_spi.h"

uint32_t rw4(uint32_t data);
extern pio_spi_inst_t* sio_spi;
#endif