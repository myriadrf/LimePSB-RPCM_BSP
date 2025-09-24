#include "pac1720_app.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

// Helper: write one byte
int i2c_write_reg(int fd, uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    if (write(fd, buf, 2) != 2) {
        perror("i2c write failed");
        return -1;
    }
    return 0;
}

// Helper: read N bytes from register
int i2c_read_regs(int fd, uint8_t reg, uint8_t *buf, size_t len) {
    if (write(fd, &reg, 1) != 1) {
        perror("i2c write reg failed");
        return -1;
    }
    if (read(fd, buf, len) != (ssize_t)len) {
        perror("i2c read failed");
        return -1;
    }
    return 0;
}

// Decode 11-bit left-justified sense voltage
double decode_vsense(uint8_t msb, uint8_t lsb) {
    int16_t raw = ((msb << 8) | lsb) >> 5; // top 11 bits
    // ±80 mV full scale => LSB = 80mV / 2^10 = 78.125 µV
    return raw * 0.000078125; // volts
}

// Decode 11-bit left-justified bus voltage
double decode_vsource(uint8_t msb, uint8_t lsb) {
    int raw = ((msb << 8) | lsb) >> 5;
    // 0–40 V full scale => LSB = 40V / 2048 = 19.53 mV
    return raw * 0.01953; // volts
}

//
int pac1720_init(pac1720_t *pac1720)
{
    memset(pac1720, 0, sizeof(pac1720_t));

    //
    pac1720->fd = open(PAC1720_I2C_DEV, O_RDWR);
    if (pac1720->fd < 0) { perror("open"); return -1; }

    do {
        // That ioctl() binds the opened /dev/i2c-X file descriptor to the PAC1720 at address 0xXX, 
        // so subsequent read()/write() go to the correct chip
        if (ioctl( pac1720->fd, I2C_SLAVE, PAC1720_I2C_ADDR) < 0) { perror("ioctl"); break; }

        // Configure: averaging, sample times, one-shot mode
        // This depends on datasheet bitfields; here an example config:
        uint8_t value = 0x7B; // AVG=8, Vsrc=20ms, Vsense=80ms, one-shot
        if (i2c_write_reg(pac1720->fd, REG_CONFIG, value) < 0) { perror("i2c_write_reg"); break; }
        value = 0x53; // AVG=8, Vsrc=20ms, Vsense=80ms, one-shot
        if (i2c_write_reg(pac1720->fd, REG_CH1_VSENSE_SAMPLING, value) < 0) { perror("i2c_write_reg"); break; }
        if (i2c_write_reg(pac1720->fd, REG_CH2_VSENSE_SAMPLING, value) < 0) { perror("i2c_write_reg"); break; }

        pac1720->initialized = 1;
        return 0;
    } while (0);

    close(pac1720->fd);
    return -1;
}

//
void pac1720_close(pac1720_t *pac1720)
{
    if (pac1720->fd > 0) close(pac1720->fd);
    memset(pac1720, 0, sizeof(pac1720_t));
}

//
int pac1720_read(pac1720_t *pac1720)
{
    uint8_t buf[2];

    if (pac1720->fd <= 0) return -1;

    do {
        // Trigger one-shot conversion
        i2c_write_reg(pac1720->fd, REG_ONESHOT, 0x01);

        // Allow time for conversion (max 100ms+ depending on averaging)
        usleep(200000);

        // Read registers
        if (i2c_read_regs(pac1720->fd, REG_CH1_VSOURCE_MSB, buf, 2) < 0) { perror("i2c_write_reg"); break; }
        pac1720->vsource1 = decode_vsource(buf[0], buf[1]);

        if (i2c_read_regs(pac1720->fd, REG_CH2_VSOURCE_MSB, buf, 2) < 0) { perror("i2c_write_reg"); break; }
        pac1720->vsource2 = decode_vsource(buf[0], buf[1]);

        if (i2c_read_regs(pac1720->fd, REG_CH1_VSENSE_MSB, buf, 2) < 0) { perror("i2c_write_reg"); break; }
        pac1720->vsense1 = decode_vsense(buf[0], buf[1]);

        if (i2c_read_regs(pac1720->fd, REG_CH2_VSENSE_MSB, buf, 2) < 0) { perror("i2c_write_reg"); break; }
        pac1720->vsense2 = decode_vsense(buf[0], buf[1]);

        // Convert to current in mA
        pac1720->i1 = 1000.0 * pac1720->vsense1 / RSHUNT1;
        pac1720->i2 = 1000.0 * pac1720->vsense2 / RSHUNT2;

        // Compute power
        pac1720->power1 = pac1720->vsource1 * pac1720->i1;
        pac1720->power2 = pac1720->vsource2 * pac1720->i2;

        return 0;
    } while (0);

    return -1;
}

void pac1720_measure()
{
    pac1720_t pac1720;

    if (pac1720_init(&pac1720) < 0) return;
    if (pac1720_read(&pac1720) < 0) return;

    // Print results
    printf("Version 1\r\n");
    printf("Channel 1:\n");
    printf("  Vsource = %.3f V\n", pac1720.vsource1);
    printf("  Vsense  = %.6f V -> I = %.2f mA\n", pac1720.vsense1, pac1720.i1);
    printf("  Power   = %.2f mW\n", pac1720.power1);

    printf("Channel 2:\n");
    printf("  Vsource = %.3f V\n", pac1720.vsource2);
    printf("  Vsense  = %.6f V -> I = %.2f mA\n", pac1720.vsense2, pac1720.i2);
    printf("  Power   = %.2f mW\n", pac1720.power2);

    pac1720_close(&pac1720);
}