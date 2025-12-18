#pragma once
#include <Arduino.h>
// #include <TFT_eSPI.h>
#include "display/fonts.h"

//////////////////////////////
//                          //
//        DEFINITIONS       //
//                          //
//////////////////////////////

// DEBUG
#define SERIAL_DEBUG          // General messages
#define SERIAL_DEBUG_IN       // Bluetooth Inputs
#define SERIAL_DEBUG_OUT      // Bluetooth Outputs
// #define MESSAGE_DEBUG         // DEBUG via BLE / Wifi Message

//BLE
#define BLE_MAX_PACKET 254   // maximal erwartete Paketgröße

// ROTARY ENCODER
#define ENCODER_PIN_A 36
#define ENCODER_PIN_B 39
#define ENCODER_TICKER_TIME 150              // Update Rate for Sending Code // was 50

// EEPROM
#define EEPROM_LENGTH 512
#define EEBluetoothHost "BTH"
#define EEBluetoothPin "BTP"
#define EEJogSpeed "JSP"
#define EEProbeOffset "POF"
#define EEProbeDepth "PDE"
#define EEProbeSpeed "PSP"
#define EEProbeBackHeight "PBH"
#define EEProbeTime "PTI"
#define EESleepTime "STI"
#define EEBrightness "BRI"

// KEYPAD
#define BUTTON_0_PIN 32         // AX-        
#define BUTTON_1_PIN 33         // AX+         
#define BUTTON_2_PIN 5         // SET0
#define BUTTON_3_PIN 25         // GOTO0
#define BUTTON_4_PIN 26         // PROBE
#define BUTTON_5_PIN 27         // CONFIG
#define BUTTON_6_PIN 14         // FEED-
#define BUTTON_7_PIN 13         // FEED+
#define BUTTON_8_PIN 16         // HOME
#define BUTTON_9_PIN 4          // STOP      
#define BUTTON_10_PIN 2         // RESET
#define BUTTON_11_PIN 15        // ENTER
#define BUTTON_TICKER_TIME 40               // Debounce for Buttons // was 80
#define BUTTON_DEBOUNCE 10               // Debounce for Buttons // was 80

// TFT
/*
TFT PINS ARE DEFINED IN TFT_ESPI -> USER_SETUP.h
#define TFT_SCK    18
#define TFT_MOSI   23
#define TFT_DC     21       //
#define TFT_CS     22       //
#define TFT_RST     4       // TO VCC !!!
*/

#define TFT_FPS 5         // was 10
#define TFT_LED GPIO_NUM_17
#define TFT_ROTATION  2
#define TFT_MESSAGE_TIME 3       // Seconds to show Messages
#define BLINK_FPS 3

#define TFT_FONT_SMALL TFT_FONT_19
#define TFT_FONT_LARGE  TFT_FONT_28
#define TFT_COLOR_BGR 0xDEFB
#define TFT_COLOR_FRM_BGR 0xFFFF
#define TFT_COLOR_FRM_LIN 0x7BEF
#define TFT_COLOR_XYZ 0x0000
#define TFT_COLOR_XYZ_INACTIVE 0x94B2 // 0x8430
#define TFT_COLOR_STA_NRM 0x6680
#define TFT_COLOR_STA_ERR 0xF80A
#define TFT_COLOR_MSG_NRM 0x8430
#define TFT_COLOR_MSG_ERR 0xF80A
#define TFT_COLOR_CNF_STD 0x0000
#define TFT_COLOR_CNF_HIL 0xF80A
#define XField 0
#define YField 1
#define ZField 2
#define AField 3
#define FField 4
#define StateField 5
#define MessageField 6


// SLEEP
#define bitSet64(value, bit) ((value) |= (1ULL << (bit)))



//////////////////////////////
//                          //
//          VARIABLES       //
//                          //
//////////////////////////////

// ----- AXIS -----
extern uint8_t activeAxis;
extern const String AxisName[4];
extern const int8_t AxisDir[4];
extern int JogSpeed[4];
extern bool axischange;

// ----- POSITION -----
extern float wx, wy, wz, wa;
extern float wxold, wyold, wzold, waold;
extern bool wxchange, wychange, wzchange, wachange;

// ----- STATE -----
extern String state;
extern String stateold;
extern bool statechange;

// ----- FACTOR -----
extern byte activeFactor;
extern const float factor[5];
extern const String strFactor[5];
extern bool factorchange;

// ----- WORKSPACE -----
extern uint8_t WorkSpace;

// ----- PROBE -----
extern float ProbeOffset;
extern uint16_t ProbeSpeed;
extern uint16_t ProbeDepth;
extern uint8_t ProbeBackHeight;
extern uint8_t ProbeTime;
extern bool Probe_Alarm;

// ----- DISPLAY -----
// extern TFT_eSPI tft;
extern uint8_t TFT_BRIGHTNESS;
extern uint8_t SleepTime;

// ----- BLUETOOTH -----
extern uint8_t BluetoothHost[6];
extern uint16_t BluetoothPin;
extern bool BLEconnected;
extern QueueHandle_t bleQueue;


// ----- ENCODER -----
extern volatile int32_t encoderValue;