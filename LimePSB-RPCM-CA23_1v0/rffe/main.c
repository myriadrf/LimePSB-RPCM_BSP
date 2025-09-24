/*
 * PAC1720 current/voltage measurement example
 * One-shot mode, dual channel
 *
 * Build: gcc -o pac1720_measure pac1720_measure.c
 * Run:   sudo ./pac1720_measure /dev/i2c-10 0x4c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <getopt.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>

#include <iio.h>

#include "src/dac/dac_app.h"
#include "src/helpers/helpers_app.h"
#include "src/rffe/rffe_app.h"
#include "src/adc/pac1720_app.h"
#include "src/pd/pd_app.h"





/* --- Helper functions --- */
void print_help(const char *progname) {
    printf("Usage: %s [options] [arguments]\n", progname);
    printf("Options:\n");
    printf("  -h, --help             Show this help message\n");
    printf("  -v, --version          Show program version\n");
    //printf("  -o, --output <file>    Specify output file\n");
    //printf("      --rx1 <1-10>       Set RX1 parameter\n");
    //printf("      --rx2 <1-10>       Set RX2 parameter\n");
    //printf("      --trx1 <1-10>      Set TRX1 parameter\n");
    //printf("      --trx2 <1-10>      Set TRX2 parameter\n");
    //printf("      --trx1ant <1-10>   Set TRX1ANT parameter\n");
    //printf("      --trx2ant <1-10>   Set TRX2ANT parameter\n");
    printf("      --lmstx <1,2>      Set LMS7002M transmit band\n");
    printf("      --lmsrx <l,h,w>    Set LMS7002M receive band\n");
    printf("      --rfband <1, 3, 7, 8, 28, 78, 48, 42, 41, 38> Set RF Frontend band\n");
    
    //printf("      --paoff <1,2>      Turn PA of RF channel off\n");
    //printf("      --paon <1,2>       Turn PA of RF channel on\n");
    //printf("      --lnaoff <1,2>     Turn PA of RF channel off\n");
    //printf("      --lnaon <1,2>      Turn PA of RF channel on\n");

    printf("      --femoff <1,2>     Turn RF FEM off\n");
    printf("      --femon <1,2>      Turn RF FEM on\n");

    printf("      --rflb <0,1>       Turn RF loopback off/on.\n");
    printf("      --rx1att <0-15>    Set RX1 attenuation.\n");
    printf("      --rx2att <0-15>    Set RX2 attenuation.\n");
    printf("      --pd <1,2>         Read power detector value\n");
    printf("      --tdd <0,1>        Set TDD signal from FPGA to 0 or 1.\n");
    printf("      --adc              Get data from PAC1720.\n");
    printf("      --tst              Used for developing.\n");
}

void print_version(const char *progname) {
    printf("%s version 1.0\n", progname);
}

/**
 * Parse string as integer and check if it's in the allowed range.
 *
 * Allowed range: [0 - 15]
 *
 * @param arg      String argument to validate
 * @param optname  Option name.
 * @return      Parsed integer if valid. Exits program on error.
 */
int parse_int_0_to_15(const char *arg, const char *optname) {
    char *endptr;
    errno = 0;
    long val = strtol(arg, &endptr, 10);

    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Error: %s expects an integer, got '%s'\n", optname, arg);
        exit(EXIT_FAILURE);
    }

    if (val < 0 || val > 15) {
        fprintf(stderr, "Error: %s value must be between 0 and 15, got %ld\n", optname, val);
        exit(EXIT_FAILURE);
    }

    return (int)val;
}

/**
 * Parse string as integer and check if it's in the allowed range.
 *
 * Allowed range: [1 - 10]
 *
 * @param arg      String argument to validate
 * @param optname  Option name.
 * @return      Parsed integer if valid. Exits program on error.
 */
int parse_int_1_to_10(const char *arg, const char *optname) {
    char *endptr;
    errno = 0;
    long val = strtol(arg, &endptr, 10);

    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Error: %s expects an integer, got '%s'\n", optname, arg);
        exit(EXIT_FAILURE);
    }

    if (val < 1 || val > 10) {
        fprintf(stderr, "Error: %s value must be between 1 and 10, got %ld\n", optname, val);
        exit(EXIT_FAILURE);
    }

    return (int)val;
}

/**
 * Parse string as integer and check if it's in the allowed range.
 *
 * Allowed range: [1, 2]
 *
 * @param arg      String argument to validate
 * @param optname  Option name.
 * @return      Parsed integer if valid. Exits program on error.
 */
int parse_int_1_to_2(const char *arg, const char *optname) {
    char *endptr;
    errno = 0;
    long val = strtol(arg, &endptr, 10);

    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Error: %s expects an integer, got '%s'\n", optname, arg);
        exit(EXIT_FAILURE);
    }

    if (val < 1 || val > 2) {
        fprintf(stderr, "Error: %s value must be 1 or 2, got %ld\n", optname, val);
        exit(EXIT_FAILURE);
    }

    return (int)val;
}

/**
 * Parse string as integer and check if it's in the allowed range.
 *
 * Allowed range: [0, 1]
 *
 * @param arg      String argument to validate
 * @param optname  Option name.
 * @return      Parsed integer if valid. Exits program on error.
 */
int parse_int_0_to_1(const char *arg, const char *optname) {
    char *endptr;
    errno = 0;
    long val = strtol(arg, &endptr, 10);

    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Error: %s expects an integer, got '%s'\n", optname, arg);
        exit(EXIT_FAILURE);
    }

    if (val < 0 || val > 1) {
        fprintf(stderr, "Error: %s value must be 0 or 1, got %ld\n", optname, val);
        exit(EXIT_FAILURE);
    }

    return (int)val;
}

/**
 * Parse string option and check if it's one of "l", "w", or "h".
 *
 * @param arg       String argument to validate.
 * @param optname   Option name.
 * @return      0 = l, 1 = w, 2 = h on success. Exits program on error.
 */
int parse_lwh_option(const char *arg, const char *optname) {
    if (!arg) {
        fprintf(stderr, "Error: missing %s argument\n", optname);
        exit(EXIT_FAILURE);
    }

    if (strcmp(arg, "l") == 0) {
        return 0;
    } else if (strcmp(arg, "w") == 0) {
        return 1;
    } else if (strcmp(arg, "h") == 0) {
        return 2;
    } else {
        fprintf(stderr, "Error: invalid %s argument '%s'. Must be 'l', 'w', or 'h'.\n", optname, arg);
        exit(EXIT_FAILURE);
    }
}

/**
 * Parse string as integer and check if it's in the allowed set.
 *
 * Allowed numbers: {1, 3, 7, 8, 28, 78, 48, 42, 41, 38}
 *
 * @param arg   String argument to validate
 * @return      Parsed integer if valid. Exits program on error.
 */
int parse_rf_bands(const char *arg, const char *optname) {
    static const int allowed[] = {1, 3, 7, 8, 28, 78, 48, 42, 41, 38};
    const size_t allowed_count = sizeof(allowed) / sizeof(allowed[0]);

    if (!arg) {
        fprintf(stderr, "Error: missing %s argument\n", optname);
        exit(EXIT_FAILURE);
    }

    char *endptr = NULL;
    errno = 0;
    long val = strtol(arg, &endptr, 10);

    if (errno != 0 || *endptr != '\0') {
        fprintf(stderr, "Error: invalid %s integer argument '%s'\n", optname, arg);
        exit(EXIT_FAILURE);
    }

    for (size_t i = 0; i < allowed_count; i++) {
        if (val == allowed[i]) {
            return (int)val;
        }
    }

    fprintf(stderr, "Error: invalid %s value '%ld'. Allowed values are: ", optname, val);
    for (size_t i = 0; i < allowed_count; i++) {
        fprintf(stderr, "%d%s", allowed[i], (i < allowed_count - 1) ? ", " : "\n");
    }
    exit(EXIT_FAILURE);
}

/* --- RF parameters grouped in a struct --- */
struct rf_param {
    int value;
    int set;
};

struct rf_params {
    struct rf_param rx1;
    struct rf_param rx2;
    struct rf_param trx1;
    struct rf_param trx2;
    struct rf_param trx1ant;
    struct rf_param trx2ant;
    struct rf_param lmstx;
    struct rf_param lmsrx;
    struct rf_param rfband;
    struct rf_param paoff;
    struct rf_param paon;
    struct rf_param lnaoff;
    struct rf_param lnaon;
    struct rf_param femoff;
    struct rf_param femon;
    struct rf_param rflb;
    struct rf_param rx1att;
    struct rf_param rx2att;
    struct rf_param pd;
    struct rf_param tdd;
    struct rf_param adc;
    struct rf_param tst;
};

/* --- Top-level options struct --- */
struct options {
    char *output_file;
    struct rf_params rf;
};

/* Option identifiers for getopt_long */
enum {
    OPT_RX1 = 1000,
    OPT_RX2,
    OPT_TRX1,
    OPT_TRX2,
    OPT_TRX1ANT,
    OPT_TRX2ANT,
    OPT_LMSTX,
    OPT_LMSRX,
    OPT_RFBAND,
    OPT_PAOFF,
    OPT_PAON,
    OPT_LNAOFF,
    OPT_LNAON,
    OPT_FEMOFF,
    OPT_FEMON,
    OPT_RFLB,
    OPT_RX1ATT,
    OPT_RX2ATT,
    OPT_PD,
    OPT_TDD,
    OPT_ADC,
    OPT_TST
};

/* Process options */
void process_options(int argc, char *argv[], struct options *opts) {
    int opt;
    int option_index = 0;

    static struct option long_options[] = {
        {"help",    no_argument,       0, 'h'},
        {"version", no_argument,       0, 'v'},
        {"output",  required_argument, 0, 'o'},
        {"rx1",     required_argument, 0, OPT_RX1},
        {"rx2",     required_argument, 0, OPT_RX2},
        {"trx1",    required_argument, 0, OPT_TRX1},
        {"trx2",    required_argument, 0, OPT_TRX2},
        {"trx1ant", required_argument, 0, OPT_TRX1ANT},
        {"trx2ant", required_argument, 0, OPT_TRX2ANT},
        {"lmstx",   required_argument, 0, OPT_LMSTX},
        {"lmsrx",   required_argument, 0, OPT_LMSRX},
        {"rfband",  required_argument, 0, OPT_RFBAND},
        {"paoff",   required_argument, 0, OPT_PAOFF},
        {"paon",    required_argument, 0, OPT_PAON},
        {"lnaoff",  required_argument, 0, OPT_LNAOFF},
        {"lnaon",   required_argument, 0, OPT_LNAON},
        {"femoff",  required_argument, 0, OPT_FEMOFF},
        {"femon",   required_argument, 0, OPT_FEMON},
        {"rflb",    required_argument, 0, OPT_RFLB},
        {"rx1att",  required_argument, 0, OPT_RX1ATT},
        {"rx2att",  required_argument, 0, OPT_RX2ATT},
        {"pd",      required_argument, 0, OPT_PD},
        {"tdd",     required_argument, 0, OPT_TDD},
        {"adc",     no_argument,       0, OPT_ADC},
        {"tst",     no_argument,       0, OPT_TST},
        {0, 0, 0, 0}
    };

    while ((opt = getopt_long(argc, argv, "hvo:", long_options, &option_index)) != -1) {
        switch (opt) {
        case 'h':
            print_help(argv[0]);
            exit(EXIT_SUCCESS);

        case 'v':
            print_version(argv[0]);
            exit(EXIT_SUCCESS);

        case 'o':
            opts->output_file = optarg;
            break;

        case OPT_RX1:
            opts->rf.rx1.value = parse_int_1_to_10(optarg, "--rx1");
            opts->rf.rx1.set = 1;
            break;

        case OPT_RX2:
            opts->rf.rx2.value = parse_int_1_to_10(optarg, "--rx2");
            opts->rf.rx2.set = 1;
            break;

        case OPT_TRX1:
            opts->rf.trx1.value = parse_int_1_to_10(optarg, "--trx1");
            opts->rf.trx1.set = 1;
            break;

        case OPT_TRX2:
            opts->rf.trx2.value = parse_int_1_to_10(optarg, "--trx2");
            opts->rf.trx2.set = 1;
            break;

        case OPT_TRX1ANT:
            opts->rf.trx1ant.value = parse_int_1_to_10(optarg, "--trx1ant");
            opts->rf.trx1ant.set = 1;
            break;

        case OPT_TRX2ANT:
            opts->rf.trx2ant.value = parse_int_1_to_10(optarg, "--trx2ant");
            opts->rf.trx2ant.set = 1;
            break;

        case OPT_LMSTX:
            opts->rf.lmstx.value = parse_int_1_to_2(optarg, "--lmstx");
            opts->rf.lmstx.set = 1;
            break;

        case OPT_LMSRX:
            opts->rf.lmsrx.value = parse_lwh_option(optarg, "--lmsrx");
            opts->rf.lmsrx.set = 1;
            break;

        case OPT_RFBAND:
            opts->rf.rfband.value = parse_rf_bands(optarg, "--rfband");
            opts->rf.rfband.set = 1;
            break;

        case OPT_PAOFF:
            opts->rf.paoff.value = parse_int_1_to_2(optarg, "--paoff");
            opts->rf.paoff.set = 1;
            break;

        case OPT_PAON:
            opts->rf.paon.value = parse_int_1_to_2(optarg, "--paon");
            opts->rf.paon.set = 1;
            break;

        case OPT_LNAOFF:
            opts->rf.lnaoff.value = parse_int_1_to_2(optarg, "--lnaoff");
            opts->rf.lnaoff.set = 1;
            break;

        case OPT_LNAON:
            opts->rf.lnaon.value = parse_int_1_to_2(optarg, "--lnaon");
            opts->rf.lnaon.set = 1;
            break;

        case OPT_FEMOFF:
            opts->rf.femoff.value = parse_int_1_to_2(optarg, "--femoff");
            opts->rf.femoff.set = 1;
            break;

        case OPT_FEMON:
            opts->rf.femon.value = parse_int_1_to_2(optarg, "--femon");
            opts->rf.femon.set = 1;
            break;

        case OPT_RFLB:
            opts->rf.rflb.value = parse_int_0_to_1(optarg, "--rflb");
            opts->rf.rflb.set = 1;
            break;

        case OPT_RX1ATT:
            opts->rf.rx1att.value = parse_int_0_to_15(optarg, "--rx1att");
            opts->rf.rx1att.set = 1;
            break;

        case OPT_RX2ATT:
            opts->rf.rx2att.value = parse_int_0_to_15(optarg, "--rx2att");
            opts->rf.rx2att.set = 1;
            break;

        case OPT_PD:
            opts->rf.pd.value = parse_int_1_to_2(optarg, "--pd");
            opts->rf.pd.set = 1;
            break;

        case OPT_TDD:
            opts->rf.tdd.value = parse_int_0_to_1(optarg, "--tdd");
            opts->rf.tdd.set = 1;
            break;

        case OPT_ADC:
            opts->rf.adc.set = 1;
            break;

        case OPT_TST:
            opts->rf.tst.set = 1;
            break;

        case '?':
            print_help(argv[0]);
            exit(EXIT_FAILURE);

        default:
            fprintf(stderr, "Unexpected option\n");
            exit(EXIT_FAILURE);
        }
    }
}


//
void execute_commands(struct options *opts)
{
    //unsigned int val = 0;

   //
   /*
    if (opts->rf.rx1.set)
    {
        val = limeSPI_read("FPGA", RX1_RF_SWITCH_ADDR);
        val = (val & 0xFF00) | rf_sw_channel[opts->rf.rx1.value];
        if (limeSPI_write("FPGA", RX1_RF_SWITCH_ADDR, val) == 0)
        {
            printf("RX1 value: %d\n", opts->rf.rx1.value);
        }
    }

    //
    if (opts->rf.rx2.set)     
    {
        val = limeSPI_read("FPGA", RX2_RF_SWITCH_ADDR);
        val = (val & 0xFF00) | rf_sw_channel[opts->rf.rx2.value];
        if (limeSPI_write("FPGA", RX2_RF_SWITCH_ADDR, val) == 0)
        {
            printf("RX2 value: %d\n", opts->rf.rx2.value);
        }
    }

    //
    if (opts->rf.trx1.set)
    {
        val = limeSPI_read("FPGA", TRX1_RF_SWITCH_ADDR);
        val = (val & 0x00FF) | (rf_sw_channel[opts->rf.trx1.value] << 8);
        if (limeSPI_write("FPGA", TRX1_RF_SWITCH_ADDR, val) == 0)
        {
            printf("TRX1 value: %d\n", opts->rf.trx1.value);
        }
    }

    //
    if (opts->rf.trx2.set)
    {
        val = limeSPI_read("FPGA", TRX2_RF_SWITCH_ADDR);
        val = (val & 0x00FF) | (rf_sw_channel[opts->rf.trx2.value] << 8);
        if (limeSPI_write("FPGA", TRX2_RF_SWITCH_ADDR, val) == 0)
        {
            printf("TRX2 value: %d\n", opts->rf.trx2.value);
        }
    }

    //
    if (opts->rf.trx1ant.set)
    {
        if (limeSPI_write("FPGA", TRX1_ANT_RF_SWITCH_ADDR, rf_sw_channel[opts->rf.trx1ant.value]) == 0)
        {
            printf("Wrote TRX1ANT value: %d\n", opts->rf.trx1ant.value);
        }
    }

    //
    if (opts->rf.trx2ant.set)
    {
        if (limeSPI_write("FPGA", TRX2_ANT_RF_SWITCH_ADDR, rf_sw_channel[opts->rf.trx2ant.value]) == 0)
        {
            printf("Wrote TRX2ANT value: %d\n", opts->rf.trx2ant.value);
        }
    }
    */

    //
    if (opts->rf.lmstx.set)
    {
        if (rffe_set_lms_tx(opts->rf.lmstx.value) == 0)
        {
            printf("LMS7002M transmit band set to: %d\n", opts->rf.lmstx.value);
        }
    }

    //
    if (opts->rf.lmsrx.set)
    {
        char rxBand; 
        rxBand = opts->rf.lmsrx.value == 0 ? 'L' : opts->rf.lmsrx.value == 1 ? 'W' : 'H';
        if (rffe_set_lms_rx(opts->rf.lmsrx.value) == 0)
        {
            printf("LMS7002M receive band set to: %c\n", rxBand);
        }
    }

    //
    if (opts->rf.rfband.set)
    {
        if (rffe_set_band(1, opts->rf.rfband.value) == 0)
        {
            printf("RF frontend channel 1 set to band: %d\n", opts->rf.rfband.value);
        }

        if (rffe_set_band(2, opts->rf.rfband.value) == 0)
        {
            printf("RF frontend channel 2 set to band: %d\n", opts->rf.rfband.value);
        }
    }

    //
    if (opts->rf.paoff.set)
    {
        if (rffe_pa_off(opts->rf.paoff.value) == 0)
        {
            printf("PA of RF channel %d is off\n", opts->rf.paoff.value);
        }
    }

    //
    if (opts->rf.paon.set)
    {
        if (rffe_pa_on(opts->rf.paon.value) == 0)
        {
            printf("PA of RF channel %d is on\n", opts->rf.paon.value);
        }
    }

    //
    if (opts->rf.lnaoff.set)
    {
        if (rffe_lna_off(opts->rf.lnaoff.value) == 0)
        {
            printf("LNA of RF channel %d is off\n", opts->rf.lnaoff.value);
        }
    }

    //
    if (opts->rf.lnaon.set)
    {
        if (rffe_lna_on(opts->rf.lnaon.value) == 0)
        {
            printf("LNA of RF channel %d is on\n", opts->rf.lnaon.value);
        }
    }

    //
    if (opts->rf.femoff.set)
    {
        if (rffe_pa_off(opts->rf.femoff.value) == 0)
        {
            printf("PA of RF channel %d is off\n", opts->rf.femoff.value);
        }
        if (rffe_lna_off(opts->rf.femoff.value) == 0)
        {
            printf("LNA of RF channel %d is off\n", opts->rf.femoff.value);
        }
    }

    //
    if (opts->rf.femon.set)
    {
        if (rffe_pa_on(opts->rf.femon.value) == 0)
        {
            printf("PA of RF channel %d is on\n", opts->rf.femon.value);
        }

        if (rffe_lna_on(opts->rf.femon.value) == 0)
        {
            printf("LNA of RF channel %d is on\n", opts->rf.femon.value);
        }
    }

    //
    if (opts->rf.rx1att.set)
    {
        if (rffe_rxatt(1, opts->rf.rx1att.value) == 0)
        {
            printf("Attenuation of channel 1 is set to %d\n", opts->rf.rx1att.value);
        }
    }

    //
    if (opts->rf.rx2att.set)
    {
        if (rffe_rxatt(2, opts->rf.rx2att.value) == 0)
        {
            printf("Attenuation of channel 2 is set to %d\n", opts->rf.rx2att.value);
        }
    }

    //
    if (opts->rf.rflb.set)
    {
        if (rffe_rflb(opts->rf.rflb.value) == 0)
        {
            printf("Onboard RF loopback is ");
            printf(opts->rf.rflb.value == 0 ? "off\n" : "on\n");
        }
    }

    //
    if (opts->rf.pd.set)
    {
        int pdval = pd_read(opts->rf.pd.value);
        if (pdval >= 0)
        {
            printf("PD value of channel%d = %d\n", opts->rf.pd.value, pdval);
        }
    }

    //
    if (opts->rf.tdd.set)
    {
        if (rffe_tdd(opts->rf.tdd.value) == 0)
        {
            printf("TDD is set to %d\n", opts->rf.tdd.value);
        }
    }

    //
    if (opts->rf.adc.set)
    {
        pac1720_measure();
    }

    //
    if (opts->rf.tst.set)
    {
        //pac1720_measure();
        pd_read(1);
        pd_read(2);
    }

}

/* --- Main function --- */
int main(int argc, char *argv[]) {
    struct options opts = {0};

    // Process all command line options
    process_options(argc, argv, &opts);

    // Print non-option arguments
    if (optind < argc) {
        printf("Non-option arguments:\n");
        while (optind < argc)
            printf("  %s\n", argv[optind++]);
    }

    if (opts.output_file)
        printf("Output file: %s\n", opts.output_file);

    // Execute commands
    //execute_rf_switch(&opts);
    execute_commands(&opts);
/*
    int val;

    // Read register
    val = limeSPI_read("FPGA", 0x00d7);
    if (val >= 0) {
        printf("Before: Register 0x00d7 = 0x%04X\n", val);
    }

    // Write register
    if (limeSPI_write("FPGA", 0x00d7, 0x0004) == 0) {
        printf("Wrote 0x0040 to 0x00d7\n");
    }

    // Read back
    val = limeSPI_read("FPGA", 0x00d7);
    if (val >= 0) {
        printf("After: Register 0x00d7 = 0x%04X\n", val);
    }

    gpio_set_value("LED1_R", 1);
*/
    return EXIT_SUCCESS;
}
