#!/bin/bash

usage() {
    echo "Usage:"
    echo "$0 EN_TXA_PA EN_RXA_LNA EN_TXB_PA EN_RXB_LNA"
    echo "Set EN_TXA_PA : 1 - enables TXA PA;  0 - disables TXA PA."
    echo "Set EN_RXA_LNA: 1 - enables RXA LNA; 0 - disables RXA LNA."
    echo "Set EN_TXA_PA : 1 - enables TXB PA;  0 - disables TXB PA."
    echo "Set EN_RXA_LNA: 1 - enables RXB LNA; 0 - disables RXB LNA."
    echo ""
    echo "Examples:"
    echo "To enalble all PAs and LNAs:"
    echo "$0 1 1 1 1"
    echo "To disable all PAs and LNAs:"
    echo "$0 0 0 0 0"
    echo "To enable TXA PA:"
    echo "$0 1 0 0 0"
}

echo ""

# Test if all the parameters are supplied
if [ $# -ne 4 ]; then
    usage
    exit 1
fi

# Test if supplied parameters are valid
for  param in "$@"; do
    if ! [[ "$param" =~ ^[0-1]$ ]]; then
	usage
	exit 1
    fi
done

# Find LEDs
led3R=$(gpiofind "LED3_R")
led3G=$(gpiofind "LED3_G")
led4R=$(gpiofind "LED4_R")
led4G=$(gpiofind "LED4_G")

# Test if GPIOs found in the system
if [ -z "$led3R" ] || [ -z "$led3G" ] || [ -z "$led4R" ] || [ -z "$led4G" ]
then
    echo "LED(s) not foud."
fi

# Find required GPIOs for PA and LNA control
en_txa_pa=$(gpiofind "EN_TXA_PA")
en_txb_pa=$(gpiofind "EN_TXB_PA")
en_rxa_lna=$(gpiofind "EN_RXA_LNA")
en_rxb_lna=$(gpiofind "EN_RXB_LNA")
#en_txa_pa=$(gpiofind "LED1_R")
#en_txb_pa=$(gpiofind "LED2_R")
#en_rxa_lna=$(gpiofind "LED3_R")
#en_rxb_lna=$(gpiofind "LED4_R")

# Test if GPIOs found in the system
if [ -z "$en_txa_pa" ] || [ -z "$en_txb_pa" ] || [ -z "$en_rxa_lna" ] || [ -z "$en_rxb_lna" ]
then
    echo "GPIO(s) not foud, exiting."
    exit 1
fi

# Set PA and LNA enable control values
gpioset $en_txa_pa=$1
gpioset $en_rxa_lna=$2
gpioset $en_txb_pa=$3
gpioset $en_rxb_lna=$4

# Set Channel A LEDs
if [ "$1" -eq 1 ] || [ "$2" -eq 1 ]
then
	# Green on, red off
	gpioset $led3G=0
	gpioset $led3R=1
else
	# Green off, red on
	gpioset $led3G=1
	gpioset $led3R=0
fi

# Set Channel B LEDs
if [ "$3" -eq 1 ] || [ "$4" -eq 1 ]
then
	# Green on, red off
	gpioset $led4G=0
	gpioset $led4R=1
else
	# Green off, red on
	gpioset $led4G=1
	gpioset $led4R=0
fi

exit 0