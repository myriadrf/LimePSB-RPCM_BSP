# Real Time Clock

LimePSB RPCM v1.3 board features RTC (PCF85063AT) controlled via I2C bus (address 0x51). 

## Installation

Raspbian OS provides driver for PCF85063AT RTC. To enable it add this line to the /boot/firmware/config.txt file:

```
dtoverlay=i2c-rtc,pcf85063a
```

Reboot.
