
// PINS
////////////////////////////////////////
// D2   ROT CLK    = GPIO 4
// A0   KEY SIG
// D3   ROT DT     = GPIO 0
// D4   TFT DC
// D5   TFT SCK
// D6   TFT MISO
// D7   TFT MOSI
// D8   TFT CS
// D1   TFT LED
// SD3  ROT SW     = GPIO 10

// WiFi
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#define httpTimeout 100 // was 50

String host;
String password;
String ssid;
String httpPort = "8080";
String stateUrl = "/state";
String statePayload = "";
String cmdUrl = "/send?cmd=";
String gCodeUrl = "/send?gcode=";

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

// PINS
#define ROTARY_PIN_CLK 4
#define ROTARY_PIN_DT 0
#define BUTTON_PIN 10       // rotary encoder button
#define KEYPAD_PIN A0

// ROTARY ENCODER
#include <Rotary.h>
Rotary rotary = Rotary(ROTARY_PIN_CLK, ROTARY_PIN_DT);
volatile int rot_clicks = 0;

// JSONs
#include <ArduinoJson.h>
StaticJsonDocument<1028> JsonIn; //?

// Ticker Interrupts
#include <TickTwo.h>
#define TFT_FPS 4
void TFTUpdate();
TickTwo TFTTicker(TFTUpdate, (1000 / TFT_FPS));
#define TFT_SLEEP 1               // minutes
void TFTSleep();
TickTwo TFTSleepTicker(TFTSleep, (60000 * TFT_SLEEP));
#define SEND_FPS 10              // Update Rate for Sending Code
void SendUpdate();
TickTwo SENDTicker(SendUpdate, (1000 / SEND_FPS));
#define DEBOUNCE_FPS 50              // Debounce for Buttons
void Read_Key_Btn();
TickTwo DEBOUNCETicker(Read_Key_Btn, (1000 / DEBOUNCE_FPS));

// TFT
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_I2CDevice.h>
#define TFT_DC D4
#define TFT_CS D8
#define TFT_LED D1  // DIMM via PWM

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
#define FField 3
#define StateField 4
#define MessageField 5

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);
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
const String AxisName[4] = {"", "X", "Y", "Z"};

float wx = 1;
float wy = 1;
float wz = 1;
String state = "";
float wxold = 1;
float wyold = 1;
float wzold = 1;
String stateold = "";
bool wxchange = true;
bool wychange = true;
bool wzchange = true;
bool statechange = true;
bool factorchange = true;
bool axischange = true;
byte activeFactor = 2;
const float factor[5] = {0.01, 0.1, 1, 10, 100};
String strFactor[5] = {"0.01", "0.10", "1.00", "10.0", "100 "};
String LastSentCommand = "";
// byte StatusCounter = 0;
byte MessageCounter = 0;
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

        pinMode(ROTARY_PIN_CLK, INPUT_PULLUP);
        pinMode(ROTARY_PIN_DT, INPUT_PULLUP);
        pinMode(BUTTON_PIN, INPUT_PULLUP);
        pinMode(KEYPAD_PIN, INPUT);
        pinMode(TFT_LED, OUTPUT);
        analogWrite(TFT_LED, 255); // TURN LIGHT ON

        // STARTING INTERRUPTS:
        attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_CLK), rotate, CHANGE); // CHANGE
        attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_DT), rotate, CHANGE);

        tft.begin();
        tft.setRotation(tftRotation);

        TFTConfigPrepare();
        TFTConfigPrint(0, "bCNC Pendant", TFT_COLOR_CNF_STD);
        TFTConfigPrint(1, "EEPROM Values:", TFT_COLOR_CNF_STD);

        EEPROM.begin(512);
        host = String(EEPROM.read(0)) + "." + String(EEPROM.read(1)) + "." + String(EEPROM.read(2)) + "." + String(EEPROM.read(3));
        TFTConfigPrint(2, "HOST:  " + host, TFT_COLOR_CNF_STD);

        for (int i = 4; i < 54; i++) { ssid = ssid + String(char(EEPROM.read(i))); }
        ssid.trim();
        TFTConfigPrint(3, "SSID:  " + ssid, TFT_COLOR_CNF_STD);

        for (int i = 54; i < 104; i++) { password = password + String(char(EEPROM.read(i))); }
        password.trim();
        TFTConfigPrint(4, "PW:    " + password, TFT_COLOR_CNF_STD);

        SleepTime = EEPROM.read(104);
        TFTConfigPrint(5, "Sleep: " + String(SleepTime) + "min", TFT_COLOR_CNF_STD);

        ProbeOffset = (EEPROM.read(105) << 8) + EEPROM.read(106);
        TFTConfigPrint(6, "Prb-Offset: " + String(ProbeOffset / 100.0), TFT_COLOR_CNF_STD);

        ProbeDepth = (EEPROM.read(107) << 8) + EEPROM.read(108);
        if (ProbeDepth > 32767) { ProbeDepth = ProbeDepth - 65536; }
        TFTConfigPrint(7, "Prb-Depth:  " + String(ProbeDepth), TFT_COLOR_CNF_STD);

        ProbeSpeed = (EEPROM.read(109) << 8) + EEPROM.read(110);
        TFTConfigPrint(8, "Prb-Speed:  " + String(ProbeSpeed), TFT_COLOR_CNF_STD);

        ProbeBackHeight = EEPROM.read(111);
        TFTConfigPrint(9, "Prb-Rise:   " + String(ProbeBackHeight), TFT_COLOR_CNF_STD);


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
        SENDTicker.start();
        DEBOUNCETicker.start();
        if (SleepTime != 0) { TFTSleepTicker.start(); }

}

//////////////////////////////////////////////////////////////////

void loop() {
        TFTTicker.update();
        TFTSleepTicker.update();
        SENDTicker.update();
        DEBOUNCETicker.update();
}
