# PA and LNA Enable Control

LimePSB RPCM v1.3 board features on board PAs and LNAs. Enable signals of these components may be controlled using this script.

## Prerequisites

It requires GPIO expander driver to be installed.

## Usage

To enable or disable particular PAs and LNAs execute the script and pass four parameters:
First parameter: 1 - enables TXA PA;  0 - disables TXA PA."
Second parameter: 1 - enables RXA LNA; 0 - disables RXA LNA."
Third parameter: 1 - enables TXB PA;  0 - disables TXB PA."
Fourth parameter: 1 - enables RXB LNA; 0 - disables RXB LNA."

To enalble all PAs and LNAs issue the following command:

```
pa_lna 1 1 1 1
```

To disable all PAs and LNAs issue the following command:

```
pa_lna 0 0 0 0
```

To enable TXA PA and RXA LNA only issue the following command:

```
pa_lna 1 1 0 0
```

