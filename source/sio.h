#ifndef SIO_H   /* Include guard */
#define SIO_H
#include "memmap.h"
#include "types.h"
#include "toolbox.h"

#define SIO_SI BIT(2)
#define SIO_SO BIT(3)
#define SIO_START BIT(7)
#define SIO_CLOCK_INTERNAL BIT(0)
#define SIO_TRANSFER_32 BIT(12)

void sioInit();
u32 sioReadWrite(u32 data);

#endif