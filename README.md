# grblHAL Pendant

(This project is working but still a bit work in progress)

I am using a WeMos Lolin32 microcontroller board to build a pendant for [grblHAL](https://github.com/grblHAL).

Machine state and position are passed from grblHAL via a plugin to a HC-05-Bluetooth-Module (recommended) or a Esp01-Wifi-Module. The Pendant connects to one of these modules, receives this data and shows it on a tft display. 12 Buttons and a handwheel are there to send commands to the machine.

![bCNC Pendant](/Pictures/bCNC-pendant.jpg)

## Basic functions:

- CNC pendant for up to 4 axis
- Rotary encoder and 12 buttons
- TFT display showing position and machine state
- Wireless connection (bluetooth or WiFi)
- Battery powered, charged via usb
- Control functions:
  - jogging (handwheel)
  - goto axis zero
  - set axis zero
  - Probing routine
  - Homing
  - Start
  - Stop/Hold
  - Unlock

## Bill of materials:

for the electronic parts see also [pictures here](/Pictures/BoM).

- grblHAL controller, breakout board is optional but recommended:
  - (https://github.com/grblHAL)
  - (https://github.com/phil-barrett/grblHAL-teensy-4.x)
  - (https://www.tindie.com/products/philba/grblhal-bob-unkit-for-teensy-41-t41u5xbb/)
- [Plugin for grblHAL](/grblHAL Plugin)
- Bluetooth (recommended) or WiFi module for the grblHAL side
- Pendant
  - WeMos Lolin32
  - 2.8" TFT Display
  - 12 Button KeyPad
  - Rotary encoder
  - LiPo Battery (to fit the case, it should be ??x??x??mm max)
  - Toggle Switch
  - PCB Board
  - 3d printed or milled case
  - Some screws (4pcs 2.2x6mm, 9pcs 2.2x16mm or similar)
  - Labels for keypad ([MISC folder](/MISC))
  - Micro USB cable for charging
  - [Firmware](/src) and some [libraries](/lib)


## HowTo: The grblHAl controller side

To be able to connect the pendant to grblHAL (at least if you use a Teensy4.1), you have to add a Bluetooth or Wifi module to your controller.

I recommend using Bluetooth (for example aHC-05 module), as I noticed some connection losses with Wifi.

### Bluetooth:
The Module cannot be connected to the Teensy as it is. You have to program it first, by using AT-Commands. Instructions on how to do this can be found [all over the internet](https://www.google.com/search?q=how+to+program+a+hc+05+bluetooth+module+using+at+commands).

you sould:
- give your bluetooth module a proper name (for example "grblHAL")
- set the bluetooth module to "slave"
- set the bluetooth password to a 4-digit number
- set the bluetooth baudrate to "115200,0,0"
- read out the modules MAC address and write it down (you need it later)

### WiFi
The Wifi module cannot be connected to the Teensy as it is. You have to program it first.

- upload [this arduino application](/ESP-01 Webserver) to the module.
- Instructions on how to do this can be found [all over the internet](https://www.google.com/search?q=how+to+upload+a+arduino+sketch).

## How to: The pendant

- get all the parts from the Bill of materials
- **... to be continued ! ...**
- Close case using the 2.2x12mm screws

## How To: Getting started

- Start your grblHAL controller. A **Pendant** plugin should be named in your console window.
- make sure if your bluetooth or wifi network is available
- press **config** button and power cycle your pendant.
- navigate to **> Connection > Connection Mode** and select **Bluetooth** or **WiFi**
- navigate to **> Connection > Connection Settings**
    - if you are using wifi, set your SSID, Password, Host IP and port
    - if you are using bluetooth, set your Address and PIN (selecting slave by SSID does not work yet)
- check, if you want to change any other settings like jog speed, probing, sleep time, brightness, etc.
- leave the config menu and have fun!


## How to: Use

- Set your active axis by pressing one of the **Set** buttons
- Jog in active axis by turning **rotary knob** left/right
- Set current Position on active axis as WPOS zero by pressing **Set 0**
- Go to active axis zero by pressing **GoTo 0**
- Increase/decrease jogging distance by pressing **F+/F-**
- Start probing routine by pressing **Probe**
- **Start, stop, home or reset/unlock** by pressing the buttons (some have to be confirmed by pressing **Enter**)
- Change settings by pressing **Config**
