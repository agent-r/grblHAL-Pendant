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
//          -> Improve ReadMe !
//
/////////////////////////////////////////

// PINS
////////////////////////////////////////
// 36   ROT CLK : A
// 39   ROT DT  : B
// 32   KEY AX-
// 33   KEY AX+
// 05   KEY SET0
// 25   KEY GOTO0
// 26   KEY PROBE
// 27   KEY CONFIG
// 14   KEY FEED-
// 13   KEY FEED+
// 16   KEY HOME
// 04   KEY STOP
// 02   KEY RESET
// 15   KEY ENTER
// 21   TFT DC
// 18   TFT SCK
// --   TFT MISO
// 23   TFT MOSI
// 22   TFT CS
// 17   TFT LED
// VCC  TFT RESET
// 34   BATTERY VOLTAGE DIVIDER // NOT USED!


#define BLE_ONLY               // disable all Wifi-Functionality
// TODO: #define WIFI_ONLY

// LIBRARIES
#ifndef BLE_ONLY
        #include <WiFi.h>                   // WIFI
        #include <WiFiClient.h>             // WIFI
        #include <esp_wifi.h>
#endif
// #include <ArduinoOTA.h>             // for Over-The-Air programming

// #include <Ardunio.h>

#include <BLEDevice.h>

#include <soc/soc.h>                    // TO DISABLE BROWNOUT DETECTOR) !!
#include <soc/rtc.h>                    // TO DISABLE BROWNOUT DETECTOR) !!

#include <EEPROM.h>                 // EEPROM
#include <TickTwo.h>                // TICKER

//#include "AiEsp32RotaryEncoder.h"   
#include <NewEncoder.h>             // ROTARY ENCODER
// #include "ESP32Encoder.h"

#include <JC_Button.h>              // BUTTON CHECKER

#include <ArduinoJson.h>            // JSON

#include <SPI.h>                    // TFT
#include <TFT_eSPI.h>               // TFT
// #include <analogWrite.h>         // TFT BRIGHTNESS
#include "fonts.h"                  // FONTS
#include "driver/adc.h"             // FOR SLEEP !

// DEBUG
#define SERIAL_DEBUG          // General messages
#define SERIAL_DEBUG_IN       // Wifi+Blluetooth Inputs
#define SERIAL_DEBUG_OUT      // Wifi+Bluetooth Outputs
#define MESSAGE_DEBUG         // DEBUG via BLE / Wifi Message

// WiFi & WiFi-AP
byte ConnectionMode = 0; // 0 = WIFI SLAVE    1 = WIFI AP    2 = BLUETOOTH SLAVE

#ifndef BLE_ONLY
        WiFiClient TCPClient;

        String WifiSSID;
        String WifiPW;
        IPAddress WifiHost(192,168,0,1);
        int WifiPort = 8880;

        String APSSID;
        String APPW;
        IPAddress APHost(192,168,0,1);
        int APPort = 8880;
#endif

// BLUETOOTH 

static BLEUUID SERVICE_UUID("825aeb6e-7e1d-4973-9c75-30c042c4770c");

static BLEUUID TX_CMD_CHARACTERISTIC_UUID("24259347-9d86-4c67-a9ae-84f6a7f0c90d");
static BLEUUID TX_GCODE_CHARACTERISTIC_UUID("604fb9a6-3cd8-48b3-a5c6-04e39f3aeccd");
static BLEUUID TX_MSG_CHARACTERISTIC_UUID("8a91b1dc-4574-41d4-a1d0-8d1d7488376a");

static BLEUUID RX_STATE_CHARACTERISTIC_UUID("b52e05ac-8a8a-4880-85c7-bd3e6a32dc0e");
static BLEUUID RX_WX_CHARACTERISTIC_UUID("8261af78-9f01-4525-80ee-30ab576de594");
static BLEUUID RX_WY_CHARACTERISTIC_UUID("47d8deb4-e094-481c-8cf5-a6ad5b20517c");
static BLEUUID RX_WZ_CHARACTERISTIC_UUID("210ea555-b93c-4d3b-8615-fd47a90b4526");
static BLEUUID RX_WA_CHARACTERISTIC_UUID("c3bf023c-45a9-49be-8f66-45600b31287a");

static BLERemoteCharacteristic* Tx_Cmd_Characteristic;
static BLERemoteCharacteristic* Tx_GCode_Characteristic;
static BLERemoteCharacteristic* Tx_Msg_Characteristic;

static BLERemoteCharacteristic* Rx_State_Characteristic;
static BLERemoteCharacteristic* Rx_Wx_Characteristic;
static BLERemoteCharacteristic* Rx_Wy_Characteristic;
static BLERemoteCharacteristic* Rx_Wz_Characteristic;
static BLERemoteCharacteristic* Rx_Wa_Characteristic;

uint8_t BluetoothHost[6];
uint16_t BluetoothPin;
static BLEClient* pClient;
static boolean BLEconnected = false;


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
#define EEBluetoothHost 133 // bis 138  Adress (6)
#define EEBluetoothPin 139  // bis 140  Int (2)
#define EEJogSpeed 141      // bis 148  Int (4x2=8) XYZA
#define EEProbeOffset 149   // bis 152  Float (4)
#define EEProbeDepth 153    // bis 154  Int (2)
#define EEProbeSpeed 155    // bis 156  Int (2)
#define EEProbeBackHeight 157 //        Byte (1)
#define EEProbeTime 158     //          Byte (1)
#define EESleepTime 159     //          Byte (1)
#define EEBrightness 160    //          Byte (1)


// ROTARY ENCODER
#define ENCODER_PIN_A 36
#define ENCODER_PIN_B 39

void handleEncoder(void *pvParameters);
void ESP_ISR callBack(NewEncoder *encPtr, const volatile NewEncoder::EncoderState *state, void *uPtr);
QueueHandle_t encoderQueue;
volatile int16_t EncoderValue = 0;

#define ENCODER_FPS 4              // Update Rate for Sending Code // was 5
void checkEncoder();
TickTwo EncoderTicker(checkEncoder, (1000 / ENCODER_FPS));


// KEYPAD
#define BUTTON_DEBOUNCE_TIME 30
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

Button Button0(BUTTON_0_PIN, BUTTON_DEBOUNCE_TIME);
Button Button1(BUTTON_1_PIN, BUTTON_DEBOUNCE_TIME);
Button Button2(BUTTON_2_PIN, BUTTON_DEBOUNCE_TIME);
Button Button3(BUTTON_3_PIN, BUTTON_DEBOUNCE_TIME);
Button Button4(BUTTON_4_PIN, BUTTON_DEBOUNCE_TIME);
Button Button5(BUTTON_5_PIN, BUTTON_DEBOUNCE_TIME);
Button Button6(BUTTON_6_PIN, BUTTON_DEBOUNCE_TIME);
Button Button7(BUTTON_7_PIN, BUTTON_DEBOUNCE_TIME); 
Button Button8(BUTTON_8_PIN, BUTTON_DEBOUNCE_TIME);
Button Button9(BUTTON_9_PIN, BUTTON_DEBOUNCE_TIME);
Button Button10(BUTTON_10_PIN, BUTTON_DEBOUNCE_TIME);
Button Button11(BUTTON_11_PIN, BUTTON_DEBOUNCE_TIME);


// #define KEYPAD_PIN GPIO_NUM_33       // ONLY ADC1-PINS !!!
#define KEYPAD_DEBOUNCE 10               // Debounce for Buttons // was 80
void checkKeypad();
TickTwo KeypadTicker(checkKeypad, (KEYPAD_DEBOUNCE));
uint8_t keypad_buffer[4] = {1,0,0,0}; // first int is active buffer number.
uint8_t keypad_old;
uint8_t keypad_current;


// BATTERY CHECK
// #define BATTERY_PIN GPIO_NUM_34
// #define BATTERY_FACTOR 0.001736
// #define BATTERY_LOW 3.35
// #define BATTERY_HIGH 4.2
// #define BATTERYCHECK 60    // Seconds
// void checkBattery();
// TickTwo BatteryTicker(checkBattery, (1000 * BATTERYCHECK));


// SLEEP
byte SleepTime = 1;     // Minutes till Sleep
void TFTSleep();
TickTwo SleepTicker(TFTSleep, (60000 * SleepTime)); // 60000
#define bitSet64(value, bit) ((value) |= (1ULL << (bit)))
uint64_t SleepPinMask = 0;

// STATE UPDATES
#ifndef BLE_ONLY
        #define STATE_FPS 4 // was 6
        void getState();
        TickTwo StateTicker(getState, (1000 / STATE_FPS));
#endif

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

#ifndef BLE_ONLY
        #define HOLD_TIME 2     // SECONDS from the last State. Or HOLD !!!
        void Hold();
        TickTwo HoldTicker(Hold, (1000 * HOLD_TIME));
        bool hold = true;       // wait till connection is REALLY there !

        #define ALIVE_TIME 1    // Send "OK" !
        void Alive();
        TickTwo AliveTicker(Alive, (1000 * ALIVE_TIME));
#endif

//////////////////////////////////////////////////////////////////

uint8_t activeAxis = 0;
const String AxisName[4] = {"X", "Y", "Z", "A"};
const int8_t AxisDir[4] = {1, 1, -1, 1};
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
uint8_t WorkSpace = 54;

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

        // DISABLE BROWNOUT DETECTOR
        WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

        #if defined(SERIAL_DEBUG) || defined(SERIAL_DEBUG_IN) || defined(SERIAL_DEBUG_OUT) 
        Serial.begin(115200);
        #endif

        // pinMode(KEYPAD_PIN, INPUT);
        Button0.begin();              // initialize the button object
        Button1.begin();              // initialize the button object
        Button2.begin();              // initialize the button object
        Button3.begin();              // initialize the button object
        Button4.begin();              // initialize the button object
        Button5.begin();              // initialize the button object
        Button6.begin();              // initialize the button object
        Button7.begin();              // initialize the button object
        Button8.begin();              // initialize the button object
        Button9.begin();              // initialize the button object
        Button10.begin();              // initialize the button object
        Button11.begin();              // initialize the button object

        // pinMode(BATTERY_PIN, INPUT);
        pinMode(TFT_LED, OUTPUT);

        // ArduinoOTA.begin();

        // STARTING ENCODER INTERRUPTS:
        BaseType_t success = xTaskCreatePinnedToCore(handleEncoder, "Handle Encoder", 1900, NULL, 2, NULL, 1);
        if (!success) {
                printf("Failed to create handleEncoder task. Aborting.\n");
                while (1) {
                        yield();
                }
        }

        // tft.begin();
        // tft.setRotation(tftRotation);

        EEPROM.begin(512);

#ifndef BLE_ONLY
        ConnectionMode = EEPROM.read(EEConnectionMode);

        WifiSSID = EepromReadString(EEWifiSSID, 30);
        WifiPW = EepromReadString(EEWifiPW, 30);
        WifiHost = EepromReadIP(EEWifiHost);
        WifiPort = EepromReadInt(EEWifiPort);

        APSSID = EepromReadString(EEAPSSID, 30);
        APPW = EepromReadString(EEAPPW, 30);
        APHost = EepromReadIP(EEAPHost);
        APPort = EepromReadInt(EEAPPort);
#endif

        BluetoothHost[0] = EEPROM.read(EEBluetoothHost); BluetoothHost[1] = EEPROM.read(EEBluetoothHost+1); BluetoothHost[2] = EEPROM.read(EEBluetoothHost+2); BluetoothHost[3] = EEPROM.read(EEBluetoothHost+3); BluetoothHost[4] = EEPROM.read(EEBluetoothHost+4); BluetoothHost[5] = EEPROM.read(EEBluetoothHost+5);
        BluetoothPin = EepromReadInt(EEBluetoothPin);

        JogSpeed[0] = EepromReadInt(EEJogSpeed);
        JogSpeed[1] = EepromReadInt(EEJogSpeed+2);
        JogSpeed[2] = EepromReadInt(EEJogSpeed+4);
        JogSpeed[3] = EepromReadInt(EEJogSpeed+6);

        ProbeOffset = EepromReadFloat(EEProbeOffset);
        ProbeDepth = EepromReadInt(EEProbeDepth);
        if (ProbeDepth > 32767) { ProbeDepth = ProbeDepth - 65536; }
        ProbeSpeed = EepromReadInt(EEProbeSpeed);
        ProbeBackHeight = EEPROM.read(EEProbeBackHeight);
        ProbeTime = EEPROM.read(EEProbeTime);

        SleepTime = EEPROM.read(EESleepTime);
        SleepTicker.interval(SleepTime * 60000);

        TFT_BRIGHTNESS = EEPROM.read(EEBrightness);


        ConnectionSetup();

        // start TFT after Connection setup. sometimes stays black otherwise...
        tft.begin();
        tft.setRotation(tftRotation);
        analogWrite(TFT_LED, TFT_BRIGHTNESS);
        TFTPrepare();

        if (checkConfig()) { config(); } // Start config routine

        Connect();

        // STARTING TICKERS !
#ifndef BLE_ONLY
        StateTicker.start();
#endif
        TftTicker.start();
        if (SleepTime > 0) { SleepTicker.start(); }
        KeypadTicker.start();
        EncoderTicker.start();
        // AliveTicker.start();
        // BatteryTicker.start();
        // BlinkTicker.start();

        pendantInit();
}

//////////////////////////////////////////////////////////////////


void loop() {
        // ArduinoOTA.handle();
#ifndef BLE_ONLY
        StateTicker.update();
#endif
        TftTicker.update();
        EncoderTicker.update();
        KeypadTicker.update();
        
        // BatteryTicker.update();
        // BlinkTicker.update();  // used only within config-loops
        MessageTicker.update();
        SleepTicker.update();
#ifndef BLE_ONLY
        HoldTicker.update();
        AliveTicker.update();
#endif
}


void pendantInit() {

        debug("[PENDANT] started & connected");

        #ifdef SERIAL_DEBUG 

                Serial.println("[PENDANT] ------- EEPROM DATA ---------");
                if (ConnectionMode == 0) { Serial.println("[PENDANT] ConnectionMode: WIFI SLAVE"); }
                else if (ConnectionMode == 1) { Serial.println("[PENDANT] ConnectionMode: WIFI AP"); }
                else if (ConnectionMode == 2) { Serial.println("[PENDANT] ConnectionMode: BLUETOOTH SLAVE"); }

                // Serial.println("[PENDANT] Wifi SSID: " + WifiSSID);
                // Serial.println("[PENDANT] Wifi PASSWORD: " + WifiPW);
                // Serial.println("[PENDANT] Wifi HOST: " + String(WifiHost[0]) + "." + String(WifiHost[1]) + "." + String(WifiHost[2]) + "." + String(WifiHost[3]));
                // Serial.println("[PENDANT] Wifi PORT: " + String(WifiPort));

                // Serial.println("[PENDANT] AP SSID: " + APSSID);
                // Serial.println("[PENDANT] AP PASSWORD: " + APPW);
                // Serial.println("[PENDANT] AP HOST: " + String(APHost[0]) + "." + String(APHost[1]) + "." + String(APHost[2]) + "." + String(APHost[3]));
                // Serial.println("[PENDANT] AP PORT: " + String(APPort));

                // Serial.println("[PENDANT] BLE HOST: " + String(BluetoothHost[0], HEX) + ":" + String(BluetoothHost[1], HEX) + ":" + String(BluetoothHost[2], HEX) + ":" + String(BluetoothHost[3], HEX) + ":" + String(BluetoothHost[4], HEX) + ":" + String(BluetoothHost[5], HEX));
                // Serial.println("[PENDANT] BLE PIN: " + String(BluetoothPin));

                Serial.println("[PENDANT] X JOG SPEED: " + String(JogSpeed[0]));
                Serial.println("[PENDANT] Y JOG SPEED: " + String(JogSpeed[1]));
                Serial.println("[PENDANT] Z JOG SPEED: " + String(JogSpeed[2]));
                Serial.println("[PENDANT] A JOG SPEED: " + String(JogSpeed[3]));

                Serial.println("[PENDANT] PROBE OFFSET: " + String(ProbeOffset));
                Serial.println("[PENDANT] PROBE DEPTH: " + String(ProbeDepth));
                Serial.println("[PENDANT] PROBE SPEED: " + String(ProbeSpeed));
                Serial.println("[PENDANT] PROBE RISE: " + String(ProbeBackHeight));
                Serial.println("[PENDANT] PROBE TIME: " + String(ProbeTime));

                Serial.println("[PENDANT] SLEEP TIME: " + String(SleepTime) + "min");
                Serial.println("[PENDANT] BRIGHTNESS: " + String(TFT_BRIGHTNESS));

                Serial.println("[PENDANT] -------- EEPROM END ---------");

        #endif
}