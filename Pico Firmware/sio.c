#include "sio.h"
#include "hardware/pio.h"
#include "pio/pio_spi.h"
#include "pico/stdlib.h"

pio_spi_inst_t* sio_spi;
//Read 4 bytes and write four bytes to the GBA
uint32_t rw4(uint32_t data)
{
    //If we are only reading, we send 0x0000 which is just sorted out by the GBA
    uint32_t rx;
    for(int i = 0; i < 4; i++)
        pio_spi_write8_read8_blocking(sio_spi, ((uint8_t*)&data)+i, ((uint8_t*)&rx)+i, 1);
    return rx;
}