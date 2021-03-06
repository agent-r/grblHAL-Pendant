/////////////////////////////////////////
//
//   GRBLHAL PENDANT
//
/////////////////////////////////////////

/////////////////////////////////////////
//
//   TODO:  -> Multi-Char Numbers in Config (BT Address, BT Pin, hosts) (sprintf ??)
//          -> More
//          -> OTA-Uploads ?
//          -> Get BT running
//          -> Improve ReadMe !
//
/////////////////////////////////////////


// PINS
////////////////////////////////////////
// 36   ROT CLK : A
// 39   ROT DT  : B
// 33   KEY SIG
// 21   TFT DC
// 18   TFT SCK
// --   TFT MISO
// 23   TFT MOSI
// 22   TFT CS
// 17   TFT LED
// VCC  TFT RESET
// 34   BATTERY VOLTAGE DIVIDER


// LIBRARIES
#include <WiFi.h>                   // WIFI
#include <WiFiClient.h>             // WIFI
// #include "BluetoothSerialPatched.h"
// #include <BluetoothSerial.h>
#include <EEPROM.h>                 // EEPROM
#include <TickTwo.h>                // TICKER

//#include "AiEsp32RotaryEncoder.h"   // ROTARY ENCODER
#include <NewEncoder.h>

// #include "ESP32Encoder.h"
#include <ArduinoJson.h>            // JSON
#include <SPI.h>                    // TFT
#include <TFT_eSPI.h>               // TFT
#include <analogWrite.h>            // TFT BRIGHTNESS
#include "fonts.h"                  // FONTS
#include "driver/adc.h"             // FOR SLEEP !
#include <esp_wifi.h>               // FOR SLEEP !
// #include <esp_bt.h>                 // FOR SLEEP !

// #include <ArduinoOTA.h>             // for Over-The-Air programming

// Needed for Bluetooth SSID-Search. This is not supported in official current arduino/platformio releases
// comment out, if you use official releases.
// you have to set BT-MAC-Address manually then.
// #define USE_NEW_ARDUINO_ESP 1

///////////    DEBUG     ////////////////////////////////////
#define SERIAL_DEBUG 1          // General messages
#define SERIAL_DEBUG_IN 0       // Wifi+Blluetooth Inputs
#define SERIAL_DEBUG_OUT 1      // Wifi+Bluetooth Outputs

// WiFi & WiFi-AP
byte ConnectionMode = 0; // 0 = WIFI SLAVE    1 = WIFI AP    2 = BLUETOOTH SLAVE
WiFiClient TCPClient;

String WifiSSID;
String WifiPW;
IPAddress WifiHost(192,168,0,1);
int WifiPort = 8880;

String APSSID;
String APPW;
IPAddress APHost(192,168,0,1);
int APPort = 8880;

/*
   // BLUETOOTH
   // 00:21:13:01:3C:6F -> CNC
   // 00:21:13:01:2C:C9 -> TEST
   uint8_t BluetoothHost[6] = {0x00,0x21,0x13,0x01,0x3C,0x6F};
   const uint8_t BluetoothHostFix[6] = {0x00,0x21,0x13,0x01,0x2C,0xC9}; // 002113013C6F
   // const uint8_t BluetoothHostFix[6] = {0x00,0x21,0x13,0x01,0x3C,0x6F}; // 002113013C6F
   int BluetoothPin = 1234;
   BluetoothSerial btSerial;
 #if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
 #error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
 #endif
 */

//EEPROM ADDRESSES
#define EEConnectionMode 0  //          Byte (1)
#define EEWifiSSID 1        // bis 30   String (30)
#define EEWifiPW 31         // bis 60   String (30)
#define EEWifiHost 61       // bis 64   IP (4)
#define EEWifiPort 65       // bis 66   Int (2)
#define EEAPSSID 67         // bis 96   String (30)
#define EEAPPW 97           // bis 126  String (30)
#define EEAPHost 127        // bis 130  IP (4)
#define EEAPPort 131        // bis 132  Int (2)
// #define EEBluetoothHost 133 // bis 138  Adress (6)
// #define EEBluetoothPin 139  // bis 140  Int (2)
#define EEJogSpeed 141      // bis 148  Int (4x2=8) XYZA
#define EEProbeOffset 149   // bis 152  Float (4)
#define EEProbeDepth 153    // bis 154  Int (2)
#define EEProbeSpeed 155    // bis 156  Int (2)
#define EEProbeBackHeight 157 //        Byte (1)
#define EEProbeTime 158     //          Byte (1)
#define EESleepTime 159     //          Byte (1)
#define EEBrightness 160    //          Byte (1)


// ROTARY ENCODER
/*
 #define ROTARY_ENCODER_A_PIN GPIO_NUM_36
 #define ROTARY_ENCODER_B_PIN GPIO_NUM_39
 #define ROTARY_ENCODER_BUTTON_PIN -1    // -1 if no Button
 #define ROTARY_ENCODER_VCC_PIN -1       // -1 if Vcc -> 3,3V
 #define ROTARY_ENCODER_STEPS 4
   volatile int rot_clicks = 0;
   AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
   void IRAM_ATTR readEncoderISR() {
        rotaryEncoder.readEncoder_ISR();
   }
 */
void handleEncoder(void *pvParameters);
void ESP_ISR callBack(NewEncoder *encPtr, const volatile NewEncoder::EncoderState *state, void *uPtr);
QueueHandle_t encoderQueue;
// volatile int16_t prevEncoderValue;
volatile int16_t EncoderValue = 0;


// ESP32Encoder rotaryEncoder;
#define ENCODER_FPS 4              // Update Rate for Sending Code
void checkEncoder();
TickTwo EncoderTicker(checkEncoder, (1000 / ENCODER_FPS));


// KEYPAD
#define KEYPAD_PIN GPIO_NUM_33       // ONLY ADC1-PINS !!!
#define KEYPAD_DEBOUNCE 80       // Debounce for Buttons
void checkKeypad();
TickTwo KeypadTicker(checkKeypad, (KEYPAD_DEBOUNCE));
int keypad_old;
int keypad_current;


// BATTERY CHECK
#define BATTERY_PIN GPIO_NUM_34
#define BATTERY_FACTOR 0.001736
#define BATTERY_LOW 3.35
#define BATTERY_HIGH 4.2
#define BATTERYCHECK 60    // Seconds
// void checkBattery();
// TickTwo BatteryTicker(checkBattery, (1000 * BATTERYCHECK));


// SLEEP
byte SleepTime = 1;     // Minutes till Sleep
void TFTSleep();
TickTwo SleepTicker(TFTSleep, (60000 * SleepTime)); // 60000
#define bitSet64(value, bit) ((value) |= (1ULL << (bit)))
uint64_t SleepPinMask = 0;

// STATE UPDATES
#define STATE_FPS 6
void getState();
TickTwo StateTicker(getState, (1000 / STATE_FPS));

// TFT UPDATES
// TFT PINS ARE DEFINED IN TFT_ESPI -> USER_SETUP.h
// #define TFT_SCK    18
// #define TFT_MOSI   23
// #define TFT_DC 21        //
// #define TFT_CS 22       //
// #define TFT_RST 4       // TO VCC !!!
#define TFT_LED GPIO_NUM_17
TFT_eSPI tft = TFT_eSPI();
#define TFT_FPS 5         // was 10
void TFTUpdate();
TickTwo TftTicker(TFTUpdate, (1000 / TFT_FPS));

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
#define tftRotation  2
byte TFT_BRIGHTNESS;

#define BLINK_FPS 3
void TFTBlink();
TickTwo BlinkTicker(TFTBlink, (1000 / BLINK_FPS));
bool blinker = true;
bool blinker_change = true;

#define MESSAGE 3       // Seconds to show Messages
void TFTMessage();
TickTwo MessageTicker(TFTMessage, (1000 * MESSAGE));

#define HOLD_TIME 2     // SECONDS from the last State. Or HOLD !!!
void Hold();
TickTwo HoldTicker(Hold, (1000 * HOLD_TIME));
bool hold = true;       // wait till connection is REALLY there !

#define ALIVE_TIME 1    // Send "OK" !
void Alive();
TickTwo AliveTicker(Alive, (1000 * ALIVE_TIME));

//////////////////////////////////////////////////////////////////

uint16_t activeAxis = 0;
const String AxisName[4] = {"X", "Y", "Z", "A"};
const int AxisDir[4] = {1, 1, -1, 1};
int JogSpeed[4];
bool axischange = true;

float wx;
float wy;
float wz;
float wa;
float wxold;
float wyold;
float wzold;
float waold;
bool wxchange = true;
bool wychange = true;
bool wzchange = true;
bool wachange = true;

String state = "";
String stateold = "";
bool statechange = true;

byte activeFactor = 2;
const float factor[5] = {0.01, 0.1, 1, 10, 100};
const String strFactor[5] = {"0.01", "0.10", "1.00", "10.0", "100 "};
bool factorchange = true;

// WORKSPACE
int WorkSpace = 54;

// PROBE
float ProbeOffset;     // Height of probe-button (mm/100) = 27.80mm
int ProbeSpeed;       // Speed to go down
int ProbeDepth;       // Distance to go down
byte ProbeBackHeight;  // Distance zo go up after probing
byte ProbeTime;        // Maximum Seconds to Wait for Probe Success / Failure
bool Probe_Alarm;   // flag to check Alarm

///////////////////////////////////////////////////////////////////////////
///////////////////////      SETUP       //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void setup() {

        Serial.begin(115200);
        pinMode(KEYPAD_PIN, INPUT);
        pinMode(BATTERY_PIN, INPUT);
        pinMode(TFT_LED, OUTPUT);

        // ArduinoOTA.begin();

        // STARTING ENCODER INTERRUPTS:
        /*
           rotaryEncoder.begin();
           rotaryEncoder.setup(readEncoderISR);
           rotaryEncoder.disableAcceleration();
         */
        BaseType_t success = xTaskCreatePinnedToCore(handleEncoder, "Handle Encoder", 1900, NULL, 2, NULL, 1);
        if (!success) {
                printf("Failed to create handleEncoder task. Aborting.\n");
                while (1) {
                        yield();
                }
        }


        tft.begin();
        tft.setRotation(tftRotation);

        if (SERIAL_DEBUG) {Serial.println("------- EEPROM START --------");}
        EEPROM.begin(512);

        ConnectionMode = EEPROM.read(EEConnectionMode);
        if (ConnectionMode == 0) { if (SERIAL_DEBUG) { Serial.println("ConnectionMode: WIFI SLAVE"); }}
        else if (ConnectionMode == 1) { if (SERIAL_DEBUG) { Serial.println("ConnectionMode: WIFI AP"); } }
        else if (ConnectionMode == 2) { if (SERIAL_DEBUG) { Serial.println("ConnectionMode: BLUETOOTH SLAVE"); } }

        WifiSSID = EepromReadString(EEWifiSSID, 30);
        if (SERIAL_DEBUG) { Serial.println("Wifi SSID: " + WifiSSID); }
        WifiPW = EepromReadString(EEWifiPW, 30);
        if (SERIAL_DEBUG) { Serial.println("Wifi PASSWORD: " + WifiPW); }
        WifiHost = EepromReadIP(EEWifiHost);
        if (SERIAL_DEBUG) { Serial.println ("Wifi HOST: " + String(WifiHost[0]) + "." + String(WifiHost[1]) + "." + String(WifiHost[2]) + "." + String(WifiHost[3]));}
        WifiPort = EepromReadInt(EEWifiPort);
        if (SERIAL_DEBUG) { Serial.println("Wifi PORT: " + String(WifiPort)); }

        APSSID = EepromReadString(EEAPSSID, 30);
        if (SERIAL_DEBUG) { Serial.println("AP SSID: " + APSSID); }
        APPW = EepromReadString(EEAPPW, 30);
        if (SERIAL_DEBUG) { Serial.println("AP PASSWORD: " + APPW); }
        APHost = EepromReadIP(EEAPHost);
        if (SERIAL_DEBUG) { Serial.println ("AP HOST: " + String(APHost[0]) + "." + String(APHost[1]) + "." + String(APHost[2]) + "." + String(APHost[3]));}
        APPort = EepromReadInt(EEAPPort);
        if (SERIAL_DEBUG) { Serial.println("AP PORT: " + String(APPort)); }

        /*
           BluetoothHost[0] = EEPROM.read(EEBluetoothHost); BluetoothHost[1] = EEPROM.read(EEBluetoothHost+1); BluetoothHost[2] = EEPROM.read(EEBluetoothHost+2); BluetoothHost[3] = EEPROM.read(EEBluetoothHost+3); BluetoothHost[4] = EEPROM.read(EEBluetoothHost+4); BluetoothHost[5] = EEPROM.read(EEBluetoothHost+5);
           if (SERIAL_DEBUG) { Serial.println("Bluetooth Host Address: " + String(BluetoothHost[0], HEX) + ":" + String(BluetoothHost[1], HEX) + ":" + String(BluetoothHost[2], HEX) + ":" + String(BluetoothHost[3], HEX) + ":" + String(BluetoothHost[4], HEX) + ":" + String(BluetoothHost[5], HEX)); }
           BluetoothPin = EepromReadInt(EEBluetoothPin);
           if (SERIAL_DEBUG) { Serial.println("Bluetooth Pin: " + String(BluetoothPin)); }
         */

        JogSpeed[0] = EepromReadInt(EEJogSpeed);
        if (SERIAL_DEBUG) { Serial.println("X JOG SPEED: " + String(JogSpeed[0])); }
        JogSpeed[1] = EepromReadInt(EEJogSpeed+1);
        if (SERIAL_DEBUG) { Serial.println("Y JOG SPEED: " + String(JogSpeed[1])); }
        JogSpeed[2] = EepromReadInt(EEJogSpeed+2);
        if (SERIAL_DEBUG) { Serial.println("Z JOG SPEED: " + String(JogSpeed[2])); }
        JogSpeed[3] = EepromReadInt(EEJogSpeed+3);
        if (SERIAL_DEBUG) { Serial.println("A JOG SPEED: " + String(JogSpeed[3])); }

        ProbeOffset = EepromReadFloat(EEProbeOffset);
        if (SERIAL_DEBUG) { Serial.println("PROBE OFFSET: " + String(ProbeOffset)); }
        ProbeDepth = EepromReadInt(EEProbeDepth);
        if (ProbeDepth > 32767) { ProbeDepth = ProbeDepth - 65536; }
        if (SERIAL_DEBUG) { Serial.println("PROBE DEPTH: " + String(ProbeDepth)); }
        ProbeSpeed = EepromReadInt(EEProbeSpeed);
        if (SERIAL_DEBUG) { Serial.println("PROBE SPEED: " + String(ProbeSpeed)); }
        ProbeBackHeight = EEPROM.read(EEProbeBackHeight);
        if (SERIAL_DEBUG) { Serial.println("PROBE RISE: " + String(ProbeBackHeight)); }
        ProbeTime = EEPROM.read(EEProbeTime);
        if (SERIAL_DEBUG) { Serial.println("PROBE TIME: " + String(ProbeBackHeight)); }

        SleepTime = EEPROM.read(EESleepTime);
        SleepTicker.interval(SleepTime * 60000);
        if (SERIAL_DEBUG) { Serial.println("SLEEP TIME: " + String(SleepTime) + "min"); }

        TFT_BRIGHTNESS = EEPROM.read(EEBrightness);
        analogWrite(TFT_LED, TFT_BRIGHTNESS); // TURN LIGHT ON
        if (SERIAL_DEBUG) { Serial.println("BRIGHTNESS: " + String(TFT_BRIGHTNESS)); }

        if (SERIAL_DEBUG) {Serial.println("-------- EEPROM END ---------");}

        // STARTING INTERRUPTS !
        StateTicker.start();
        TftTicker.start();
        if (SleepTime > 0) { SleepTicker.start(); }
        KeypadTicker.start();
        EncoderTicker.start();
        AliveTicker.start();
        // BatteryTicker.start();
        // BlinkTicker.start();

        if (checkConfig()) { config(); } // Start config routine

        TFTPrepare();
        ConnectionSetup();
        Connect();

}

//////////////////////////////////////////////////////////////////



void loop() {
        // ArduinoOTA.handle();
        StateTicker.update();
        TftTicker.update();
        EncoderTicker.update();
        KeypadTicker.update();
        // BatteryTicker.update();
        // BlinkTicker.update();  // used only within config-loops
        MessageTicker.update();
        SleepTicker.update();
        HoldTicker.update();
        AliveTicker.update();
}
