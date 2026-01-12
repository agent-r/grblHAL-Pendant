#pragma once
#include <Arduino.h>


//////////////////////////////////////////// LOAD DEFAULTS //////////////////////////////

void defaultsLoad();

//////////////////////////////////////////// STATES /////////////////////////////////////////

// =======================
// System State
// =======================
enum SystemState
{
    STATE_STARTUP,
    STATE_RUN,
    STATE_CONFIRM_HOMING,
    STATE_CONFIRM_PROBING,
    STATE_PROBING,
    STATE_CONFIGMENU,
    STATE_SLEEP,
    STATE_ERROR
};

extern volatile SystemState systemState;


// =======================
// Queues & Handles
extern QueueHandle_t inputQueue;
extern QueueHandle_t timerQueue;
extern QueueHandle_t bleRxQueue;
extern QueueHandle_t bleTxQueue;
extern QueueHandle_t bleScanEventQueue;
extern QueueHandle_t bleCmdQueue;
extern QueueHandle_t displayQueue;
extern QueueHandle_t debugQueue;
// extern TaskHandle_t timerTaskHandle;

// =======================
// Task Priorities & STACK SIZES
#define DEBUG_TASK_PRIO 4
#define TIMER_TASK_PRIO 3
#define INPUT_TASK_PRIO 3
#define CONTROL_TASK_PRIO 2
#define DISPLAY_TASK_PRIO 2
#define BLE_TASK_PRIO 1

#define TASK_STACK_SIZE 4096
#define TASK_STACK_SIZE_TIMER 2048


// =======================
// Input (Buttons & Encoder)
// =======================

#define ENCODER_PIN_A 36
#define ENCODER_PIN_B 39

#define BUTTON_PIN_0 32    // AX-
#define BUTTON_PIN_1 33    // AX+
#define BUTTON_PIN_2 5     // SET0
#define BUTTON_PIN_3 25    // GOTO0
#define BUTTON_PIN_4 26    // PROBE
#define BUTTON_PIN_5 27    // CONFIG
#define BUTTON_PIN_6 14    // FEED-
#define BUTTON_PIN_7 13    // FEED+
#define BUTTON_PIN_8 16    // HOME
#define BUTTON_PIN_9 4     // STOP
#define BUTTON_PIN_10 2    // RESET
#define BUTTON_PIN_11 15   // ENTER

#define BUTTON_DEBOUNCE_MS 30
#define CONFIRM_TIMEOUT_MS 3000

#define INPUT_POLL_INTERVAL_MS 10
#define INPUT_POLL_MULTIPLIER_ENCODER 10

enum InputEventType
{
    BTN_AXIS_PREV,
    BTN_AXIS_NEXT,
    BTN_SET_ZERO,
    BTN_GOTO_ZERO,
    BTN_PROBE,
    BTN_MENU,
    BTN_FACTOR_DOWN,
    BTN_FACTOR_UP,
    BTN_HOME,
    BTN_STOP,
    BTN_UNLOCK,
    BTN_ENTER,
    ENC_MOVE
};

struct InputEvent
{
    InputEventType type;
    int32_t delta;
};


// =======================
// Display
// =======================

// TFT PINS ARE DEFINED IN PLATFORMIO.ini
// #define TFT_SCK    18
// #define TFT_MOSI   23
// #define TFT_DC     21       //
// #define TFT_CS     22       //
// #define TFT_RST     4       // TO VCC !!!

#define TFT_LED_PIN 17
#define TFT_ROTATION 2

#define TFT_FONT_SMALL TFT_FONT_11
#define TFT_FONT_LARGE TFT_FONT_18
#define TFT_COLOR_BGR 0xDEFB
#define TFT_COLOR_FRM_BGR 0xFFFF
#define TFT_COLOR_FRM_LIN 0x7BEF
#define TFT_COLOR_XYZ 0x0000
#define TFT_COLOR_XYZ_INACTIVE 0x94B2
#define TFT_COLOR_STA_NRM 0x6680
#define TFT_COLOR_STA_ERR 0xF80A
#define TFT_COLOR_MSG_NRM 0x8430
#define TFT_COLOR_MSG_ERR 0xF80A
#define TFT_COLOR_CNF_STD 0x0000
#define TFT_COLOR_CNF_HIL 0xF80A

enum DisplayFontSize {
    LARGE,
    SMALL
};

struct DisplayFormField {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    DisplayFontSize fontSize;
    uint16_t xTextOffset;
    uint16_t yTextOffset;
};

struct DisplayLine {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t color;
};

struct DisplayMenuForm {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
};

struct DisplayMenuField {
    uint16_t x;
    uint16_t y;
    uint16_t width;
    uint16_t height;
    DisplayFontSize fontSize;
    uint16_t xTextOffset;
    uint16_t yTextOffset;
};

/////////////////////////////////////////////////////////

#define DISP_MAX_MENU_LINES 11
#define DISP_MAX_MENU_TITLE_TEXT 12
#define DISP_MAX_MENU_TEXT 19
#define DISP_MAX_TEXT 12
#define DISP_MAX_TEXT_MESSAGE_STATE 19

#define MESSAGE_TIMEOUT_MS 2500

enum DisplayEventType
{
    DISP_PREPARE,
    DISP_UPDATE_STATUS,
    DISP_MENU_PREPARE,
    DISP_MENU_NAV_UPDATE,
    DISP_MENU_LINE_UPDATE,
    DISP_MENU_ADDRESS_UPDATE,
    DISP_MENU_EXIT,
    DISP_SLEEP
};

struct DisplayStateData
{
    char x[DISP_MAX_TEXT] = {">X     0.00"};
    char y[DISP_MAX_TEXT] = {" Y     0.00"};
    char z[DISP_MAX_TEXT] = {" Z     0.00"};
    char a[DISP_MAX_TEXT] = {" A     0.00"};
    char jogFactor[DISP_MAX_TEXT] = {" F     1.00"};
    char machineState[DISP_MAX_TEXT_MESSAGE_STATE] = {""};
    char message[DISP_MAX_TEXT_MESSAGE_STATE] = {""};
    bool xChanged = false;
    bool yChanged = false;
    bool zChanged = false;
    bool aChanged = false;
    bool axisChanged = false;
    bool jogFactorChanged = false;
    bool stateChanged = false;
    bool messageChanged = false;
    uint8_t activeAxisIndex = 0;
};

struct DisplayNavMenuData
{
    char title[DISP_MAX_MENU_TITLE_TEXT];
    uint8_t itemCount;
    uint8_t activeIndex;
    char items[DISP_MAX_MENU_LINES][DISP_MAX_MENU_TEXT];
};

struct DisplayLineMenuData
{
    char title[DISP_MAX_MENU_TITLE_TEXT];
    uint8_t line; 
    char printLine[DISP_MAX_MENU_TEXT]; 
};

struct DisplayAddressMenuData
{
    char left[DISP_MAX_MENU_TEXT];
    char mid[DISP_MAX_MENU_TEXT];
    char right[DISP_MAX_MENU_TEXT];
    char title[DISP_MAX_MENU_TITLE_TEXT];
};

struct DisplayEvent
{
    DisplayEventType type;
    DisplayStateData state;
    DisplayNavMenuData nav;
    DisplayLineMenuData line;
    DisplayAddressMenuData bleaddress;
};


// =======================
//          BLE
// =======================

#define BLE_SERVICE_UUID "825aeb6e-7e1d-4973-9c75-30c042c4770c"
#define BLE_TX_UUID "24259347-9d86-4c67-a9ae-84f6a7f0c90d" // PENDANT → TEENSY
#define BLE_RX_UUID "b52e05ac-8a8a-4880-85c7-bd3e6a32dc0e" // TEENSY → PENDANT

#define BLE_MTU 247
#define BLE_RX_MAX_LEN 128
#define BLE_TX_MAX_LEN 128

#define MAX_BLE_DEVICES 60
#define BLE_SCAN_DURATION 10000
#define BLE_SCAN_DURATION_SEC 10


enum BLEStatusType
{
    BLE_CONNECTED,
    BLE_DISCONNECTED,
    BLE_SCANNING,
    BLE_ERROR
};

struct BLERxEvent
{
    char json[BLE_RX_MAX_LEN + 1];
    uint8_t len;
};

struct BLETxEvent
{
    char json[BLE_TX_MAX_LEN + 1];
    uint8_t len;
};

//////////////////////////////////

enum BLEScanEventType
{
    BLE_SCAN_STARTED,
    BLE_SCAN_RESULT,
    BLE_SCAN_FINISHED
};

struct BLEScanResult
{
    char name[DISP_MAX_MENU_TEXT];
    uint8_t address[6];
};

struct BLEScanEvent
{
    BLEScanEventType type;
    BLEScanResult result;
};

///////////////////////////////////

enum BLECmdType
{
    BLE_SCAN_START,
    BLE_SCAN_STOP,
    BLE_DISCONNECT
};

struct BLECmd
{
    BLECmdType type;
};


// =======================
// Debug Events
// =======================
enum DebugLevel
{
    DBG_ERROR,
    DBG_WARN,
    DBG_INFO,
};

#define DEBUG_MSG_LEN 512

struct DebugEvent
{
    DebugLevel level;
    const char *source;
    char msg[DEBUG_MSG_LEN];
};


// =======================
// TIMER
// =======================

enum TimerID
{
    TIMER_CONFIRM_HOMING,
    TIMER_CONFIRM_PROBING,
    TIMER_PROBING,
    TIMER_MESSAGE_SHOW,
    TIMER_MESSAGE_BLINK,
    TIMER_SLEEP
};


// =======================
//      EEPROM
// =======================

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


// =======================
//      CONFIG OBJECT
// =======================

struct CONFIG
{
    uint8_t BluetoothHost[6];
    uint16_t BluetoothPin;

    float ProbeOffset;
    uint16_t ProbeDepth;
    uint16_t ProbeSpeed;
    uint8_t ProbeReturn;
    uint8_t ProbeTime;
    bool ProbeAlarm;
    
    uint8_t SleepTime;
    uint8_t TFTBrightness;

    uint8_t workspace;

};

extern CONFIG config;


// =======================
//      CNC OBJECT
// =======================

struct Axis
{
    float position;
    float positionOld;
    uint16_t jogSpeed;
    int8_t direction;
    char name;
};

struct CNC
{
    Axis axis[4];
    uint8_t activeAxisIndex;
    float jogFactor;
    char machineState[DISP_MAX_TEXT_MESSAGE_STATE];
    char machineStateOld[DISP_MAX_TEXT_MESSAGE_STATE];
};

extern CNC cnc;


///////////////////////////////////////////// DEBUG MAKRO ////////////////////////////////////////////////////

#define DEBUGF(lvl, src, fmt, ...)                        \
    do                                                    \
    {                                                     \
        char __buf[DEBUG_MSG_LEN];                        \
        snprintf(__buf, sizeof(__buf), fmt, __VA_ARGS__); \
        debugLog(lvl, src, __buf);                        \
    } while (0)