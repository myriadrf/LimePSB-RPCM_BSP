# Fan Control

This is fan control script for LimePSB RPCM CA23 v1.0 board. It lowers fan RPM at startup.

## Installation

To install it execute the command

```
sudo make install
```

This will copy fan_setup.sh file to /usr/local/bin/ folder:

```
sudo cp fan_setup.sh /usr/local/bin/
```

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
dtoverlay=emc2301
```

Make sure that the following lines are uncommented in /boot/firmware/config.txt file:

```
dtparam=i2c_arm=on
dtparam=i2c_vc=on
```

Reboot.