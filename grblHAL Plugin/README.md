# grblHAL pendant plugin

This plugin adds support for a HC-05 bluetooth module or a ESP-01 wifi module (or any other serial-connected board) to [grblHAL](https://github.com/grblHAL).

It works with Teensy4.1, I use it with [this breakout board](https://github.com/phil-barrett/grblHAL-teensy-4.x). Other boards might as well work.

------------------

## Connecting:

### to Teensy4.1:
- 5V > 3.3V (works!)
- GND > GND
- TX > Pin 0
- RX > Pin 1

### to the linked breakout board:
- 5V > 5V
- GND > GND
- TX > RX
- RX > TX

------------------

## Installation:

1. copy the /Pendant/ folder to your grblHAL folder.
2. Then add the following lines to grblHAL:

    - my_machine.h:
    ```
    #define PENDANT_ENABLE 1
    ```
    - grbl/plugins_init.h
    ```
    #if PENDANT_ENABLE
        extern void pendant_init (void);
        pendant_init();
    #endif
    ```
    
3. compile and upload to your controller
4. Should work!

---
2022-03-31
