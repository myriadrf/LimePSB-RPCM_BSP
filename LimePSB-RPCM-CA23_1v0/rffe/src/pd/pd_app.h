#ifndef PD_APP_H
#define PD_APP_H

#define RPI_SPI3_SCLK "GPIO7"
#define RPI_SPI3_MISO "GPIO5"
#define RPI_SPI3_SS0  "GPIO4"
#define RPI_SPI3_SS1  "GPIO25"
#define RFPD_EN       "RFPD_EN"
#define NBITS         12

//sudo apt install libgpiod-dev gpiod
//https://wiki.friendlyelec.com/wiki/index.php/GPIO_Programming:_Using_the_libgpiod_Library
//https://libgpiod-dlang.dpldocs.info/gpiod.html
//https://libgpiod.readthedocs.io/en/latest/
#include <gpiod.h>

typedef struct {
    int init;
    int val;
    char chip_name[32];
    struct gpiod_chip *chip;
    struct gpiod_line *cs, *clk, *miso;
} PDSPI_t;

int pd_read(unsigned int channel);

#endif // PD_APP_H
