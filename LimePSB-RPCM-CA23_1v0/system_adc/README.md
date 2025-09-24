# System Voltage ADC

LimePSB RPCM CA23 v1.0 board features ADC (MCP3208) controlled via SPI bus. It is used to measure various on-board voltages.

## Device Tree

Raspberry Pi OS provides driver for MCP3208 but there is no proper device tree overlay. The following instructions will help to create device tree overlay based on mcp3202.dtbo.

Copy /boot/firmware/overlays/mcp3202.dtbo file.
Convert it to DTS:
  
```
dtc -I dtb -O dts -o mcp3208.dts mcp3202.dtbo
```

Change all ocurances of mcp3202 to mcp3208 in the mcp3208.dts file.

Create dtbo file:
  
```
dtc -@ -I dts -O dtb -o mcp3208.dtbo mcp3208.dts
```

Copy compiled file to overlay folder:
  
```
sudo cp mcp3208.dtbo /boot/firmware/overlays/
```

## Installation

Raspberry Pi OS provides driver for MCP3208 ADC. To enable it add this line to the /boot/firmware/config.txt file:

```
dtoverlay=mcp3208,spi1-0-present
```

Make sure there is a following line in the /boot/firmware/config.txt file (cs0_pin parameter is most important while ADC is controlled by CS0 pin):

```
dtoverlay=spi1-1cs,cs0_pin=18
```

Reboot.

## Check if it is Working

Navigate to /sys/bus/iio/devices/iio:device0

Note please that there may be more IIO devices. In this case our ADC may be represented by different iio:deviceN, not iio:device0.

Read first analog channel:

```
cat in_voltage0_raw
```
