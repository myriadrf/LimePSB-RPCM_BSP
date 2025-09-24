#ifndef DAC_APP_H
#define DAC_APP_H

#include <iio.h>

#define DAC_DEVICE_NAME "ad5662"

#define DAC_MIN_VALUE 0
#define DAC_MAX_VALUE 65535

//int32_t pps_enable(char *device, char pps_en);

int32_t test_dac();
int32_t find_dac(unsigned int dac);
int32_t write_dac_value(int32_t value, uint8_t trace);

void print_device_info(const struct iio_device *dev);

void list_iio_devices(void);
struct iio_device *find_ad5662_by_cs(struct iio_context *ctx, int cs_target);
int ad5662_write_raw(struct iio_device *dev, unsigned int value);
int ad5662_read_raw(struct iio_device *dev, unsigned int *value);

#endif // DAC_APP_H
