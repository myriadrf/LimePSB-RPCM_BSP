#!/bin/bash

#    A script to configure Lattice iCE40 FPGA by SPI from Raspberry Pi
#
#    Copyright (C) 2015 Jan Marjanovic <jan@marjanovic.pro>
#    Copyright (C) 2025 Lime Microsystems <apps@limemicro.com>
#
#    This program is free software: you can redistribute it and/or modify
#    it under the terms of the GNU General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU General Public License for more details.
#
#    You should have received a copy of the GNU General Public License
#    along with this program.  If not, see <http://www.gnu.org/licenses/>.

echo ""
if [ $# -ne 1 ]; then
    echo "Usage: $0 FPGA-bin-file "
    exit 1
fi

if [ $EUID -ne 0 ]; then
    echo "This script must be run as root, exiting." 1>&2
    exit 1
fi

# Search for GPIO which controls CRESET
creset=$(gpiofind "FPGA_CRESET")

# Test if CRESET GPIO found in the system
if [ -z "$creset" ]
then
    echo "CRESET GPIO not foud, exiting."
    exit 1
fi
echo "OK: CRESET GPIO found"

# CRESET control
#gpioset $creset=0
#sleep 0.001
#gpioset $creset=1
#sleep 0.001

# Search for GPIO which controls Chip Select
chip_select=$(gpiofind "GPIO8")

# Test if Chip Select GPIO found in the system
if [ -z "$chip_select" ]
then
    echo "Chip Select GPIO not foud, exiting."
    exit 1
fi
echo "OK: Chip Select GPIO found"

# Check if SPI bus available
if [ -e /dev/spidev0.0 ]; then
    echo "OK: SPI driver loaded"
else
    echo "spidev does not exist"
    
    lsmod | grep spi_bcm2835 >& /dev/null

    if [ $? -ne 0 ]; then
	echo "SPI driver not loaded, try to load it..."
	modprobe spi_bcm2835

	if [ $? -eq 0 ]; then
	    echo "OK: SPI driver loaded"
	else
	    echo "Could not load SPI driver"
	    exit 1
	fi  
    fi
fi

# Set SPI speed and keep the file descriptor open. Will close it at the
# end of the script. It is requiret to not reset SPI speed.
spi-config -d /dev/spidev0.0 -s 5000000 -w &
PID=$!


#echo ""
#echo "Changing CSN direction to out"
#echo out > /sys/class/gpio/gpio520/direction
#cat /sys/class/gpio/gpio520/direction

echo "Setting CSN to low"
gpioset $chip_select=0
#gpioget $chip_select
#echo 0 > /sys/class/gpio/gpio520/value
#cat /sys/class/gpio/gpio520/value

#echo ""
#echo "Please power cycle the iCE40 FPGA board"
#echo "Press any key..."
#read

# CRESET control
echo "Setting CRESET to low"
gpioset $creset=0
gpioset --mode=time --usec=100 $creset=0
echo "Setting CRESET to high"
gpioset --mode=time --usec=100 $creset=1
#gpioget $creset

echo "Setting CSN to high"
gpioset $chip_select=1
#gpioget $chip_select
#echo 1 > /sys/class/gpio/gpio520/value
#cat /sys/class/gpio/gpio520/value

echo "Sending dummy byte"
echo -e "\x0" > /dev/spidev0.0
echo "Done"

echo "Setting CSN to low"
gpioset $chip_select=0
#gpioget $chip_select
#echo 0 > /sys/class/gpio/gpio520/value
#cat /sys/class/gpio/gpio520/value





echo "Continuing with configuration procedure"
dd if=$1 of=/dev/spidev0.0 status=progress
#cat $1 | spi-pipe -d /dev/spidev0.0

#echo -e "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" > /dev/spidev0.0

echo "Setting CSN to high"
gpioset $chip_select=1
#gpioget $chip_select
#echo 1 > /sys/class/gpio/gpio520/value
#cat /sys/class/gpio/gpio520/value

echo "Sending dummy bytes"
echo -e "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" > /dev/spidev0.0
#echo -e "\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0" > /dev/spidev0.0
echo "Done"

# Close SPI device descriptor
kill $PID

#
fpga_cdone=$(gpiofind "FPGA_CDONE")
if [ -z "$fpga_cdone" ]
then
    exit 1
fi

fpga_cdone_led=$(gpiofind "FPGA_CDONE_LED")
if [ -z "$fpga_cdone_led" ]
then
    exit 1
fi

done=$(gpioget $fpga_cdone)
gpioset $fpga_cdone_led=$done
sleep 1
