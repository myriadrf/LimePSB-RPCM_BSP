#include "pd_app.h"

#include "../helpers/helpers_app.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>

void delay_us(unsigned int us)
{
    struct timespec ts;
    ts.tv_sec = us / 1000000;
    ts.tv_nsec = (us % 1000000) * 1000;
    nanosleep(&ts, NULL);
}

int pd_init(unsigned int channel, PDSPI_t *pdspi)
{
    unsigned int line_num;

    // Find GPIO chip by clock pin. Other pins will belong to the same chip.
    if (find_gpiochip_for_gpio_string(RPI_SPI3_SCLK, pdspi->chip_name, sizeof(pdspi->chip_name), &line_num) < 0)
    {
        perror("gpiochip not found!\n");
        return -1;
    }

    // Open GPIO chip (use gpioinfo to check the correct /dev/gpiochipX)
    pdspi->chip = gpiod_chip_open_by_name(pdspi->chip_name);
    if (!pdspi->chip) {
        perror("Failed to open gpiochip");
        return -1;
    }

    //
    pdspi->cs   = gpiod_chip_find_line(pdspi->chip, channel == 1 ? RPI_SPI3_SS0 : RPI_SPI3_SS1);
    if (!pdspi->cs) return -1;
    pdspi->clk  = gpiod_chip_find_line(pdspi->chip, RPI_SPI3_SCLK);
    if (!pdspi->clk) return -1;
    pdspi->miso = gpiod_chip_find_line(pdspi->chip, RPI_SPI3_MISO);
    if (!pdspi->miso) return -1;

    //
    int res;
    res = gpiod_line_request_output(pdspi->cs, "spi3-cs", 1);
    if (res < 0) return -1;
    res = gpiod_line_request_output(pdspi->clk, "spi3-clk", 0);
    if (res < 0) return -1;
    res = gpiod_line_request_input(pdspi->miso, "spi3-miso");
    if (res < 0) return -1;

    pdspi->init = 1;
    return 0;
}

void pd_close(PDSPI_t *pdspi)
{
    if (pdspi->cs)   gpiod_line_release(pdspi->cs);
    if (pdspi->clk)  gpiod_line_release(pdspi->clk);
    if (pdspi->miso) gpiod_line_release(pdspi->miso);

    if (pdspi->chip) gpiod_chip_close(pdspi->chip);

    pdspi->cs = 0;
    pdspi->clk = 0;
    pdspi->miso = 0;
    pdspi->chip = 0;
    pdspi->init = 0;
}

int pd_convert(PDSPI_t *pdspi)
{
    //
    gpiod_line_set_value(pdspi->cs, 0);
    gpiod_line_set_value(pdspi->cs, 1);
    gpiod_line_set_value(pdspi->clk, 0);
    delay_us(2);           // ~2 µs delay
    gpiod_line_set_value(pdspi->cs, 0);

    //
    for (int bit = NBITS-1; bit >= 0; bit--)
    {


        int bitVal = gpiod_line_get_value(pdspi->miso);
        pdspi->val |= ((int)bitVal << bit);

        gpiod_line_set_value(pdspi->clk, 1);   // Falling edge
        delay_us(1);
        gpiod_line_set_value(pdspi->clk, 0);   // Rising edge
        delay_us(1);                      // ~1 µs delay
    }

    // Convert and go to sleep
    gpiod_line_set_value(pdspi->cs, 1);
    gpiod_line_set_value(pdspi->clk, 0);

    return 0;
}


int pd_read(unsigned int channel)
{
    PDSPI_t pdspi = {0};
    char chip_name[32];
    unsigned int line_num;

    struct gpiod_line *cs0, *cs1, *clk, *miso;


    if (pd_init(channel, &pdspi) < 0)
    {
        pd_close(&pdspi);
        return -1;
    }

    pd_convert(&pdspi);
    //printf("PD value = %d\n", pdspi.val);

    pd_close(&pdspi);

    return pdspi.val;



    //printf("gpiod version: %s\n", gpiod_version_string());

    if (find_gpiochip_for_gpio_string(RPI_SPI3_SCLK, chip_name, sizeof(chip_name), &line_num) == 0) {
        printf("GPIO4 is on %s, line %u\n", chip_name, line_num);
    } else {
        printf("GPIO4 not found!\n");
        return -1;
    }

    // Open GPIO chip (use gpioinfo to check the correct /dev/gpiochipX)
    struct gpiod_chip *chip = gpiod_chip_open_by_name(chip_name);
    if (!chip) {
        perror("gpiod_chip_open_by_name");
        return 1;
    }

    // Make sure RF loopback is off
    if (gpio_set_value(RFPD_EN, 1) < 0)
        return -1;

    //
    cs0  = gpiod_chip_find_line(chip, RPI_SPI3_SS0);
    cs1  = gpiod_chip_find_line(chip, RPI_SPI3_SS1);
    clk  = gpiod_chip_find_line(chip, RPI_SPI3_SCLK);
    miso = gpiod_chip_find_line(chip, RPI_SPI3_MISO);

    //
    int res;
    res = gpiod_line_request_output(cs0, "spi3-cs0", 1);
    if (res < 0) printf("RPI_SPI3_SS0 failed.\n");
    res = gpiod_line_request_output(cs1, "spi3-cs1", 1);
    if (res < 0) printf("RPI_SPI3_SS1 failed.\n");
    res = gpiod_line_request_output(clk, "spi3-clk", 0);
    if (res < 0) printf("RPI_SPI3_SCLK failed.\n");
    res = gpiod_line_request_input(miso, "spi3-miso");
    if (res < 0) printf("RPI_SPI3_MISO failed.\n");





    gpiod_line_release(cs0);
    gpiod_line_release(cs1);
    gpiod_line_release(clk);
    gpiod_line_release(miso);

    gpiod_chip_close(chip);

    return res;
}


/*
int pd_convert(int pigpio, unsigned int channel)
{
    gpio_write(pigpio, channel, 0);
    gpio_write(pigpio, channel, 1);
    time_sleep(2e-6);           // ~2 µs delay
    gpio_write(pigpio, channel, 0);
}

int pd_read(unsigned int channel)
{
    int pigpio = pigpio_start(NULL, NULL);  // Connect to pigpiod
    if (pigpio < 0) {
        printf("Failed to connect to pigpio daemon\n");
        return -1;
    }

    // Make sure RF loopback is off
    if (gpio_set_value(RFPD_EN, 1) < 0)
        return -1;

    //
    set_mode(pigpio, RPI_SPI3_SS0,  PI_OUTPUT);
    set_mode(pigpio, RPI_SPI3_SS1,  PI_OUTPUT);
    set_mode(pigpio, RPI_SPI3_SCLK, PI_OUTPUT);
    set_mode(pigpio, RPI_SPI3_MISO, PI_INPUT);

    //
    gpio_write(pigpio, RPI_SPI3_SS0, 1);  // Idle CS high
    gpio_write(pigpio, RPI_SPI3_SS1, 1);  // Idle CS high
    gpio_write(pigpio, RPI_SPI3_SCLK, 0); // Idle CLK low (mode 0)

    // Dummy conversion
    pd_convert(pigpio, channel == 1 ? RPI_SPI3_SS0 : RPI_SPI3_SS1);

    // Real conversion
    pd_convert(pigpio, channel == 1 ? RPI_SPI3_SS0 : RPI_SPI3_SS1);

    // Read data

    //
    pigpio_stop(pigpio);

    return 0;
};
*/
