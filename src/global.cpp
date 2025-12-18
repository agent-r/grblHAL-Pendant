#include <Arduino.h>
#include "global.h"


// ----- AXIS -----
uint8_t activeAxis = 0;
const String AxisName[4] = {"X", "Y", "Z", "A"};
const int8_t AxisDir[4] = {1, 1, -1, 1};
int JogSpeed[4] = {100, 100, 100, 100};
bool axischange = true;

// ----- POSITION -----
float wx, wy, wz, wa;
float wxold, wyold, wzold, waold;
bool wxchange, wychange, wzchange, wachange;

// ----- STATE -----
String state = "";
String stateold = "";
bool statechange = true;

// ----- FACTOR -----
byte activeFactor = 2;
const float factor[5] = {0.01, 0.1, 1, 10, 100};
const String strFactor[5] = {"0.01", "0.10", "1.00", "10.0", "100 "};
bool factorchange = true;

// ----- WORKSPACE -----
uint8_t WorkSpace = 54;

// ----- PROBE -----
float ProbeOffset = 10.0;
uint16_t ProbeSpeed = 100;
uint16_t ProbeDepth = 10;
uint8_t ProbeBackHeight = 10;
uint8_t ProbeTime = 10;
bool Probe_Alarm = false;

// ----- DISPLAY -----
uint8_t SleepTime = 1;
uint8_t TFT_BRIGHTNESS = 127;

// ----- BLUETOOTH -----
uint8_t BluetoothHost[6];
uint16_t BluetoothPin;
bool BLEconnected = false;

// ----- ENCODER -----
volatile int16_t encoderValue = 0;