#include "helpers_app.h"

#include <stdio.h>

#include <string.h>
#include <errno.h>

//sudo apt install libgpiod-dev gpiod
#include <gpiod.h>

/**
 * @brief Find which gpiochip and line a BCM GPIO belongs to.
 *
 * @param bcm_gpio  BCM GPIO number (e.g. 23)
 * @param chip_name Output buffer for chip name (e.g. "gpiochip4")
 * @param line_num  Output for line number within that chip
 * @return 0 on success, -1 if not found
 */
int find_gpiochip_for_bcm_gpio(unsigned int bcm_gpio,
                               char *chip_name, size_t chip_name_size,
                               unsigned int *line_num)
{
    int chip_index = 0;
    struct gpiod_chip *chip;

    // Iterate through all gpiochips until none left
    while (1) {
        char name[32];
        snprintf(name, sizeof(name), "gpiochip%d", chip_index);

        chip = gpiod_chip_open_by_name(name);
        if (!chip) break; // No more chips

        int num_lines = gpiod_chip_num_lines(chip);

        for (int i = 0; i < num_lines; i++) {
            struct gpiod_line *line = gpiod_chip_get_line(chip, i);
            const char *lname = gpiod_line_name(line);

            if (lname && strstr(lname, "GPIO")) {
                unsigned int gpio_num = atoi(lname + 4); // skip "GPIO"
                if (gpio_num == bcm_gpio) {
                    strncpy(chip_name, name, chip_name_size);
                    *line_num = i;
                    gpiod_chip_close(chip);
                    return 0;
                }
            }
        }

        gpiod_chip_close(chip);
        chip_index++;
    }

    return -1; // Not found
}

/**
 * @brief Find which gpiochip and line a GPIO belongs to.
 *
 * @param gpio_str     String representing GPIO, e.g., "GPIO4"
 * @param chip_name    Output buffer for chip name (e.g., "gpiochip0")
 * @param chip_name_size Size of the chip_name buffer
 * @param line_num     Output for line number within that chip
 * @return int         0 on success, -1 if not found
 */
int find_gpiochip_for_gpio_string(const char *gpio_str,
                                  char *chip_name, size_t chip_name_size,
                                  unsigned int *line_num)
{
    if (!gpio_str || strncasecmp(gpio_str, "GPIO", 4) != 0) {
        return -1; // invalid format
    }

    unsigned int gpio_num = atoi(gpio_str + 4); // parse number after "GPIO"
    if (gpio_num == 0 && gpio_str[4] != '0') {
        return -1; // invalid number
    }

    int chip_index = 0;
    struct gpiod_chip *chip;

    while (1) {
        char name[32];
        snprintf(name, sizeof(name), "gpiochip%d", chip_index);

        chip = gpiod_chip_open_by_name(name);
        if (!chip) break; // no more chips

        int num_lines = gpiod_chip_num_lines(chip);

        for (int i = 0; i < num_lines; i++) {
            struct gpiod_line *line = gpiod_chip_get_line(chip, i);
            const char *lname = gpiod_line_name(line);

            if (lname && strncasecmp(lname, "GPIO", 4) == 0) {
                unsigned int num = atoi(lname + 4);
                if (num == gpio_num) {
                    strncpy(chip_name, name, chip_name_size);
                    *line_num = i;
                    gpiod_chip_close(chip);
                    return 0;
                }
            }
        }

        gpiod_chip_close(chip);
        chip_index++;
    }

    return -1; // Not found
}


/**
 * Set a GPIO line (by name) to high or low using gpiofind + gpioset.
 *
 * @param name  - GPIO name (as used in gpiofind)
 * @param value - 0 = low, 1 = high
 * @return 0 on success, -1 on failure
 */
int gpio_set_value(const char *name, int value) {
    char cmd[128];
    char buf[128];
    FILE *fp;
    char chip[64];
    int line;
    int ret;

    if (value != 0 && value != 1) {
        fprintf(stderr, "Error: invalid GPIO value %d (must be 0 or 1)\n", value);
        return -1;
    }

    // Step 1: run gpiofind to resolve name → chip + line
    snprintf(cmd, sizeof(cmd), "gpiofind %s 2>/dev/null", name);

    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: failed to run command '%s': %s\n", cmd, strerror(errno));
        return -1;
    }

    if (fgets(buf, sizeof(buf), fp) == NULL) {
        fprintf(stderr, "Error: gpiofind did not return output for '%s'\n", name);
        pclose(fp);
        return -1;
    }
    pclose(fp);

    // Strip newline
    buf[strcspn(buf, "\r\n")] = 0;

    // Parse output: "gpiochip16 7"
    if (sscanf(buf, "%63s %d", chip, &line) != 2) {
        fprintf(stderr, "Error: could not parse gpiofind output '%s'\n", buf);
        return -1;
    }

    // Step 2: run gpioset to set the pin
    snprintf(cmd, sizeof(cmd), "gpioset %s %d=%d", chip, line, value);
    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Error: gpioset failed with exit code %d\n", WEXITSTATUS(ret));
        return -1;
    }

    return 0;
}

/**
 * Reads a register value from limeSPI CLI tool.
 *
 * @param chip    - chip name (e.g., "FPGA")
 * @param addr    - register address (hex, e.g., 0x00d7)
 * @return register value as int, or -1 on error
 */
int limeSPI_read(const char *chip, unsigned int addr) {
    char cmd[128];
    char buf[64];
    FILE *fp;
    int value = -1;

    // Build command string (address formatted as 4 hex digits)
    snprintf(cmd, sizeof(cmd),
             "limeSPI read --chip %s --stream %04x 2>/dev/null",
             chip, addr);

    // Run the command and open a pipe for reading output
    fp = popen(cmd, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: failed to run command '%s': %s\n", cmd, strerror(errno));
        return -1;
    }

    // Read first line of output
    if (fgets(buf, sizeof(buf), fp) != NULL) {
        // Strip newline if present
        buf[strcspn(buf, "\r\n")] = 0;

        // Parse hex value
        if (sscanf(buf, "%x", &value) != 1) {
            fprintf(stderr, "Error: could not parse output '%s'\n", buf);
            value = -1;
        }
    } else {
        fprintf(stderr, "Error: no output from command '%s'\n", cmd);
    }

    pclose(fp);
    return value;
}

/**
 * Writes a value to a register via limeSPI CLI tool.
 *
 * @param chip    - chip name (e.g., "FPGA")
 * @param addr    - register address (hex, e.g., 0x00d7)
 * @param value   - value to write (hex, e.g., 0x0004)
 * @return 0 on success, -1 on failure
 */
int limeSPI_write(const char *chip, unsigned int addr, unsigned int value) {
    char cmd[128];
    int ret;

    // Build command string (ADDR + DATA as one 8-hex-digit word)
    snprintf(cmd, sizeof(cmd),
             "limeSPI write --chip %s --stream %04x%04x 2>/dev/null",
             chip, addr & 0xFFFF, value & 0xFFFF);

    ret = system(cmd);
    if (ret != 0) {
        fprintf(stderr, "Error: command failed '%s' (exit code %d)\n",
                cmd, WEXITSTATUS(ret));
        return -1;
    }

    return 0;
}