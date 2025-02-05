# Fan Control

This is fan control script for LimePSB RPCM v1.3 board. It lowers fan RPM at startup.

## Installation

To install it execute the command

```
sudo make install
```

This will copy fan_setup.sh file to /usr/local/bin/ folder.
To execute it at startup set up the crontab task. Open crontab file by executing the following command:

```
sudo crontab -e
```

Add the following line at the end of the file:

```
@reboot sudo bash /usr/local/bin/fan_setup.sh
```

Add the following line to the /boot/firmware/config.txt file:

```
dtoverlay=i2c-rtc,pcf85063a,i2c_csi_dsi,i2c-fan,emc2301
```

Make sure that the following lines are uncommented in /boot/firmware/config.txt file:

```
dtparam=i2c_arm=on
dtparam=i2c_vc=on
```

Reboot.