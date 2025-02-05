# GPIO Expander

LimePSB RPCM v1.3 board features GPIO expander (MCP23017) controlled via I2C bus (address 0x20). GPIO expander controls LEDs, enable signals for PAs and LNAs, few other signals.

## Installation

Raspbian OS provides driver for MCP23017 GPIO expander. To enable it add this line to the /boot/firmware/config.txt file:

```
dtoverlay=mcp23017,noints,i2c_csi_dsi
```

After reboot GPIO expander may be detected using gpiodetect command. There should be the following lines (or similar) in the output

```
gpiochip0 [pinctrl-bcm2711] (58 lines)
gpiochip1 [raspberrypi-exp-gpio] (8 lines)
gpiochip2 [mcp23017] (16 lines)
gpiochip0 [pinctrl-bcm2711] (58 lines)
```

Our GPIO expander is gpiochip2. Using gpioinfo command it is possible to see the GPIOs:

```
gpiochip2 - 16 lines:
	line   0:     "GPIO0"       unused   input  active-high 
	line   1:     "GPIO1"       unused   input  active-high 
	line   2:     "GPIO2"       unused   input  active-high 
	line   3:     "GPIO3"       unused   input  active-high 
	line   4:     "GPIO4"       unused   input  active-high 
	line   5:     "GPIO5"       unused   input  active-high 
	line   6:     "GPIO6"       unused   input  active-high 
	line   7:     "GPIO7"       unused   input  active-high 
	line   8:     "GPIO8"       unused   input  active-high 
	line   9:     "GPIO9"       unused   input  active-high 
	line  10:     "GPIO10"      unused   input  active-high 
	line  11:     "GPIO11"      unused   input  active-high 
	line  12:     "GPIO12"      unused   input  active-high 
	line  13:     "GPIO13"      unused   input  active-high 
	line  14:     "GPIO14"      unused   input  active-high 
	line  15:     "GPIO15"      unused   input  active-high 
```

Although there is a custom version of device tree overlay which lists GPIO names according to LimePSB RPCM v1.3 schematic. Device tree source file is mcp23017_LimePSB-RPCM.dts. It may be compiled to binary using this command:

```
dtc -@ -I dts -O dtb -o mcp23017_LimePSB-RPCM.dtbo mcp23017_LimePSB-RPCM.dts
```

The output of this command is file mcp23017_LimePSB-RPCM.dtbo which should be copied to /boot/firmware/overlays/ folder. To enable it add this line to the /boot/firmware/config.txt file:

```
dtoverlay=mcp23017_LimePSB-RPCM,noints,i2c_csi_dsi
```

After reboot gpioinfo command output should be as follows:

```
gpiochip2 - 16 lines:
	line   0:     "LED1_R"       unused   input  active-high 
	line   1:     "LED1_G"       unused   input  active-high 
	line   2:     "LED2_R"       unused   input  active-high 
	line   3:     "LED2_G"       unused   input  active-high 
	line   4:     "LED3_R"       unused   input  active-high 
	line   5:     "LED3_G"       unused   input  active-high 
	line   6:     "LED4_R"       unused   input  active-high 
	line   7:     "LED4_G"       unused   input  active-high 
	line   8:  "EN_TXA_PA"       unused   input  active-high 
	line   9: "EN_RXA_LNA"       unused   input  active-high 
	line  10:  "EN_TXB_PA"       unused   input  active-high 
	line  11: "EN_RXB_LNA"       unused   input  active-high 
	line  12:  "XO_VC_SEL"       unused   input  active-high 
	line  13: "FPGA_CRESET" unused input active-high 
	line  14: "PTP_SYNC_SEL" unused input active-high 
	line  15:   "FAN_CTRL"       unused   input  active-high 
```
