**bCNC Pendant**

I am using a NodeMCU (ESP8266) / Arduino to build a Pendant for bCNC [CAD/CAM/CNC-Controller](https://github.com/vlachoudis/bCNC)

Basic Functions:

- TFT display with machine positions and machine state.
- 12 Keys + rotary encoder for the following functions:
  - Set active axis (XYZ)
  - jogging in active axis
  - set WPos zero in active axis (rotary encoder switch)
  - GoTo (XYZ) zero
  - Increase / decrease jogging Factor
  - Home all axis
  - Start (run)
  - Stop
  - Reset/Unlock machine

A python script transfers machine state and position from the bCNC web server to the pendant (trough USB / Serial) and commands or gcode from pendant to bCNC.



**What you need (see pictures in "Pictures" folder):**

- NodeMCU Esp8266 Board
- 2.8" TFT
- 3x4 key pad
- KY-040 Rotary Encoder (shortened)
- 3d-printed case (see: "Case" folder)
- Some screws (4pcs 2.2x6mm, 9pcs 2.2x16mm or similar)
- Labels for keypad (.pdf)

- you might have to install some python or arduino libraries (see .py and .ino files)

**Basic wireing:**

- First: solder out all pins to save some space.

Solder the following connections:

NodeMCU D0   >   Rotary Encoder CLK
NodeMCU SD3  >   Rotary Encoder SW
NodeMCU D3   >   Rotary Encoder DT

NodeMCU A0   >   KeyPad SIG

NodeMCU D4   >   TFT DC
NodeMCU D5   >   TFT SCK
NodeMCU D6   >   TFT MISO
NodeMCU D7   >   TFT MOSI
NodeMCU D8   >   TFT CS
NodeMCU 3.3V >   TFT LED
NodeMCU 3.3V >   TFT RESET ?

NodeMCU GND  >   all GND
NodeMCU 3.3V >   all VCC








(This Project is stillworking but still "Work in Progress")
