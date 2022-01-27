
// PINS
////////////////////////////////////////
// 36   ROT CLK : A
// 39   ROT DT  : B
// 32   ROT SW  : N/A
// 33   KEY SIG
// 21   TFT DC
// 18   TFT SCK
// --   TFT MISO
// 23   TFT MOSI
// 22   TFT CS
// 17   TFT LED

static bool SERIAL_DEBUG = true;

// WiFi
#include <WiFi.h>
#include <WiFiClient.h>
WiFiClient TCPClient;

IPAddress APhost(192,168,0,1);
IPAddress host(192,168,0,1);
byte APOn = 1;
String password;
String ssid;
int port = 8880;

// CONFIG
bool configClick = false;

//EEPROM
#include <EEPROM.h>
//  0-3      host IP
//  4-53     SSID
//  54-103   PASSWORD
//  104      SLEEP TIME
//  105-106  PROBE OFFSET
//  107-108  PROBE DEPTH
//  109-110  PROBE SPEED
//  111      PROBE RETURN HEIGHT
//  112      AP ON
//  113      TFT BRIGHTNESS
//  114-115  PORT
//  116-123  Jog Speed

// ROTARY & BUTTON PINS
// #define ROTARY_PIN_CLK 36
// #define ROTARY_PIN_DT 39
#define BUTTON_PIN 32       // rotary encoder button
#define KEYPAD_PIN 33       // ONLY ADC1-PINS !!!

// ROTARY ENCODER
// #include <Rotary.h>
// Rotary rotary = Rotary(ROTARY_PIN_CLK, ROTARY_PIN_DT);
volatile int rot_clicks = 0;

#include "AiEsp32RotaryEncoder.h"
#include "Arduino.h"
#define ROTARY_ENCODER_A_PIN 36
#define ROTARY_ENCODER_B_PIN 39
#define ROTARY_ENCODER_BUTTON_PIN -1    // -1 if no Button
#define ROTARY_ENCODER_VCC_PIN -1       // -1 if Vcc -> 3,3V
#define ROTARY_ENCODER_STEPS 4
AiEsp32RotaryEncoder rotaryEncoder = AiEsp32RotaryEncoder(ROTARY_ENCODER_A_PIN, ROTARY_ENCODER_B_PIN, ROTARY_ENCODER_BUTTON_PIN, ROTARY_ENCODER_VCC_PIN, ROTARY_ENCODER_STEPS);
void IRAM_ATTR readEncoderISR() {
        rotaryEncoder.readEncoder_ISR();
}

// JSONs
#include <ArduinoJson.h>
char TCPBuffer[128];
String TCPBufferCopy = "";
bool buffer_active = false;
int buffer_idx = 0;
char buffer_char;
DynamicJsonDocument JsonIn(128);

// Ticker Interrupts
#include <TickTwo.h>
#define TFT_FPS 20
void TFTUpdate();
TickTwo TFTTicker(TFTUpdate, (1000 / TFT_FPS));
#define TFT_SLEEP 1               // minutes
void TFTSleep();
TickTwo TFTSleepTicker(TFTSleep, (60000 * TFT_SLEEP));
#define SEND_FPS 20              // Update Rate for Sending Code
void SendUpdate();
TickTwo SENDTicker(SendUpdate, (1000 / SEND_FPS));
#define DEBOUNCE_FPS 50              // Debounce for Buttons
void Read_Key_Btn();
TickTwo DEBOUNCETicker(Read_Key_Btn, (1000 / DEBOUNCE_FPS));

// TFT
#include <SPI.h>
#include <TFT_eSPI.h>
TFT_eSPI tft = TFT_eSPI();
#include <analogWrite.h>

// TFT PINS ARE DEFINED IN TFT_ESPI -> USER_SETUP.h
// #define TFT_SCK    18
// #define TFT_MOSI   23
// #define TFT_DC 21        //
// #define TFT_CS 22       //
#define TFT_LED 17      // DIMM via PWM
// #define TFT_RST 4       // TO VCC !!!

#include "fonts.h"
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

byte TFT_BRIGHTNESS = 127;

#define XField 0
#define YField 1
#define ZField 2
#define AField 3
#define FField 4
#define StateField 5
#define MessageField 6

#define tftRotation  0
int SleepCounter = 0;
byte SleepTime = 0;     // Minutes till Sleep

// KEYPAD, BUTTON
int keypad_last;
int keypad_current1;
int keypad_current2;
bool btn_last;
bool btn_current1;
bool btn_current2;

//////////////////////////////////////////////////////////////////

uint16_t activeAxis = 0;
const String AxisName[5] = {"", "X", "Y", "Z", "A"};

float wx = 1;
float wy = 1;
float wz = 1;
float wa = 1;
String state = "";
float wxold = 1;
float wyold = 1;
float wzold = 1;
float waold = 1;
String stateold = "";
bool wxchange = true;
bool wychange = true;
bool wzchange = true;
bool wachange = true;
bool statechange = true;
bool factorchange = true;
bool axischange = true;
byte activeFactor = 2;
const float factor[5] = {0.01, 0.1, 1, 10, 100};
String strFactor[5] = {"0.01", "0.10", "1.00", "10.0", "100 "};
int JogSpeed[5] = {0, 2000, 2000, 1000, 1000};
String LastSentCommand = "";
byte MessageCounter = 0;

// TODO: CHANGE THIS BEHAVIOUR !!!
#define MessageTime  6 * TFT_FPS    // seconds (How many Frames of TFT Update)


// PROBE
int ProbeOffset = 2780; // Height of probe-button (mm/100) = 27.80mm
int ProbeSpeed = 100;     // Speed to go down
int ProbeDepth = -30;     // Distance to go down
byte ProbeBackHeight = 10; // Distance zo go up after probing

///////////////////////////////////////////////////////////////////////////
///////////////////////      SETUP       //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

void setup() {

        Serial.begin(115200);

        // pinMode(ROTARY_PIN_CLK, INPUT_PULLUP);
        // pinMode(ROTARY_PIN_DT, INPUT_PULLUP);
        pinMode(BUTTON_PIN, INPUT_PULLUP);
        pinMode(KEYPAD_PIN, INPUT);
        pinMode(TFT_LED, OUTPUT);
        // digitalWrite(TFT_LED, HIGH);
        analogWrite(TFT_LED, TFT_BRIGHTNESS); // TURN LIGHT ON

        // STARTING ENCODER INTERRUPTS:
        // attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_CLK), rotate, CHANGE); // CHANGE
        //attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_DT), rotate, CHANGE);

        rotaryEncoder.begin();
        rotaryEncoder.setup(readEncoderISR);
        //set boundaries and if values should cycle or not
        //in this example we will set possible values between 0 and 1000;
        // bool circleValues = false;
        // rotaryEncoder.setBoundaries(0, 1000, circleValues); //minValue, maxValue, circleValues true|false (when max go to min and vice versa)
        rotaryEncoder.disableAcceleration(); //acceleration is now enabled by default - disable if you dont need it


        tft.begin();
        tft.setRotation(tftRotation);
        TFTConfigPrepare();

        TFTConfigPrint(0, "bCNC Pendant", TFT_COLOR_CNF_STD);
        TFTConfigPrint(1, "EEPROM Values:", TFT_COLOR_CNF_STD);

        if (SERIAL_DEBUG) {Serial.println("------- EEPROM START --------");}

        EEPROM.begin(512);
        host[0] = EEPROM.read(0); host[1] = EEPROM.read(1); host[2] = EEPROM.read(2); host[3] = EEPROM.read(3);
        TFTConfigPrint(2, "HOST:  " + String(host[0]) + "." + String(host[1]) + "." + String(host[2]) + "." + String(host[3]), TFT_COLOR_CNF_STD);
        if (SERIAL_DEBUG) { Serial.println ("HOST:  " + String(host[0]) + "." + String(host[1]) + "." + String(host[2]) + "." + String(host[3]));}

        for (int i = 4; i < 54; i++) { ssid = ssid + String(char(EEPROM.read(i))); }
        ssid.trim();
        TFTConfigPrint(3, "SSID:  " + ssid, TFT_COLOR_CNF_STD);
        if (SERIAL_DEBUG) { Serial.println("SSID: " + ssid); }

        for (int i = 54; i < 104; i++) { password = password + String(char(EEPROM.read(i))); }
        password.trim();
        TFTConfigPrint(4, "PW:    " + password, TFT_COLOR_CNF_STD);
        if (SERIAL_DEBUG) { Serial.println("PASSWORD: " + password); }

        SleepTime = EEPROM.read(104);
        TFTConfigPrint(5, "Sleep: " + String(SleepTime) + "min", TFT_COLOR_CNF_STD);
        if (SERIAL_DEBUG) { Serial.println("SLEEP: " + String(SleepTime) + "min"); }

        ProbeOffset = (EEPROM.read(105) << 8) + EEPROM.read(106);
        TFTConfigPrint(6, "Prb-Offset: " + String(ProbeOffset / 100.0), TFT_COLOR_CNF_STD);
        if (SERIAL_DEBUG) { Serial.println("PROBE OFFSET: " + String(ProbeOffset / 100.0)); }

        ProbeDepth = (EEPROM.read(107) << 8) + EEPROM.read(108);
        if (ProbeDepth > 32767) { ProbeDepth = ProbeDepth - 65536; }
        TFTConfigPrint(7, "Prb-Depth:  " + String(ProbeDepth), TFT_COLOR_CNF_STD);
        if (SERIAL_DEBUG) { Serial.println("PROBE DEPTH: " + String(ProbeDepth)); }

        ProbeSpeed = (EEPROM.read(109) << 8) + EEPROM.read(110);
        TFTConfigPrint(8, "Prb-Speed:  " + String(ProbeSpeed), TFT_COLOR_CNF_STD);
        if (SERIAL_DEBUG) { Serial.println("PROBE SPEED: " + String(ProbeSpeed)); }

        ProbeBackHeight = EEPROM.read(111);
        TFTConfigPrint(9, "Prb-Rise:   " + String(ProbeBackHeight), TFT_COLOR_CNF_STD);
        if (SERIAL_DEBUG) { Serial.println("PROBE RISE: " + String(ProbeBackHeight)); }

        APOn = EEPROM.read(112);
        if (APOn >= 1) { TFTConfigPrint(10, "AccessPoint:On", TFT_COLOR_CNF_STD); if (SERIAL_DEBUG) { Serial.println("ACCESS POINT: ON"); }}
        else { TFTConfigPrint(10, "AccessPoint:Off", TFT_COLOR_CNF_STD);  if (SERIAL_DEBUG) { Serial.println("ACCESS POINT: OFF"); }}

        TFT_BRIGHTNESS = EEPROM.read(113);
        if (SERIAL_DEBUG) { Serial.println("TFT BRIGHTNESS: " + String(TFT_BRIGHTNESS)); }

        port = (EEPROM.read(114) << 8) + EEPROM.read(115);
        if (SERIAL_DEBUG) { Serial.println("PORT: " + String(port)); }

        JogSpeed[1] = (EEPROM.read(116) << 8) + EEPROM.read(117);
        if (SERIAL_DEBUG) { Serial.println("X JOG SPEED: " + String(JogSpeed[1])); }
        JogSpeed[2] = (EEPROM.read(118) << 8) + EEPROM.read(119);
        if (SERIAL_DEBUG) { Serial.println("Y JOG SPEED: " + String(JogSpeed[2])); }
        JogSpeed[3] = (EEPROM.read(120) << 8) + EEPROM.read(121);
        if (SERIAL_DEBUG) { Serial.println("Z JOG SPEED: " + String(JogSpeed[3])); }
        JogSpeed[4] = (EEPROM.read(122) << 8) + EEPROM.read(123);
        if (SERIAL_DEBUG) { Serial.println("A JOG SPEED: " + String(JogSpeed[4])); }

        if (SERIAL_DEBUG) {Serial.println("-------- EEPROM END ---------");}

        // ASK FOR CONFIG START !
        TFTConfigPrint(11, "Config? Button!", TFT_COLOR_CNF_STD);
        delay(1500);
        if (digitalRead(BUTTON_PIN) == LOW) {
                while (digitalRead(BUTTON_PIN) == LOW) {delay(20);}
                config(); // Start config routine
        }

        TFTPrepare();
        WifiConnect();

        // STARTING INTERRUPTS !
        TFTTicker.start();
        DEBOUNCETicker.start();
        SENDTicker.start();
        if (SleepTime != 0) { TFTSleepTicker.start(); }

}

//////////////////////////////////////////////////////////////////

void loop() {
        TFTTicker.update();
        TFTSleepTicker.update();
        SENDTicker.update();
        DEBOUNCETicker.update();
}
