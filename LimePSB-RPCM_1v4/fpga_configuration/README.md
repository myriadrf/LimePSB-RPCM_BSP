# FPGA Configuration

LimePSB RPCM v1.4 board features on board FPGA (Lattice ICE5LP4K). It has to be configured at system startup to allow additional functions of the on board infrastructure to be controlled.

## Prerequisites

It requires GPIO expander driver and spi-tools to be installed. To install spi-tools use this [repository](https://github.com/cpb-/spi-tools) and follow instructions.

## Installation

To install it execute the command

```
sudo make install
```

This will copy fpga_conf.sh and LimePSB_RPCM_top_bitmap.bin files to /usr/local/bin/ folder.
To execute it at startup set up the crontab task. Open crontab file by executing the following command:

```
sudo crontab -e
```

Add the following line at the end of the file:

```
@reboot sudo bash /usr/local/bin/fpga_conf.sh /usr/local/bin/LimePSB_RPCM_top_bitmap.bin
```

Add the following line to the /boot/firmware/config.txt file:

```
dtoverlay=spi0-0cs
```

Reboot.
