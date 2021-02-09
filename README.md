# bCNC Pendant

(This Project is already well working but still "work in progress")

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

![bCNC Pendant](/Pictures/bCNC-pendant.jpg)



**What you need (see pictures in "Pictures" folder):**

- NodeMCU Esp8266 board
- 2.8" TFT display
- 3x4 key pad
- KY-040 Rotary Encoder (I shortened the knob)
- 3d-printed case, NodeMCU mount and spinner knob (see: "MISC" folder)
- Some screws (4pcs 2.2x6mm, 9pcs 2.2x16mm or similar)
- Labels for keypad ("MISC" folder)
- Micro USB cable
- Python 3
- you might have to install some python or arduino libraries (see .py and .ino files)



**Basic wireing:**

- First: solder out pins from all modules to save some space.

- Place rotary encoder into the top side of the case and fix by tightening input
- Place Buttons and TFT display in the top side of the Increase
- place NodeMCU mount behind the TFT display
- Screw NodeMCU onto that mount (2.2x6mm screws)

Solder the following connections using thin cables:

- NodeMCU D0   >   Rotary Encoder CLK
- NodeMCU SD3  >   Rotary Encoder SW
- NodeMCU D3   >   Rotary Encoder DT

- NodeMCU A0   >   Key Pad SIG

- NodeMCU D4   >   TFT DC
- NodeMCU D5   >   TFT SCK
- NodeMCU D6   >   TFT MISO
- NodeMCU D7   >   TFT MOSI
- NodeMCU D8   >   TFT CS
- NodeMCU 3.3V >   TFT LED
- NodeMCU 3.3V >   TFT RESET ?

- NodeMCU GND  >   all GND
- NodeMCU 3.3V >   all VCC


- Close case using the 2.2x12mm screws
- Put knob on rotary encoder




**How to use?**

- Plug in USB-pendant
- Start bCNC
- Start python script (bCNC DATA TRANSFER 01.py)
- Set your active axis by pressing one of  the "Set" buttons
- Jog in active axis by turning rotary knob left/right
- Set current Position as WPOS zero by pressing rotary knob
- Go to X0, Y0 or Z0 by pressing one of the "GoTo" buttons
- Increase/decrease joffing factor by pressing F+/F-
- Start, stop, home or reset/unlock by pressing the buttons (some have to be confirmed by pressing rotary knob)
