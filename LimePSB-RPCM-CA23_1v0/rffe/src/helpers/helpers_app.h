#ifndef HELPERS_APP_H
#define HELPERS_APP_H

#include <stdlib.h>

int gpio_set_value(const char *name, int value);
int limeSPI_read(const char *chip, unsigned int addr);
int limeSPI_write(const char *chip, unsigned int addr, unsigned int value);
int find_gpiochip_for_bcm_gpio(unsigned int bcm_gpio, char *chip_name, size_t chip_name_size, unsigned int *line_num);
int find_gpiochip_for_gpio_string(const char *gpio_str, char *chip_name, size_t chip_name_size, unsigned int *line_num);

#endif // HELPERS_APP_H
