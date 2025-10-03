#include "rffe_app.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
//#include <time.h>

#include "../helpers/helpers_app.h"
#include "../dac/dac_app.h"
#include "../adc/pac1720_app.h"

// Define required constatns to select appropriate RF switch input. For instance, if we want to select
// 2nd RF switch input, we have to send 0x0A via MIPI interface.
const unsigned int rf_sw_channel[] = {0x00, 0x02, 0x0A, 0x0E, 0x0B, 0x01, 0x09, 0x06, 0x04, 0x0C, 0x08};





/**
 * Sets RFFE band.
 *
 * @param channel - channel number (e.g. 1 for TRX1, 2 for TRX2)
 * @param band    - band number (e.g., 28, 8, 7, 3, 1, 38, 41, 42, 48, 78)
 * @return 0 on success, -1 on failure
 */
int rffe_set_band(unsigned int channel, unsigned int band)
{
    unsigned int trx, rx, trxant, fdd;

    //
    switch (band) {
    // FDD bands
    case 1:
        trx = 5; rx = 5; trxant = 5; fdd = 1;
    break;
    
    case 3:
        trx = 4; rx = 4; trxant = 4; fdd = 1;
    break;

    case 7:
        trx = 3; rx = 3; trxant = 3; fdd = 1;
    break;

    case 8:
        trx = 2; rx = 2; trxant = 2; fdd = 1;
    break;

    case 28:
        trx = 1; rx = 1; trxant = 1; fdd = 1;
    break;

    // TDD Bands
    case 48:
    case 78:
        trx = 10; rx = 5; trxant = 10; fdd = 0;
    break;

    case 42:
        trx = 8; rx = 5; trxant = 8; fdd = 0;
    break;

    case 41:
        trx = 7; rx = 5; trxant = 7; fdd = 0;
    break;

    case 38:
        trx = 6; rx = 5; trxant = 6; fdd = 0;
    break;

    default:
        fprintf(stderr, "Unexpected band.\n");
        return -1;
    }

    //
    if ( (channel < 1) || (channel > 2) ) {
        fprintf(stderr, "Unexpected channel.\n");
        return -1;
    }

    // Make sure RF loopback is off
    if (gpio_set_value("RF_SW_FBCAK", 0) < 0)
        return -1;

    // Read current register value
    int rval = limeSPI_read("FPGA", 0x000A);
    if (rval < 0)
        return -1;
    rval = rval & 0xFF1F;

    // Set TDD switch according to band
    if (fdd)
    {
        // Modify and write
        if (limeSPI_write("FPGA", 0x000A, (rval | 0x0020)) < 0) // Manual control
            return -1;
    }
    else
    {
        // TO DO
        if (limeSPI_write("FPGA", 0x000A, (rval | 0x0040)) < 0) // Automatic control, no invert
            return -1;
    }

    // Control RF switches via MIPI
    if (channel == 1)
    {
        if (limeSPI_write("FPGA", RX1_RF_SWITCH_ADDR, (rf_sw_channel[trx] << 8) | rf_sw_channel[rx]) < 0)
            return -1;
        if (limeSPI_write("FPGA", TRX1_ANT_RF_SWITCH_ADDR, rf_sw_channel[trxant]) < 0)
            return -1;
    }
    else
    {
        if (limeSPI_write("FPGA", RX2_RF_SWITCH_ADDR, (rf_sw_channel[trx] << 8) | rf_sw_channel[rx]) < 0)
            return -1;
        if (limeSPI_write("FPGA", TRX2_ANT_RF_SWITCH_ADDR, rf_sw_channel[trxant]) < 0)
            return -1;
    }

    return 0;
}

/**
 * Selects LMS7002M transmit channel. 
 *
 * @param channel - channel number (e.g. 1 for TX(1/2)_1, 2 for TX(1/2)_2)
 * @return 0 on success, -1 on failure
 */
int rffe_set_lms_tx(unsigned int channel)
{
    //
    switch (channel) {

    case 1:
        // Select TX(1/2)_1
        if (gpio_set_value("RF_SW_LMS_TX", 0) < 0)
            return -1;
    break;

    case 2:
        // Select TX(1/2)_2
        if (gpio_set_value("RF_SW_LMS_TX", 1) < 0)
            return -1;
    break;

    default:
        fprintf(stderr, "Unexpected LMS7002M transmit channel.\n");
        return -1;
    }

    return 0;
}

/**
 * Selects LMS7002M receive channel. 
 *
 * @param channel - channel number (e.g. 0 for RX(1/2)_L, 1 for RX(1/2)_W, 2 for RX(1/2)_H)
 * @return 0 on success, -1 on failure
 */
int rffe_set_lms_rx(unsigned int channel)
{
    //
    switch (channel) {

    case 0:
        // Select RX(1/2)_L
        if (gpio_set_value("RF_SW_LMS_RX2", 0) < 0)
            return -1;
        if (gpio_set_value("RF_SW_LMS_RX3", 1) < 0)
            return -1;
    break;

    case 1:
        // Select RX(1/2)_W
        if (gpio_set_value("RF_SW_LMS_RX2", 0) < 0)
            return -1;
        if (gpio_set_value("RF_SW_LMS_RX3", 0) < 0)
            return -1;
    break;

    case 2:
        // Select RX(1/2)_H
        if (gpio_set_value("RF_SW_LMS_RX2", 1) < 0)
            return -1;
        if (gpio_set_value("RF_SW_LMS_RX3", 0) < 0)
            return -1;
    break;

    default:
        fprintf(stderr, "Unexpected LMS7002M receive channel.\n");
        return -1;
    }

    // Turn on LNA power on

    return 0;
}

/**
 * Switch power amplifier of RF channel off.
 *
 * @param channel - channel number (e.g. 1 for TRX1, 2 for TRX2)
 * @return 0 on success, -1 on failure
 */
int rffe_pa_off(unsigned int channel)
{
    int ret = 0;
    struct iio_device *dac = NULL;

    struct iio_context *ctx = iio_create_default_context();
    if (!ctx) {
        fprintf(stderr, "Unable to create IIO context\n");
        return -1;
    }

    //struct iio_device *dac0 = find_ad5662_by_cs(ctx, 0); // CS0 (GPIO3)
    //struct iio_device *dac1 = find_ad5662_by_cs(ctx, 1); // CS1 (GPIO2)

    do
    {
    
        //
        if (channel == 1)
        {
            dac = find_ad5662_by_cs(ctx, 0); // CS0 (GPIO3)
            //dac = find_ad5662_by_cs(ctx, 1); // CS1 (GPIO2)
            if (dac == NULL) { ret = -1; break; }
            // 1. Turn OFF RF signal
            
            // TO TO: Turn OFF LMS TX1/2.

            // Disable VCC5P0_RF1
            if (gpio_set_value("EN_VCC5P0_RF1", 0) < 0) { ret = -1; break; }

            // Disable VCC8P0_RF1
            if (gpio_set_value("EN_VCC8P0_RF1", 0) < 0) { ret = -1; break; }
            
            // 2. Decrease VG1 to -2 V. Value of 63000 corresponds to ~ -2V of VG1
            if (ad5662_write_raw(dac, 63000) < 0) { ret = -1; break; }

            // 3. Turn OFF VDD
            if (gpio_set_value("EN_TX1_PA_VDD", 0) < 0) { ret = -1; break; }

            // 4. Turn OFF VG2
            if (gpio_set_value("EN_TX1_PA_VG2", 0) < 0) { ret = -1; break; }
            
            // 5. Turn OFF VG1 (set DAC value to 0 to get 0V)
            if (ad5662_write_raw(dac, 0) < 0) { ret = -1; break; }

            ret = 0;
        }
        else if (channel == 2)
        {
            //dac = find_ad5662_by_cs(ctx, 0); // CS0 (GPIO3)
            dac = find_ad5662_by_cs(ctx, 1); // CS1 (GPIO2)
            if (dac == NULL) { ret = -1; break; }
            // 1. Turn OFF RF signal
            
            // TO TO: Turn OFF LMS TX1/2.

            // Disable VCC5P0_RF1
            if (gpio_set_value("EN_VCC5P0_RF2", 0) < 0) { ret = -1; break; }

            // Disable VCC8P0_RF1
            if (gpio_set_value("EN_VCC8P0_RF2", 0) < 0) { ret = -1; break; }
            
            // 2. Decrease VG1 to -2 V. Value of 63000 corresponds to ~ -2V of VG1
            if (ad5662_write_raw(dac, 63000) < 0) { ret = -1; break; }

            // 3. Turn OFF VDD
            if (gpio_set_value("EN_TX2_PA_VDD", 0) < 0) { ret = -1; break; }

            // 4. Turn OFF VG2
            if (gpio_set_value("EN_TX2_PA_VG2", 0) < 0) { ret = -1; break; }
            
            // 5. Turn OFF VG1 (set DAC value to 0 to get 0V)
            if (ad5662_write_raw(dac, 0) < 0) { ret = -1; break; }

            ret = 0;
        }
        else
            ret = -1;


    } while (0);

    iio_context_destroy(ctx);
    return ret;
}

/**
 * Switch power amplifier of RF channel off.
 *
 * @param channel - channel number (e.g. 1 for TRX1, 2 for TRX2)
 * @return 0 on success, -1 on failure
 */
int rffe_pa_on(unsigned int channel)
{
    int ret = 0;
    struct iio_device *dac = NULL;
    pac1720_t adc;

    struct iio_context *ctx = iio_create_default_context();
    if (!ctx) {
        fprintf(stderr, "Unable to create IIO context\n");
        return -1;
    }

    if (pac1720_init(&adc) < 0)
    {
        fprintf(stderr, "Unable to communicate with PAC1270\n");
        return -1;
    }

    //struct iio_device *dac0 = find_ad5662_by_cs(ctx, 0); // CS0 (GPIO3)
    //struct iio_device *dac1 = find_ad5662_by_cs(ctx, 1); // CS1 (GPIO2)

    do
    {
    
        //
        if (channel == 1)
        {
            dac = find_ad5662_by_cs(ctx, 0); // CS0 (GPIO3)
            //dac = find_ad5662_by_cs(ctx, 1); // CS1 (GPIO2)
            if (dac == NULL) { ret = -1; break; }
            // 1. Turn OFF RF signal
            
            // TO TO: Turn OFF LMS TX1/2.

            // Disable VCC5P0_RF1
            if (gpio_set_value("EN_VCC5P0_RF1", 0) < 0) { ret = -1; break; }

            // Disable VCC8P0_RF1
            if (gpio_set_value("EN_VCC8P0_RF1", 0) < 0) { ret = -1; break; }
            
            //usleep(50);

            // Set TX1/2 PA VG1 DAC value (2V at DAC output) to get -2V at TX1/2_PA_VG1 rail.
            // Value of 63000 corresponds to ~ -2V of VG1
            if (ad5662_write_raw(dac, 63999) < 0) { ret = -1; break; }

            // 2. Set VG2 = +5 V and Turn ON.
            if (gpio_set_value("EN_TX1_PA_VG2", 1) < 0) { ret = -1; break; }

            // 3. Set VDD = +12 V and Turn ON.
            if (gpio_set_value("EN_TX1_PA_VDD", 1) < 0) { ret = -1; break; }

            //sleep(50);
            // 4. Increase VG1 (min:-2.0V; typ:-0.8V; max:-0.2V) to desired IDD (typ. 400mA).
            if (ad5662_write_raw(dac, 29000) < 0) { ret = -1; break; }
            //if (ad5662_write_raw(dac, 33000) < 0) { ret = -1; break; }
            //pac1720_read(&adc);

            // 5. Turn ON RF signal.
            if (gpio_set_value("EN_VCC5P0_RF1", 1) < 0) { ret = -1; break; }
            if (gpio_set_value("EN_VCC8P0_RF1", 1) < 0) { ret = -1; break; }

            ret = 0;
        }
        else if (channel == 2)
        {
            //dac = find_ad5662_by_cs(ctx, 0); // CS0 (GPIO3)
            dac = find_ad5662_by_cs(ctx, 1); // CS1 (GPIO2)
            if (dac == NULL) { ret = -1; break; }
            // 1. Turn OFF RF signal
            
            // TO TO: Turn OFF LMS TX1/2.

            // Disable VCC5P0_RF2
            if (gpio_set_value("EN_VCC5P0_RF2", 0) < 0) { ret = -1; break; }

            // Disable VCC8P0_RF2
            if (gpio_set_value("EN_VCC8P0_RF2", 0) < 0) { ret = -1; break; }
            
            //usleep(50);

            // Set TX1/2 PA VG1 DAC value (2V at DAC output) to get -2V at TX1/2_PA_VG1 rail.
            // Value of 63000 corresponds to ~ -2V of VG1
            if (ad5662_write_raw(dac, 63999) < 0) { ret = -1; break; }

            // 2. Set VG2 = +5 V and Turn ON.
            if (gpio_set_value("EN_TX2_PA_VG2", 1) < 0) { ret = -1; break; }

            // 3. Set VDD = +12 V and Turn ON.
            if (gpio_set_value("EN_TX2_PA_VDD", 1) < 0) { ret = -1; break; }

            //sleep(50);
            // 4. Increase VG1 (min:-2.0V; typ:-0.8V; max:-0.2V) to desired IDD (typ. 400mA).
            if (ad5662_write_raw(dac, 29000) < 0) { ret = -1; break; }
            //if (ad5662_write_raw(dac, 33000) < 0) { ret = -1; break; }
            //pac1720_read(&adc);

            // 5. Turn ON RF signal.
            if (gpio_set_value("EN_VCC5P0_RF2", 1) < 0) { ret = -1; break; }
            if (gpio_set_value("EN_VCC8P0_RF2", 1) < 0) { ret = -1; break; }

            ret = 0;
        }
        else
            ret = -1;


    } while (0);

    pac1720_close(&adc);
    iio_context_destroy(ctx);
    return ret;
}

/**
 * Read current DAC AD5662 value
 *
 * @param channel - channel number (e.g. 1 for TRX1, 2 for TRX2)
 * @param value - Pointer to an unsigned integer where the read value will be stored.
 * @return 0 on success, -1 on failure
 */
int rffe_rd_dac(unsigned int channel, unsigned int *value)
{
    int ret = 0;
    struct iio_device *dac = NULL;

    struct iio_context *ctx = iio_create_default_context();
    if (!ctx) {
        fprintf(stderr, "Unable to create IIO context\n");
        return -1;
    }

    //struct iio_device *dac0 = find_ad5662_by_cs(ctx, 0); // CS0 (GPIO3)
    //struct iio_device *dac1 = find_ad5662_by_cs(ctx, 1); // CS1 (GPIO2)

    do
    {
        //
        if (channel == 1)
        {
            dac = find_ad5662_by_cs(ctx, 0); // CS0 (GPIO3)
            if (dac == NULL) { ret = -1; break; }
            ret = 0;
        }
        else if (channel == 2)
        {
            dac = find_ad5662_by_cs(ctx, 1); // CS1 (GPIO2)
            if (dac == NULL) { ret = -1; break; }
            ret = 0;
        }
        else
            ret = -1;

        if (ad5662_read_raw(dac, value) < 0) { ret = -1; break; }

    } while (0);

    iio_context_destroy(ctx);
    return ret;
}

/**
 * Read current DAC AD5662 value
 *
 * @param channel - channel number (e.g. 1 for TRX1, 2 for TRX2)
 * @param value - value to be written.
 * @return 0 on success, -1 on failure
 */
int rffe_wr_dac(unsigned int channel, unsigned int value)
{
    int ret = 0;
    struct iio_device *dac = NULL;

    //
    struct iio_context *ctx = iio_create_default_context();
    if (!ctx) {
        fprintf(stderr, "Unable to create IIO context\n");
        return -1;
    }

    //
    do
    {
        //
        if (channel == 1)
        {
            dac = find_ad5662_by_cs(ctx, 0); // CS0 (GPIO3)
            if (dac == NULL) { ret = -1; break; }
            ret = 0;
        }
        else if (channel == 2)
        {
            dac = find_ad5662_by_cs(ctx, 1); // CS1 (GPIO2)
            if (dac == NULL) { ret = -1; break; }
            ret = 0;
        }
        else
            ret = -1;

        if (ad5662_write_raw(dac, value) < 0) { ret = -1; break; }

    } while (0);

    iio_context_destroy(ctx);
    return ret;
}

/**
 * Switch LNA of RF channel off.
 *
 * @param channel - channel number (e.g. 1 for TRX1, 2 for TRX2)
 * @return 0 on success, -1 on failure
 */
int rffe_lna_off(unsigned int channel)
{
    switch (channel) {

    case 1:
        if (gpio_set_value("EN_VCC3P3_RF1", 0) < 0) return -1;
        if (gpio_set_value("EN_VCC5P0_RF1", 0) < 0) return -1;
    break;

    case 2:
        if (gpio_set_value("EN_VCC3P3_RF2", 0) < 0) return -1;
        if (gpio_set_value("EN_VCC5P0_RF2", 0) < 0) return -1;
    break;

    default:
        return -1;
    }

    return 0;
}

/**
 * Switch LNA of RF channel on.
 *
 * @param channel - channel number (e.g. 1 for TRX1, 2 for TRX2)
 * @return 0 on success, -1 on failure
 */
int rffe_lna_on(unsigned int channel)
{
    switch (channel) {

    case 1:
        if (gpio_set_value("EN_VCC3P3_RF1", 1) < 0) return -1;
        if (gpio_set_value("EN_VCC5P0_RF1", 1) < 0) return -1;
    break;

    case 2:
        if (gpio_set_value("EN_VCC3P3_RF2", 1) < 0) return -1;
        if (gpio_set_value("EN_VCC5P0_RF2", 1) < 0) return -1;
    break;

    default:
        return -1;
    }

    return 0;
}

/**
 * Switch LNA of RF channel on.
 *
 * @param channel - channel number (e.g. 1 for RX1, 2 for RX2)
 * @param att     - attenuation (e.g. 0 to 15)
 * @return 0 on success, -1 on failure
 */
int rffe_rxatt(unsigned int channel, unsigned int att)
{
    att = ~att & 0xF;
    switch (channel) {

    case 1:
        if (gpio_set_value("RX1_ADD_V1", (att & 0x8) >> 3) < 0) return -1;
        if (gpio_set_value("RX1_ADD_V2", (att & 0x4) >> 2) < 0) return -1;
        if (gpio_set_value("RX1_ADD_V3", (att & 0x2) >> 1) < 0) return -1;
        if (gpio_set_value("RX1_ADD_V4", (att & 0x1) >> 0) < 0) return -1;
    break;

    case 2:
        if (gpio_set_value("RX2_ADD_V1", (att & 0x8) >> 3) < 0) return -1;
        if (gpio_set_value("RX2_ADD_V2", (att & 0x4) >> 2) < 0) return -1;
        if (gpio_set_value("RX2_ADD_V3", (att & 0x2) >> 1) < 0) return -1;
        if (gpio_set_value("RX2_ADD_V4", (att & 0x1) >> 0) < 0) return -1;
    break;

    default:
        return -1;
    }

    return 0;
}

/**
 * Activate/deactivate onboard RF loopback.
 *
 * @param onoff - 1: activate RF loopback; 0: dectivate RF loopback.
 * @return 0 on success, -1 on failure
 */
int rffe_rflb(unsigned int onoff)
{
    //if (onoff < 0 || onoff > 1) return -1;
    if (onoff > 1) return -1;

    // Make sure RF loopback is off
    if (gpio_set_value("RF_SW_FBCAK", onoff) < 0) return -1;

    return 0;
}

/**
 * Set TDD signal to high/low
 *
 * @param lh - 0: ste TDD signal low; 1: set TDD signal high.
 * @return 0 on success, -1 on failure
 */
int rffe_tdd(unsigned int lh)
{
    //if (lh < 0 || lh > 1) return -1;
    if (lh > 1) return -1;

    // Read current register value
    int rval = limeSPI_read("FPGA", 0x000A);
    if (rval < 0)
        return -1;
    rval = rval & 0xFF1F;

    // Set TDD signal
    //if (limeSPI_write("FPGA", 0x000A, lh == 1 ? (rval | 0x0020) : (rval & (~0x0020))) < 0)
    if (limeSPI_write("FPGA", 0x000A, lh == 1 ? (rval | 0x0020) : rval) < 0)
        return -1;

    return 0;
}