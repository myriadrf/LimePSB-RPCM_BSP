# Digital Temperature Sensor

LimePSB RPCM CA23 v1.0 board features Digital Temperature Sensor (LM75) controlled via I2C bus (address 0x48). 

## Installation

Raspbian OS provides driver for LM75. To enable it add this line to the /boot/firmware/config.txt file:

```
dtoverlay=i2c-sensor,addr=0x48,lm75,i2c_csi_dsi0
```

Reboot.
