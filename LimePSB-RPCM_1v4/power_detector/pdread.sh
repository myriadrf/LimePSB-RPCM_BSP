#!/bin/bash

#    A script to configure read value from LTC5587 power detector
#
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


read_value() {
    # Read 2 bytes of binary data from SPI
    raw_data=$(spi-pipe --device=/dev/spidev0.0 -b 2 -n 1 < /dev/zero)

    # Get hex representation
    hex_data=$(echo -n "$raw_data" | xxd -p)

    # Ensure exactly 4 hex digits (2 bytes)
    if [[ ${#hex_data} -ne 4 ]]; then
	echo "Error: Expected 2 bytes (4 hex characters), got '$hex_data'"
	exit 1
    fi

    # Extract high and low bytes
    high_byte=${hex_data:0:2}
    low_byte=${hex_data:2:2}

    # Convert to decimal (big-endian)
    decimal_value=$((16#$high_byte * 256 + 16#$low_byte))
    decimal_value=$((decimal_value >> 4))

    echo "High byte: $high_byte"
    echo "Low byte: $low_byte"
    echo "Decimal value: $decimal_value"
}


echo ""

if [ $EUID -ne 0 ]; then
    echo "This script must be run as root, exiting." 1>&2
    exit 1
fi

# Search for GPIO which controls CRESET
gpio25=$(gpiofind "GPIO25")
# Search for GPIO which controls CRESET
gpio26=$(gpiofind "GPIO26")
# Search for RFPD_EN which controls CRESET
rfpd_en=$(gpiofind "RFPD_EN")

# Test if CRESET GPIO found in the system
if [ -z "$gpio25" ]
then
    echo "GPIO25 not foud, exiting."
    exit 1
fi
echo "OK: GPIO25 found"

# Test if CRESET GPIO found in the system
if [ -z "$gpio26" ]
then
    echo "GPIO26 not foud, exiting."
    exit 1
fi
echo "OK: GPIO26 found"

# Test if RFPD_EN GPIO found in the system
if [ -z "$rfpd_en" ]
then
    echo "RFPD_EN not foud, exiting."
    exit 1
fi
echo "OK: RFPD_EN found"


# Check SPI bus
if [ -e /dev/spidev0.0 ]; then
    echo "OK: SPI driver loaded"
else
    echo "spidev does not exist"
    
#    lsmod | grep spi_bcm2708 >& /dev/null
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

# Enable power detectors
echo "Enabling detectors"
gpioset $rfpd_en=1
sleep 0.001

# Set SPI speed and keep the file descriptor open. Will close it at the
# end of the script. It is requiret to not reset SPI speed.
spi-config -d /dev/spidev0.0 -s 500000 -w &
PID=$!

# Initialize GPIO lines to default
echo "Initializing GPIO25 and to GPIO26 high"
gpioset $gpio25=1
gpioset $gpio26=1
sleep 0.001

#
# Read channel A
#

# Dummy read, channel A
echo "Setting GPIO25 to low, dummy read"
gpioset $gpio25=0
sleep 0.001

# Dummy read, channel A
echo "Setting GPIO25 to high"
gpioset $gpio25=1
sleep 0.001

# Actual read, channel A
echo "Setting GPIO25 to low, actual read"
gpioset $gpio25=0

# Read and print power detector value, channel A
echo "Channel A value:"
read_value

# End of reading channel A
echo "Setting GPIO25 to high"
gpioset $gpio25=1

#
# Read channel B
#

# Dummy read, channel B
echo "Setting GPIO26 to low, dummy read"
gpioset $gpio26=0
sleep 0.001

# Dummy read, channel B
echo "Setting GPIO26 to high"
gpioset $gpio26=1
sleep 0.001

# Actual read, channel B
echo "Setting GPIO26 to low, actual read"
gpioset $gpio26=0


# Read and print power detector value, channel B
echo "Channel B value:"
read_value

# End of reading channel B
echo "Setting GPIO26 to high"
gpioset $gpio26=1

#
# Finalize
#

echo "Disabling detectors"
gpioset $rfpd_en=0

# Close SPI device descriptor
kill $PID

#
echo "Done"

