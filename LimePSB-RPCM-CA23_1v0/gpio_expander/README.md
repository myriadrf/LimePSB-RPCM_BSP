# GPIO Expander

LimePSB RPCM CA23 v1.0 board features three GPIO expanders (MCP23017) controlled via I2C bus (addresses 0x20, 0x21 and 0x22). GPIO expander controls LEDs, enable signals for PAs and LNAs, and other signals. There is a custom version of device tree overlay provided which lists GPIO names according to LimePSB RPCM CA23 v1.0 schematic. 

## Installation

Device tree source file is mcp23017_ca23.dts. It may be compiled to binary using this command:

```
dtc -@ -I dts -O dtb -o mcp23017_ca23.dtbo mcp23017_ca23.dts
```

The output of this command is file mcp23017_ca23.dtbo which should be copied to /boot/firmware/overlays/ folder:

```
sudo cp mcp23017_ca23.dtbo /boot/firmware/overlays/
```

To enable it add the following line to the /boot/firmware/config.txt file:

```
dtoverlay=mcp23017_ca23,noints,i2c_csi_dsi0

```

Reboot the system.

## Checking

After reboot GPIO expander may be detected using gpiodetect command. There should be the following lines (or similar) in the output

```
gpiochip0 [pinctrl-rp1] (54 lines)
gpiochip10 [gpio-brcmstb@107d508500] (32 lines)
gpiochip11 [gpio-brcmstb@107d517c00] (15 lines)
gpiochip12 [gpio-brcmstb@107d517c20] (6 lines)
gpiochip13 [gpio-brcmstb@107d508520] (4 lines)
gpiochip14 [mcp23017] (16 lines)
gpiochip15 [mcp23017] (16 lines)
gpiochip16 [mcp23017] (16 lines)
gpiochip0 [pinctrl-rp1] (54 lines)
```

Our GPIO expanders in this example are gpiochip14, gpiochip15, gpiochip16. Using gpioinfo command it is possible to see the GPIOs:

```
<...>
gpiochip14 - 16 lines:
	line   0:     "LED1_R"       unused   input  active-high 
	line   1:     "LED1_G"       unused   input  active-high 
	line   2:     "LED2_R"       unused   input  active-high 
	line   3:     "LED2_G"       unused   input  active-high 
	line   4:     "LED3_R"       unused   input  active-high 
	line   5:     "LED3_G"       unused   input  active-high 
	line   6:     "LED4_R"       unused   input  active-high 
	line   7:     "LED4_G"       unused   input  active-high 
	line   8: "FPGA_PROGRAM_B" unused input active-high 
	line   9:  "FPGA_DONE"       unused   input  active-high 
	line  10: "FPGA_FLASH_SEL" unused input active-high 
	line  11: "ADF_MUXOUT"       unused   input  active-high 
	line  12:   "POE_TYP4"       unused   input  active-high 
	line  13: "USB_PD_FAULT" unused input active-high 
	line  14: "M.2_LED_WWAN" unused input active-high 
	line  15: "M.2_W_DISABLE_1" unused input active-high 
gpiochip15 - 16 lines:
	line   0: "EN_VCC3P3_RF1" unused input active-high 
	line   1: "EN_VCC5P0_RF1" unused input active-high 
	line   2: "EN_VCC8P0_RF1" unused input active-high 
	line   3: "EN_TX1_PA_VDD" unused input active-high 
	line   4: "EN_TX1_PA_VG2" unused input active-high 
	line   5: "EN_VCC3P3_RF2" unused input active-high 
	line   6: "EN_VCC5P0_RF2" unused input active-high 
	line   7: "EN_VCC8P0_RF2" unused input active-high 
	line   8: "EN_TX2_PA_VDD" unused input active-high 
	line   9: "EN_TX2_PA_VG2" unused input active-high 
	line  10: "SW_REG_RST"       unused   input  active-high 
	line  11:    "RFPD_EN"       unused   input  active-high 
	line  12: "RPI_SYNC_IO_SEL" unused input active-high 
	line  13: "RPI_SYNC_IN_SEL" unused input active-high 
	line  14: "SYNC_OUT_SEL" unused input active-high 
	line  15: "EXT_SYNC_OUT_SEL" unused input active-high 
gpiochip16 - 16 lines:
	line   0: "M.2_CONFIG_0" unused input active-high 
	line   1: "M.2_CONFIG_1" unused input active-high 
	line   2: "M.2_CONFIG_2" unused input active-high 
	line   3: "M.2_CONFIG_3" unused input active-high 
	line   4: "RF_SW_LMS_TX" unused input active-high 
	line   5: "RF_SW_LMS_RX2" unused input active-high 
	line   6: "RF_SW_LMS_RX3" unused input active-high 
	line   7: "RF_SW_FBCAK" unused input active-high 
	line   8: "RX1_ADD_V1"       unused   input  active-high 
	line   9: "RX1_ADD_V2"       unused   input  active-high 
	line  10: "RX1_ADD_V3"       unused   input  active-high 
	line  11: "RX1_ADD_V4"       unused   input  active-high 
	line  12: "RX2_ADD_V1"       unused   input  active-high 
	line  13: "RX2_ADD_V2"       unused   input  active-high 
	line  14: "RX2_ADD_V3"       unused   input  active-high 
	line  15: "RX2_ADD_V4"       unused   input  active-high 
<...>
```

