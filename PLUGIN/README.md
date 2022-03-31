## grblHAL pendant plugin

This plugin adds support for a HC-05 Bluetooth module or a ESP01-Wifi module to grblHAL

It works with Teensy4.1, I use it with THIS breakout board.

Connect:

- GND > GND
- 5V > 3.3V (works!)
- RX > TX
- TX > RX

Add the follwoing lines to grblHAL:

my_machine.h
------------------
#define PENDANT_ENABLE 1


grbl/plugins_init.h
-------------------
#if PENDANT_ENABLE
    extern void pendant_init (void);
    pendant_init();
#endif



---
2021-06-27
