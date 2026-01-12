
///////////////////////////////////////////////////////////////////////////
///////////////////////      CONFIG      //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include <vector>
#include <functional>
#include <ArduinoJson.h>

#include "controller/configmenu.h"
#include "display/display.h"
#include "global/global.h"
#include "global/eeprom.h"

enum MenuMode
{
    MENU_NAVIGATION,
    MENU_BLE_ADDRESS,
    MENU_VALUE_EDIT,
    MENU_INFO
};

static MenuMode menuMode = MENU_NAVIGATION;

static const MenuValueDesc *activeValueDesc = nullptr;
static float editValue = 0;

// Navigation Menus
struct MenuItem;
using MenuAction = void (*)();

struct MenuItem
{
    const char *title;
    MenuAction action;
    MenuItem *parent;
    MenuItem **children;
    uint8_t childCount;
};

MenuItem menuBack;

MenuItem menuBLEScanning;
MenuItem menuBLEScan;
MenuItem menuBLEHost;
MenuItem menuBLEPin;

MenuItem menuJoggingX;
MenuItem menuJoggingY;
MenuItem menuJoggingZ;
MenuItem menuJoggingA;

MenuItem menuProbeOffset;
MenuItem menuProbeDepth;
MenuItem menuProbeSpeed;
MenuItem menuProbeReturn;
MenuItem menuProbeTime;

// Submenus
MenuItem menuConnection;
MenuItem menuJogging;
MenuItem menuWorkspace;
MenuItem menuProbe;
MenuItem menuBrightness;
MenuItem menuSleep;
MenuItem menuInfo;
MenuItem menuExit;

// Root
MenuItem menuMain;

/////////////// BLE SCAN //////////////////

static BLEScanResult bleResults[MAX_BLE_DEVICES];
static MenuItem bleDeviceItems[MAX_BLE_DEVICES];
static MenuItem *childrenBLEScan[MAX_BLE_DEVICES + 1];
static uint8_t bleMenuCount = 0;

//////////////////////////////////////////////

MenuItem *childrenConnection[] = {
    &menuBLEScan,
    &menuBLEHost,
    &menuBLEPin,
    &menuBack};

MenuItem *childrenJogging[] = {
    &menuJoggingX,
    &menuJoggingY,
    &menuJoggingZ,
    &menuJoggingA,
    &menuBack};

MenuItem *childrenProbe[] = {
    &menuProbeOffset,
    &menuProbeDepth,
    &menuProbeSpeed,
    &menuProbeReturn,
    &menuProbeTime,
    &menuBack};

MenuItem *childrenMain[] = {
    &menuConnection,
    &menuJogging,
    &menuWorkspace,
    &menuProbe,
    &menuBrightness,
    &menuSleep,
    &menuInfo,
    &menuExit};

static MenuItem *currentMenu;
static int16_t activeIndex = 0;
static int16_t menuScrollOffset = 0;
static uint8_t BLEAdressActiveByte = 0;
static uint8_t bleAddressEdit[6];

void updateMenu(InputEvent &inEv)
{

    if (menuMode == MENU_VALUE_EDIT)
    {
        if (inEv.type == ENC_MOVE)
        {

            editValue += inEv.delta * activeValueDesc->step;

            if (editValue > activeValueDesc->maxValue)
                editValue = activeValueDesc->minValue;

            if (editValue < activeValueDesc->minValue)
                editValue = activeValueDesc->maxValue;

            if (activeValueDesc->onChange)
                activeValueDesc->onChange(editValue);

            menuSendValueDisplay();

        }

        if (inEv.type == BTN_ENTER)
        {

            writeValue(activeValueDesc, editValue);

            if (activeValueDesc->onExit)
                activeValueDesc->onExit();

            menuMode = MENU_NAVIGATION;
            activeValueDesc = nullptr;

            menuPrepareDisplay();
            menuSendDisplay();
        }

        return;
    }

    else if (menuMode == MENU_NAVIGATION)
    {
        if (inEv.type == ENC_MOVE)
        {
            int next = activeIndex + inEv.delta;
            if (next < 0)
                next = currentMenu->childCount - 1;
            if (next >= currentMenu->childCount)
                next = 0;
            activeIndex = next;
            menuSendDisplay();
        }

        if (inEv.type == BTN_ENTER)
        {
            MenuItem *sel = currentMenu->children[activeIndex];

            if (sel->childCount > 0)
            {
                currentMenu = sel;
                activeIndex = 0;
                menuPrepareDisplay();
                menuSendDisplay();
            }
            if (sel->action)
            {
                sel->action();
            }
        }
    }

    else if (menuMode == MENU_INFO)
    {
        if (inEv.type == ENC_MOVE)
        {
        }
        if (inEv.type == BTN_ENTER)
        {
            menuMode = MENU_NAVIGATION;
            actionBack();
        }
    }

    else if (menuMode == MENU_BLE_ADDRESS)
    {

        if (inEv.type == ENC_MOVE)
        {
            bleAddressEdit[BLEAdressActiveByte] += inEv.delta;
            menuSendBLEAddrDisplay();
        }

        if (inEv.type == BTN_ENTER)
        {
            BLEAdressActiveByte++;

            if (BLEAdressActiveByte >= 6)
            {
                memcpy(config.BluetoothHost, bleAddressEdit, 6);

                menuMode = MENU_NAVIGATION;
                menuPrepareDisplay();
                menuSendDisplay();
                return;
            }

            menuSendBLEAddrDisplay();
        }
    }
}

void initMenu()
{

    menuBack = {"Back", actionBack, nullptr, nullptr, 0};

    menuBLEScanning = {"Scanning ... ", nullptr, nullptr, 0};
    menuBLEScan = {"BLE Devices", actionBLEScan, nullptr, childrenBLEScan, 1};
    menuBLEHost = {"BLE Host", actionBLEAddress, nullptr, nullptr, 0};
    menuBLEPin = {"BLE Pin", actionBLEPin, nullptr, nullptr, 0};

    menuConnection = {"Connection", nullptr, nullptr, childrenConnection, sizeof(childrenConnection) / sizeof(childrenConnection[0])};

    menuJoggingX = {"Jogging X", actionJoggingX, nullptr, nullptr, 0};
    menuJoggingY = {"Jogging Y", actionJoggingY, nullptr, nullptr, 0};
    menuJoggingZ = {"Jogging Z", actionJoggingZ, nullptr, nullptr, 0};
    menuJoggingA = {"Jogging A", actionJoggingA, nullptr, nullptr, 0};

    menuJogging = {"Jogging", nullptr, nullptr, childrenJogging, sizeof(childrenJogging) / sizeof(childrenJogging[0])};

    menuWorkspace = {"Workspace", actionWorkspace, nullptr, nullptr, 0};

    menuProbeOffset = {"Probe Offset", actionProbeOffset, nullptr, nullptr, 0};
    menuProbeDepth = {"Probe Depth", actionProbeDepth, nullptr, nullptr, 0};
    menuProbeSpeed = {"Probe Speed", actionProbeSpeed, nullptr, nullptr, 0};
    menuProbeReturn = {"Probe Return", actionProbeReturn, nullptr, nullptr, 0};
    menuProbeTime = {"Probe Time", actionProbeTime, nullptr, nullptr, 0};

    menuProbe = {"Probe", nullptr, nullptr, childrenProbe, sizeof(childrenProbe) / sizeof(childrenProbe[0])};

    menuBrightness = {"Brightness", actionBrightness, nullptr, nullptr, 0};
    menuSleep = {"Sleep Mode", actionSleepTime, nullptr, nullptr, 0};
    menuInfo = {"Info", actionInfoDisplay, nullptr, nullptr, 0};
    menuExit = {"Exit", actionExit, nullptr, nullptr, 0};

    menuMain = {"Config", nullptr, nullptr, childrenMain, sizeof(childrenMain) / sizeof(childrenMain[0])};

    menuBLEScan.parent = &menuConnection;
    menuBLEHost.parent = &menuConnection;
    menuBLEPin.parent = &menuConnection;
    menuConnection.parent = &menuMain;

    menuJoggingX.parent = &menuJogging;
    menuJoggingY.parent = &menuJogging;
    menuJoggingZ.parent = &menuJogging;
    menuJoggingA.parent = &menuJogging;
    menuJogging.parent = &menuMain;

    menuProbeOffset.parent = &menuProbe;
    menuProbeDepth.parent = &menuProbe;
    menuProbeSpeed.parent = &menuProbe;
    menuProbeReturn.parent = &menuProbe;
    menuProbeTime.parent = &menuProbe;
    menuProbe.parent = &menuMain;

    menuBrightness.parent = &menuMain;
    menuSleep.parent = &menuMain;
    menuInfo.parent = &menuMain;
    menuExit.parent = &menuMain;

    menuBack.parent = nullptr;

    currentMenu = &menuMain;
    activeIndex = 0;

    DisplayEvent ev = {};
    ev.type = DISP_MENU_PREPARE;
    xQueueSend(displayQueue, &ev, 0);

    ev = {};
    ev.type = DISP_MENU_NAV_UPDATE;

    strncpy(ev.nav.title, currentMenu->title, DISP_MAX_MENU_TITLE_TEXT - 1);
    ev.nav.title[DISP_MAX_TEXT - 1] = '\0';

    ev.nav.itemCount =
        (currentMenu->childCount < DISP_MAX_MENU_LINES)
            ? currentMenu->childCount
            : DISP_MAX_MENU_LINES;

    if (activeIndex >= ev.nav.itemCount)
    {
        activeIndex = 0;
    }
    ev.nav.activeIndex = activeIndex;

    for (uint8_t i = 0; i < ev.nav.itemCount; i++)
    {
        if (currentMenu->children[i] && currentMenu->children[i]->title)
        {
            strncpy(ev.nav.items[i],
                    currentMenu->children[i]->title,
                    DISP_MAX_MENU_TEXT - 1);
        }
        else
        {
            strncpy(ev.nav.items[i], "<empty>", DISP_MAX_MENU_TEXT - 1);
        }
        ev.nav.items[i][DISP_MAX_MENU_TEXT - 1] = '\0';
    }

    xQueueSend(displayQueue, &ev, 0);

}

void menuPrepareDisplay()
{

    DisplayEvent ev = {};
    ev.type = DISP_MENU_PREPARE;
    xQueueSend(displayQueue, &ev, 0);
}

void menuSendDisplay()
{
    if (!currentMenu) return;

    DisplayEvent ev = {};
    ev.type = DISP_MENU_NAV_UPDATE;

    strncpy(ev.nav.title, currentMenu->title, DISP_MAX_MENU_TITLE_TEXT - 1);
    ev.nav.title[DISP_MAX_MENU_TITLE_TEXT - 1] = '\0';

    uint8_t visibleCount = (currentMenu->childCount < DISP_MAX_MENU_LINES) 
                            ? currentMenu->childCount 
                            : DISP_MAX_MENU_LINES;

    ev.nav.itemCount = visibleCount;

    if (activeIndex < menuScrollOffset) {
        menuScrollOffset = activeIndex;
    } else if (activeIndex >= menuScrollOffset + visibleCount) {
        menuScrollOffset = activeIndex - visibleCount + 1;
    }

    ev.nav.activeIndex = activeIndex - menuScrollOffset;

    for (uint8_t i = 0; i < visibleCount; i++) {
        uint8_t entryIndex = menuScrollOffset + i;

        if (entryIndex < currentMenu->childCount && currentMenu->children[entryIndex]) {
            const char *title = currentMenu->children[entryIndex]->title;
            if (title) {
                strncpy(ev.nav.items[i], title, DISP_MAX_MENU_TEXT - 1);
            } else {
                strncpy(ev.nav.items[i], "<empty>", DISP_MAX_MENU_TEXT - 1);
            }
        } else {
            strncpy(ev.nav.items[i], "<empty>", DISP_MAX_MENU_TEXT - 1);
        }

        ev.nav.items[i][DISP_MAX_MENU_TEXT - 1] = '\0';
    }

    xQueueSend(displayQueue, &ev, 0);
}


/////////// BASIC ACTIONS ////////////////////

void actionBack()
{
    if (currentMenu && currentMenu->parent)
    {
        currentMenu = currentMenu->parent;
        activeIndex = 0;
        menuScrollOffset = 0;
        menuPrepareDisplay();
        menuSendDisplay();
    }
}

void actionExit()
{

    eepromSave();
    systemState = STATE_RUN;

    DisplayEvent ev = {};
    ev.type = DISP_PREPARE;
    xQueueSend(displayQueue, &ev, 0);
}


/////////////// BLE SCAN //////////////////

void actionSelectBLEDevice()
{

    MenuItem *sel = currentMenu->children[activeIndex];
    if (!sel)
        return;

    memcpy(config.BluetoothHost, bleResults[activeIndex].address, 6);

    memset(bleResults, 0, sizeof(bleResults));
    memset(bleDeviceItems, 0, sizeof(bleDeviceItems));
    memset(childrenBLEScan, 0, sizeof(childrenBLEScan));

    activeIndex = 0;
    menuScrollOffset = 0;
    
    currentMenu = currentMenu->parent;

    menuPrepareDisplay();
    menuSendDisplay();

}

void handleBLEScanEvent(const BLEScanEvent &ev)
{

    switch (ev.type)
    {

    case BLE_SCAN_STARTED:

        break;

    case BLE_SCAN_RESULT:

        if (bleMenuCount < MAX_BLE_DEVICES)
        {

            bleResults[bleMenuCount] = ev.result;

            bleDeviceItems[bleMenuCount] = {bleResults[bleMenuCount].name, nullptr, &menuConnection, nullptr, 0};

            childrenBLEScan[bleMenuCount] = &bleDeviceItems[bleMenuCount];
            bleMenuCount++;

            childrenBLEScan[bleMenuCount] = &menuBLEScanning;

            menuBLEScan.children = childrenBLEScan;
            menuBLEScan.childCount = bleMenuCount + 1;

            menuPrepareDisplay();
            menuSendDisplay();

        }
        else
        {
            BLECmd cmd;
            cmd.type = BLE_SCAN_STOP;
            xQueueSend(bleCmdQueue, &cmd, 0);
        }
        break;

    case BLE_SCAN_FINISHED:

        childrenBLEScan[bleMenuCount] = &menuBack;

        for (int i = 0; i < bleMenuCount; i++) {
            childrenBLEScan[i]->action = actionSelectBLEDevice;
        }

        menuPrepareDisplay();
        menuSendDisplay();

        break;
    }
}

void actionBLEScan()
{

    activeIndex = 0;
    menuScrollOffset = 0;
    bleMenuCount = 0;

    memset(bleResults, 0, sizeof(bleResults));
    memset(bleDeviceItems, 0, sizeof(bleDeviceItems));
    memset(childrenBLEScan, 0, sizeof(childrenBLEScan));

    menuBLEScan.children = childrenBLEScan;
    menuBLEScan.childCount = bleMenuCount + 1;

    childrenBLEScan[bleMenuCount] = &menuBLEScanning;

    currentMenu = &menuBLEScan;

    menuPrepareDisplay();
    menuSendDisplay();

    BLECmd cmd;
    cmd.type = BLE_SCAN_START;
    xQueueSend(bleCmdQueue, &cmd, 0);

}


/////////// Direct Address Edit ///////////////

void actionBLEAddress()
{

    memcpy(bleAddressEdit, config.BluetoothHost, 6);
    BLEAdressActiveByte = 0;
    menuMode = MENU_BLE_ADDRESS;

    DisplayEvent ev = {};
    ev.type = DISP_MENU_PREPARE;
    xQueueSend(displayQueue, &ev, 0);

    menuSendBLEAddrDisplay();
}

void menuSendBLEAddrDisplay()
{

    DisplayEvent ev = {};
    ev.type = DISP_MENU_ADDRESS_UPDATE;

    char left[20] = "";
    char mid[4] = "";
    char right[20] = "";

    for (uint8_t i = 0; i < BLEAdressActiveByte; i++)
    {
        char tmp[4];
        sprintf(tmp, "%02X:", bleAddressEdit[i]);
        strcat(left, tmp);
    }

    sprintf(mid, "%02X", bleAddressEdit[BLEAdressActiveByte]);

    for (uint8_t i = BLEAdressActiveByte + 1; i < 6; i++)
    {
        char tmp[4];
        sprintf(tmp, ":%02X", bleAddressEdit[i]);
        strcat(right, tmp);
    }

    strncpy(ev.bleaddress.title, "BLE Host", DISP_MAX_MENU_TITLE_TEXT - 1);
    strncpy(ev.bleaddress.left, left, DISP_MAX_MENU_TEXT - 1);
    strncpy(ev.bleaddress.mid, mid, DISP_MAX_MENU_TEXT - 1);
    strncpy(ev.bleaddress.right, right, DISP_MAX_MENU_TEXT - 1);

    xQueueSend(displayQueue, &ev, 0);
}


///////// GENERIC VALUE MENU ///////////////

void menuSendValueDisplay()
{

    if (!activeValueDesc)
        return;

    DisplayEvent ev = {};
    ev.type = DISP_MENU_LINE_UPDATE;
    strncpy(ev.line.title, activeValueDesc->title, DISP_MAX_MENU_TITLE_TEXT - 1);
    ev.line.line = 2;

    char printValue[DISP_MAX_MENU_TEXT];
    sprintf(printValue, "%s%0*.*f %s", activeValueDesc->predescriptor, activeValueDesc->leadingZeros, activeValueDesc->decimals, editValue, activeValueDesc->unit);
    strncpy(ev.line.printLine, printValue, DISP_MAX_MENU_TEXT - 1);

    xQueueSend(displayQueue, &ev, 0);
}

float readValue(const MenuValueDesc *d)
{

    switch (d->type)
    {
    case VALUE_INT8:
        return *(int8_t *)d->valuePtr;
    case VALUE_UINT8:
        return *(uint8_t *)d->valuePtr;
    case VALUE_INT16:
        return *(int16_t *)d->valuePtr;
    case VALUE_UINT16:
        return *(uint16_t *)d->valuePtr;
    case VALUE_INT32:
        return *(int32_t *)d->valuePtr;
    case VALUE_FLOAT:
        return *(float *)d->valuePtr;
    }
    return 0;
}

void writeValue(const MenuValueDesc *d, float v)
{

    switch (d->type)
    {
    case VALUE_INT8:
        *(int8_t *)d->valuePtr = (int8_t)v;
        break;
    case VALUE_UINT8:
        *(uint8_t *)d->valuePtr = (uint8_t)v;
        break;
    case VALUE_INT16:
        *(int16_t *)d->valuePtr = (int16_t)v;
        break;
    case VALUE_UINT16:
        *(uint16_t *)d->valuePtr = (uint16_t)v;
        break;
    case VALUE_INT32:
        *(int32_t *)d->valuePtr = (int32_t)v;
        break;
    case VALUE_FLOAT:
        *(float *)d->valuePtr = v;
        break;
    }
}

void actionEditValue(const MenuValueDesc *desc)
{

    activeValueDesc = desc;
    editValue = readValue(desc);
    menuMode = MENU_VALUE_EDIT;

    DisplayEvent ev = {};
    ev.type = DISP_MENU_PREPARE;
    xQueueSend(displayQueue, &ev, 0);

    menuSendValueDisplay();
}


//////// Jogging ////////////////

MenuValueDesc valBLEPin = {
    .title = "Bluetooth Pin",
    .type = VALUE_UINT16,
    .valuePtr = &config.BluetoothPin,
    .minValue = 0,
    .maxValue = 9999,
    .step = 1,
    .decimals = 0,
    .predescriptor = "",
    .unit = "",
    .leadingZeros = 4,
    .onChange = nullptr,
    .onExit = nullptr
};

void actionBLEPin()
{
    actionEditValue(&valBLEPin);
}

MenuValueDesc valJoggingX = {
    .title = "Jog Speed X",
    .type = VALUE_UINT16,
    .valuePtr = &cnc.axis[0].jogSpeed,
    .minValue = 0,
    .maxValue = 10000,
    .step = 100,
    .decimals = 0,
    .predescriptor = "",
    .unit = "mm/min",
    .leadingZeros = 0,
    .onChange = nullptr,
    .onExit = nullptr
};

void actionJoggingX()
{
    actionEditValue(&valJoggingX);
}

MenuValueDesc valJoggingY = {
    .title = "Jog Speed Y",
    .type = VALUE_UINT16,
    .valuePtr = &cnc.axis[1].jogSpeed,
    .minValue = 0,
    .maxValue = 10000,
    .step = 100,
    .decimals = 0,
    .predescriptor = "",
    .unit = "mm/min",
    .leadingZeros = 0,
    .onChange = nullptr,
    .onExit = nullptr
};

void actionJoggingY()
{
    actionEditValue(&valJoggingY);
}

MenuValueDesc valJoggingZ = {
    .title = "Jog Speed Z",
    .type = VALUE_UINT16,
    .valuePtr = &cnc.axis[2].jogSpeed,
    .minValue = 0,
    .maxValue = 10000,
    .step = 100,
    .decimals = 0,
    .predescriptor = "",
    .unit = "mm/min",
    .leadingZeros = 0,
    .onChange = nullptr,
    .onExit = nullptr
};

void actionJoggingZ()
{
    actionEditValue(&valJoggingZ);
}

MenuValueDesc valJoggingA = {
    .title = "Jog Speed A",
    .type = VALUE_UINT16,
    .valuePtr = &cnc.axis[3].jogSpeed,
    .minValue = 0,
    .maxValue = 10000,
    .step = 100,
    .decimals = 0,
    .predescriptor = "",
    .unit = "mm/min",
    .leadingZeros = 0,
    .onChange = nullptr,
    .onExit = nullptr
};

void actionJoggingA()
{
    actionEditValue(&valJoggingA);
}


////////// Workspace //////////////

MenuValueDesc valWorkspace = {
    .title = "Workspace",
    .type = VALUE_UINT8,
    .valuePtr = &config.workspace,
    .minValue = 54,
    .maxValue = 59,
    .step = 1,
    .decimals = 0,
    .predescriptor = "G",
    .unit = "",
    .leadingZeros = 0,
    .onChange = nullptr,
    .onExit = setWorkspace
};

void actionWorkspace()
{
    actionEditValue(&valWorkspace);
}

void setWorkspace() {

    char wsStr[8];
    snprintf(wsStr, sizeof(wsStr), "G%02d", config.workspace);

    BLETxEvent ev = {};
  
    JsonDocument txJsonDoc;
    txJsonDoc["gcode"] = wsStr;
    ev.len = serializeJson(txJsonDoc, ev.json, sizeof(ev.json));
  
    xQueueSend(bleTxQueue, &ev, 0);
    
  }


//////// Probe ////////////////

MenuValueDesc valProbeOffset = {
    .title = "Probe Offset",
    .type = VALUE_FLOAT,
    .valuePtr = &config.ProbeOffset,
    .minValue = 0,
    .maxValue = 100,
    .step = 0.01,
    .decimals = 2,
    .predescriptor = "",
    .unit = "mm",
    .leadingZeros = 0,
    .onChange = nullptr,
    .onExit = nullptr
};

void actionProbeOffset()
{
    actionEditValue(&valProbeOffset);
}

MenuValueDesc valProbeDepth = {
    .title = "Probe Depth",
    .type = VALUE_UINT16,
    .valuePtr = &config.ProbeDepth,
    .minValue = 0,
    .maxValue = 50,
    .step = 1,
    .decimals = 0,
    .predescriptor = "",
    .unit = "mm",
    .leadingZeros = 0,
    .onChange = nullptr,
    .onExit = nullptr
};

void actionProbeDepth()
{
    actionEditValue(&valProbeDepth);
}

MenuValueDesc valProbeSpeed = {
    .title = "Probe Speed",
    .type = VALUE_UINT16,
    .valuePtr = &config.ProbeSpeed,
    .minValue = 0,
    .maxValue = 2000,
    .step = 100,
    .decimals = 0,
    .predescriptor = "",
    .unit = "mm/min",
    .leadingZeros = 0,
    .onChange = nullptr,
    .onExit = nullptr
};

void actionProbeSpeed()
{
    actionEditValue(&valProbeSpeed);
}

MenuValueDesc valProbeReturn = {
    .title = "Probe Return",
    .type = VALUE_UINT8,
    .valuePtr = &config.ProbeReturn,
    .minValue = 0,
    .maxValue = 50,
    .step = 1,
    .decimals = 0,
    .predescriptor = "",
    .unit = "mm",
    .leadingZeros = 0,
    .onChange = nullptr,
    .onExit = nullptr
};

void actionProbeReturn()
{
    actionEditValue(&valProbeReturn);
}

MenuValueDesc valProbeTime = {
    .title = "Probe Time",
    .type = VALUE_UINT8,
    .valuePtr = &config.ProbeTime,
    .minValue = 0,
    .maxValue = 10,
    .step = 1,
    .decimals = 0,
    .predescriptor = "",
    .unit = "sec",
    .leadingZeros = 0,
    .onChange = nullptr,
    .onExit = nullptr
};

void actionProbeTime()
{
    actionEditValue(&valProbeTime);
}


//////// Sleep ////////////////

MenuValueDesc valSleepTime = {
    .title = "Sleep Time",
    .type = VALUE_UINT8,
    .valuePtr = &config.SleepTime,
    .minValue = 0,
    .maxValue = 30,
    .step = 1,
    .decimals = 0,
    .predescriptor = "",
    .unit = "",
    .leadingZeros = 0,
    .onChange = nullptr,
    .onExit = nullptr
};

void actionSleepTime()
{
    actionEditValue(&valSleepTime);
}


//////// Brightness ////////////////

MenuValueDesc valBrightness = {
    .title = "Brightness",
    .type = VALUE_UINT8,
    .valuePtr = &config.TFTBrightness,
    .minValue = 0,
    .maxValue = 255,
    .step = 1,
    .decimals = 0,
    .predescriptor = "",
    .unit = "",
    .leadingZeros = 0,
    .onChange = setBrightness,
    .onExit = nullptr
};

void actionBrightness()
{
    actionEditValue(&valBrightness);
}

void setBrightness(int brightness) {
    analogWrite(TFT_LED_PIN, brightness);
}


///////////// INFO //////////////////////

void actionInfoDisplay()
{

    const char *infoText[DISP_MAX_MENU_LINES] = {
        "(c) 2022",
        "Paul Schwaderer",
        "github.com/agent-r",
        "/grblhal-pendant",
        "",
        "",
        "",
        "",
        "",
        "",
        ""
    };

    DisplayEvent ev = {};
    ev.type = DISP_MENU_LINE_UPDATE;
    strncpy(ev.line.title, "Info", DISP_MAX_MENU_TITLE_TEXT - 1);

    for (uint8_t i = 0; i < DISP_MAX_MENU_LINES; i++)
    {
        ev.line.line = i + 1;
        strncpy(ev.line.printLine, infoText[i], DISP_MAX_MENU_TEXT - 1);
        xQueueSend(displayQueue, &ev, 0);
    }
}
