#include <stdio.h>
#include <iio.h>
#include <string.h>
//#include <unistd.h>

#include "dac_app.h"

#include <limits.h>
//#include <linux/limits.h>


//
int32_t write_dac_value(int32_t value, uint8_t trace)
{
	struct iio_context *ctx = NULL;
	struct iio_device *dev = NULL;
	unsigned int major, minor;
	char git_tag[8];
	int ret;
	
	// Get LIBIIO library version
	iio_library_get_version(&major, &minor, git_tag);
	//printf(" Library version: %u.%u (git tag: %s)\n", major, minor, git_tag);

	//
	//printf(" Acquiring IIO context\n");
	if (!((ctx = iio_create_local_context()) && "No context"))
		printf(" CTX Assertion Failed\n");
		
	// Get available device count from context
    if (!(iio_context_get_devices_count(ctx) > 0 && "No devices"))
        printf(" No devices in CTX\n");
    
    //    
	if (trace) printf(" Acquiring device: %s\n", DAC_DEVICE_NAME);
	dev = iio_context_find_device(ctx, DAC_DEVICE_NAME);
	if (!dev)
	{
		printf(" No device %s found during acquire\n", DAC_DEVICE_NAME);
	}
	
	//
	char charbuf[256];
    memset(charbuf, '\0', 256);
    ret = iio_device_attr_read(dev, "out_voltage0_raw\0", charbuf, sizeof(charbuf));
    if(ret < 0)
    {
        printf(" Device attribute read: %d: %s\n", (int)ret, strerror(-ret));
    }
	if (trace) printf(" Old DAC Value: %s\n", charbuf);
	
	//
	memset(charbuf, '\0', 256);
	sprintf(charbuf, "%d", value);
	ret = iio_device_attr_write(dev, "out_voltage0_raw\0", charbuf);
	if(ret < 0)
	{
		printf("Error in_accel_x_calibbias set: %d: %s\n", (int)ret, strerror(-ret));
	}
		
	//
	memset(charbuf, '\0', 256);
    ret = iio_device_attr_read(dev, "out_voltage0_raw\0", charbuf, sizeof(charbuf));
    if(ret < 0)
    {
        printf(" Device attribute read: %d: %s\n", (int)ret, strerror(-ret));
	}
	if (trace) printf(" New DAC Value: %s\n", charbuf);
 
	//printf(" Destroying context\n");
	if (ctx)
	{
		iio_context_destroy(ctx);
	}
        
	return 0;
}

//
int32_t test_dac()
{
	struct iio_context *ctx = NULL;
	struct iio_device *dev = NULL;
	int ret = 0;
	unsigned int major, minor;
	char git_tag[8];

	// Get LIBIIO library version
	iio_library_get_version(&major, &minor, git_tag);
	printf(" Library version: %u.%u (git tag: %s)\n", major, minor, git_tag);
	
	while (1)
	{
		//
		//printf(" Acquiring IIO context\n");
		if (!((ctx = iio_create_local_context()) && "No context"))
		{
			//printf(" CTX Assertion Failed\n");
			return -1;
		}
			
		// Get available device count from context
		if (!(iio_context_get_devices_count(ctx) > 0 && "No devices"))
		{
			//printf(" No devices in CTX\n");
			ret = -1;
			break;
		}
		
		//    
		//printf(" Acquiring device: %s\n", DAC_DEVICE_NAME);
		dev = iio_context_find_device(ctx, DAC_DEVICE_NAME);
		if (!dev)
		{
			//printf(" No device found\n");
			ret = -1;
			break;
		}
		
		//
		char charbuf[256];
		memset(charbuf, '\0', 256);
		ret = iio_device_attr_read(dev, "out_voltage0_raw\0", charbuf, sizeof(charbuf));
		if(ret < 0)
		{
			printf(" Device attribute read: %d: %s\n", (int)ret, strerror(-ret));
			ret = -1;
			break;
		}
		printf(" Old DAC Value: %s\n", charbuf);
		break;
	}
 
	printf(" Destroying context\n");
	if (ctx)
	{
		iio_context_destroy(ctx);
	}
        
	return ret;
}


//
int32_t find_dac(unsigned int dac)
{
	struct iio_context *ctx = NULL;
	struct iio_device *dev = NULL;
	int ret = 0;
	unsigned int major, minor;
	char git_tag[8];

	// Get LIBIIO library version
	iio_library_get_version(&major, &minor, git_tag);
	printf(" Library version: %u.%u (git tag: %s)\n", major, minor, git_tag);
	
	while (1)
	{
		//
		//printf(" Acquiring IIO context\n");
		if (!((ctx = iio_create_local_context()) && "No context"))
		{
			//printf(" CTX Assertion Failed\n");
			return -1;
		}
			
		// Get available device count from context
		if (!(iio_context_get_devices_count(ctx) > 0 && "No devices"))
		{
			//printf(" No devices in CTX\n");
			ret = -1;
			break;
		}
		
		//    
		//printf(" Acquiring device: %s\n", DAC_DEVICE_NAME);
		dev = iio_context_find_device(ctx, DAC_DEVICE_NAME);
		if (!dev)
		{
			//printf(" No device found\n");
			ret = -1;
			break;
		}
		
		//
		char charbuf[256];
		memset(charbuf, '\0', 256);
		//ret = iio_device_attr_read(dev, "out_voltage0_raw\0", charbuf, sizeof(charbuf));
		ret = iio_device_attr_read(dev, "of_node/reg\0", charbuf, sizeof(charbuf));
		if(ret < 0)
		{
			printf(" Device attribute read: %d: %s\n", (int)ret, strerror(-ret));
			ret = -1;
			break;
		}
		printf(" Old DAC Value: %s\n", charbuf);
		//break;
	}
 
	printf(" Destroying context\n");
	if (ctx)
	{
		iio_context_destroy(ctx);
	}
        
	return ret;
}

////////////////////////

// Map chip-select (reg) value to GPIO number
// (based on your overlay: CS0 -> GPIO3, CS1 -> GPIO2)
const char* cs_to_gpio(const char *reg_value) {
    if (!reg_value) return "unknown";
    if (strcmp(reg_value, "0") == 0) return "GPIO3";
    if (strcmp(reg_value, "1") == 0) return "GPIO2";
    return "unknown";
}

// Print info for one device
void print_device_info(const struct iio_device *dev) {
    const char *name = iio_device_get_name(dev);
    if (!name) name = "unknown";

    printf("Device: %s\n", name);

    // Try to read the "of_node.reg" attribute
    char reg_buf[64];
    ssize_t ret = iio_device_attr_read(dev, "of_node.reg", reg_buf, sizeof(reg_buf) - 1);
    if (ret > 0) {
        reg_buf[ret] = '\0';
        printf("  reg = %s\n", reg_buf);
        printf("  attached to %s\n", cs_to_gpio(reg_buf));
    } else {
        printf("  reg = (not available)\n");
    }

    // Print compatible too (helps distinguish devices)
    char comp_buf[64];
    ret = iio_device_attr_read(dev, "of_node.compatible", comp_buf, sizeof(comp_buf) - 1);
    if (ret > 0) {
        comp_buf[ret] = '\0';
        printf("  compatible = %s\n", comp_buf);
    }
}


//////////////////////////////

// Fixed mapping
static int cs_to_gpio_map[] = {
    3,  // CS0 → GPIO3
    2,  // CS1 → GPIO2
    -1, // CS2 unused
    -1, // CS3 unused
};

// Get CS index from of_node symlink
int get_cs_from_of_node(const char *iio_id) {
    char linkpath[PATH_MAX];
    char target[PATH_MAX];
    snprintf(linkpath, sizeof(linkpath), "/sys/bus/iio/devices/%s/of_node", iio_id);

    ssize_t n = readlink(linkpath, target, sizeof(target) - 1);
    if (n < 0) return -1;
    target[n] = '\0';

    // Last path component, e.g. "ad5662@0"
    const char *node = strrchr(target, '/');
    node = node ? node + 1 : target;

    const char *at = strrchr(node, '@');
    if (!at || !*(at + 1)) return -1;

    return atoi(at + 1);
}


// Helper function: list all IIO devices with CS/GPIO mapping
void list_iio_devices(void) {
    struct iio_context *ctx = iio_create_default_context();
    if (!ctx) {
        fprintf(stderr, "Unable to create IIO context\n");
        return;
    }

    unsigned int ndev = iio_context_get_devices_count(ctx);
    for (unsigned int i = 0; i < ndev; i++) {
        const struct iio_device *dev = iio_context_get_device(ctx, i);
        const char *name = iio_device_get_name(dev);
        if (!name) name = "unknown";

        const char *id = iio_device_get_id(dev);
        char id_buf[32];
        if (!id) {
            snprintf(id_buf, sizeof(id_buf), "iio:device%u", i);
            id = id_buf;
        }

        int cs = get_cs_from_of_node(id);
        int gpio = (cs >= 0 && cs < (int)(sizeof(cs_to_gpio_map)/sizeof(cs_to_gpio_map[0])))
                     ? cs_to_gpio_map[cs] : -1;

        printf("%-12s  %-12s", id, name);
        if (cs >= 0) {
            if (gpio >= 0)
                printf("  CS%-2d  GPIO%d\n", cs, gpio);
            else
                printf("  CS%-2d  GPIO:N/A\n", cs);
        } else {
            printf("  CS:N/A  GPIO:N/A\n");
        }
    }

    iio_context_destroy(ctx);
}




// Find ad5662 device by chip select index
struct iio_device *find_ad5662_by_cs(struct iio_context *ctx, int cs_target) {
    if (!ctx || cs_target < 0) return NULL;

    unsigned int ndev = iio_context_get_devices_count(ctx);
    for (unsigned int i = 0; i < ndev; i++) {
        struct iio_device *dev = iio_context_get_device(ctx, i);
        const char *name = iio_device_get_name(dev);
        if (!name || strcmp(name, "ad5662") != 0)
            continue;

        const char *id = iio_device_get_id(dev);
        char id_buf[32];
        if (!id) {
            snprintf(id_buf, sizeof(id_buf), "iio:device%u", i);
            id = id_buf;
        }

        int cs = get_cs_from_of_node(id);
        if (cs == cs_target)
            return dev;  // Found match
    }

    return NULL;  // Not found
}


/**
 * @brief Write a raw DAC value to an AD5662 IIO device.
 *
 * This helper writes an unsigned integer value (0–65535) to the
 * `out_voltage0_raw` attribute of the AD5662 DAC device.
 *
 * @param dev   Pointer to the IIO device corresponding to the AD5662 DAC.
 *              Must not be NULL and must represent a valid DAC device.
 * @param value Unsigned integer value to write. Must be in the range 0–65535.
 *
 * @return 0 on success.
 * @return -1 if dev is NULL.
 * @return -2 if the value is outside the valid range.
 * @return -3 if the voltage0 channel is not found.
 * @return -4 if writing the attribute fails.
 */
int ad5662_write_raw(struct iio_device *dev, unsigned int value) {
    if (!dev)
        return -1;

    if (value > 65535) {
        fprintf(stderr, "Value out of range (0..65535): %u\n", value);
        return -2;
    }

    // AD5662 has one output channel (index 0)
    struct iio_channel *ch = iio_device_find_channel(dev, "voltage0", true);
    if (!ch) {
        fprintf(stderr, "Channel voltage0 not found\n");
        return -3;
    }

    int ret = iio_channel_attr_write_longlong(ch, "raw", value);
    if (ret < 0) {
        fprintf(stderr, "Failed to write raw attribute: %d\n", ret);
        return -4;
    }

    return 0; // success
}

/**
 * @brief Read the raw DAC value from an AD5662 IIO device.
 *
 * This helper reads the current unsigned integer value (0–65535) from the
 * `out_voltage0_raw` attribute of the AD5662 DAC device.
 *
 * @param dev   Pointer to the IIO device corresponding to the AD5662 DAC.
 *              Must not be NULL and must represent a valid DAC device.
 * @param value Pointer to an unsigned integer where the read value will be stored.
 *
 * @return 0 on success.
 * @return -1 if dev is NULL.
 * @return -2 if value pointer is NULL.
 * @return -3 if the voltage0 channel is not found.
 * @return -4 if reading the attribute fails.
 */
int ad5662_read_raw(struct iio_device *dev, unsigned int *value) {
    if (!dev)
        return -1;

    if (!value)
        return -2;

    struct iio_channel *ch = iio_device_find_channel(dev, "voltage0", true);
    if (!ch) {
        fprintf(stderr, "Channel voltage0 not found\n");
        return -3;
    }

    long long raw_val = 0;
    int ret = iio_channel_attr_read_longlong(ch, "raw", &raw_val);
    if (ret < 0) {
        fprintf(stderr, "Failed to read raw attribute: %d\n", ret);
        return -4;
    }

    *value = (unsigned int) raw_val;
    return 0; // success
}