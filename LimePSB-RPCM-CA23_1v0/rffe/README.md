# Radio Frequency Front End (RFFE)

The LimePSB RPCM CA23 v1.0 board integrates a complete radio frequency (RF) front-end subsystem. The RF front end includes:
- Power Amplifiers (PA) – provide transmit signal amplification.
- Low-Noise Amplifiers (LNA) – improve receive sensitivity by minimizing noise contribution.
- Band Filters – ensure signal integrity by suppressing out-of-band interference.

Control and configuration of the RF front-end functions are managed via the rffe CLI software utility. This interface provides command-line access for adjusting, monitoring, and validating RF front-end parameters.

## Prerequisites

Operation of the RF front-end control software requires the following components to be installed on the host system:
- GPIO Expander Driver
  - Required for low-level hardware control of the RF front-end.
  - Installation instructions are provided in the gpio_expander folder.
- LIBIIO Library (v0.26)
  - Used for communication with RF hardware.
  - [Source](https://github.com/analogdevicesinc/libiio/tree/main) and [installation instructions](https://github.com/analogdevicesinc/libiio/blob/main/README_BUILD.md) are available in the official LIBIIO repository
- LIBGPIOD library (v 1.6.3)
  - Used for accessing GPIOs
  - Install it using te following command: sudo apt install libgpiod-dev gpiod
  - Documentation may be find [here](https://libgpiod.readthedocs.io/en/latest/index.html)

  The RFFE CLI software depends on both components to ensure correct functionality.

## Compilation

To compile the software do the following in the rffe folder:

```
- mkdir build
- cd build
- cmake ..
- make
```