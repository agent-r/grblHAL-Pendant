# grblHAL Pendant

(This project is work in progress)

I am using a WeMos Lolin32 to build a Pendant for grblHAL.

Machine state and position are passed from grblHAL via a plugin to a Esp01-Wifi-Module or a HC-05-Bluetooth-Module. The Pendant connects to one of these modules, receives this data and shows it on a tft display. 12 Buttons and a handwheel are there to send commands to the machine.

![bCNC Pendant](/Pictures/bCNC-pendant.jpg)

## Basic Functions:

- CNC pendant for up to 4 axis
- Rotary encoder and 12 buttons
- TFT display showing position and machine state
- Wireless connection (Bluetooth or WiFi)
- Battery powered
- Functions:
  - jogging (jandwheel)
  - goto Axis Zero
  - set Axis Zero
  - Probing Routine
  - Homing
  - Start
  - Stop/Hold
  - Unlock

  ![bCNC Pendant](/Pictures/bCNC-pendant.jpg)

## What you need:

- grblHAL breakout board + Firmware:
  - (https://github.com/grblHAL)
  - (https://github.com/phil-barrett/grblHAL-teensy-4.x)
  - (https://www.tindie.com/products/philba/grblhal-bob-unkit-for-teensy-41-t41u5xbb/)
- WiFi-Plugin for grblHAL
- Esp01 (as WiFi AccessPoint for grblHAL) + Firmware
- Pendant + Firmware
  - WeMos Lolin32
  - TFT Display
  - 12 Button KeyPad
  - Rotary encoder
  - LiPo Battery
  - PCB Board


## Bill of Materials (see pictures in "Pictures" folder):

- Esp01 Module
- WeMos Lolin32 Module
- 2.8" TFT display
- 3x4 key pad
- Rotary Encoder

- Toggle Switch MTS-203-A1 (or similar)

- LiPo Battery

- 3d-printed case, NodeMCU mount and spinner knob (see: "MISC" folder)
- Some screws (4pcs 2.2x6mm, 9pcs 2.2x16mm or similar)
- Labels for keypad ("MISC" folder)
- Micro USB cable for charging

- you might have to install some arduino libraries (see .ino files)


## Basic wireing:

**Preparation**

- First: solder out pins from all modules to save space.
- Place rotary encoder into the top side of the case and fix by tightening input
- Place Buttons and TFT display in the top side of the Increase
- place NodeMCU mount behind the TFT display
- Screw NodeMCU onto that mount (2.2x6mm screws)

**Soldering:**

Rotary encoder:
- NodeMCU D0   >   Rotary Encoder CLK
- NodeMCU SD3  >   Rotary Encoder SW
- NodeMCU D2   >   Rotary Encoder DT

KeyPad:
- NodeMCU A0   >   Key Pad SIG

TFT:
- NodeMCU D4   >   TFT DC
- NodeMCU D5   >   TFT SCK
- NodeMCU D6   >   TFT MISO
- NodeMCU D7   >   TFT MOSI
- NodeMCU D8   >   TFT CS
- NodeMCU D1   >   TFT LED
- NodeMCU 3.3V >   TFT RESET ?

VCC & GND:
- NodeMCU GND  >   all Modules GND
- NodeMCU 3.3V >   all Modules VCC
- NodeMCU VIN  >   Toggle Switch  >  9V Battery Connector (red)
- NodeMCU GND  >   Toggle Switch  >  9V Battery Connector (black)


**Finishing**

- Close case using the 2.2x12mm screws
- Put knob on rotary encoder




## Getting started!

- Start bCNC (make sure, web-pendant is running (http://localhost:8080))
- make sure, bCNC host computer is connected to a wifi network (with DHCP) (or set up an access point)
- Switch on pendant
- While starting, press rotary knob button to access config
- Set wifi SSID and password, set bCNC host computers IP
- You can also set values for probing and sleep time of the pendant

## How to use?

- Set your active axis by pressing one of  the "Set" buttons
- Jog in active axis by turning rotary knob left/right
- Set current Position as WPOS zero by pressing rotary knob
- Go to (active axis) zero by pressing the "GoTo" button
- Increase/decrease joffing factor by pressing F+/F-
- Start probing routine by pressing "probe" Button
- Start, stop, home or reset/unlock by pressing the buttons (some have to be confirmed by pressing rotary knob)
- Change config by pressing "Config" button
