#ifndef PAC1720_APP_H
#define PAC1720_APP_H

// I2C device
#define PAC1720_I2C_DEV  "/dev/i2c-10"
#define PAC1720_I2C_ADDR 0x4C

// PAC1720 register addresses
#define REG_CONFIG              0x00
#define REG_ONESHOT             0x02
#define REG_CH1_VSENSE_SAMPLING 0x0B
#define REG_CH2_VSENSE_SAMPLING 0x0C
#define REG_CH1_VSENSE_MSB      0x0D
#define REG_CH2_VSENSE_MSB      0x0F
#define REG_CH1_VSOURCE_MSB     0x11
#define REG_CH2_VSOURCE_MSB     0x13

// Shunt resistor value (Ohms)
#define RSHUNT1 0.01
#define RSHUNT2 0.01

typedef struct pac1720_t {
    int fd;
    int initialized;

    double vsource1;
    double vsense1;
    double power1;
    double i1;
    double vsource2;
    double vsense2;
    double power2;
    double i2;
} pac1720_t;

int pac1720_init(pac1720_t *pac1720);
void pac1720_close(pac1720_t *pac1720);
int pac1720_read(pac1720_t *pac1720);
void pac1720_measure();

#endif // PAC1720_APP_H
