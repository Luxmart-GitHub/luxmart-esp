# Luxmart ESP WiFi module firmware

This is a firmware for ESP 01S module, which creates a wireless end point for the Luxmart gadget.

## Building

1. First, use Arduino IDE to download and install the following packages:

```
AsyncTCP
ESPAsyncTCP
ESPAsyncWebSrv
```

2. Build using make:

```
make -f makeEspArduino/makeEspArduino.mk
```

