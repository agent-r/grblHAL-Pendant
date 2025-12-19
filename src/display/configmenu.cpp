
///////////////////////////////////////////////////////////////////////////
///////////////////////      CONFIG      //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

#include <Arduino.h>
#include "display/configfunct.h"
#include "display/configmenu.h"
#include "display/display.h"
#include "global.h"
#include "communication/bluetooth.h"
#include "controls/controls.h"
#include "communication/debug.h"
#include "config.h"

const int ConfigForms[2][5] = {
        {10, 15, 220, 35, 3},  // Field Heading
        {10, 60, 220, 250, 2}  // Field Content
};

const int ConfigFields[12][5] = {
        {10, 17, 220, 33, 3},  // HEADING
        {10, 62, 220, 22, 2},  // CONTENT
        {10, 84, 220, 22, 2},
        {10, 106, 220, 22, 2},
        {10, 128, 220, 22, 2},
        {10, 150, 220, 22, 2},
        {10, 172, 220, 22, 2},
        {10, 194, 220, 22, 2},
        {10, 216, 220, 22, 2},
        {10, 238, 220, 22, 2},
        {10, 260, 220, 22, 2},
        {10, 282, 220, 22, 2},
};
 

void config() {

        // PREPARE CONFIG MENU STATE
        EncoderTicker.stop(); // DO NOT SEND ENCODER MOVEMENTS ANY MORE!

        const byte ContentNum = 10;
        const String Content[ContentNum] = {
                "Config",
                "Connection",
                "Jogging",
                "Workspace",
                "Probe",
                "Sleep Mode",
                "Brightness",
                "Battery",
                "Info",
                //        "Firmware Update",
                "Exit"
        };
        while(true) {
                switch (TFTConfigMenu(Content, ContentNum)) {
                case 1: configConnection(); break;
                case 2: configJogging(); break;
                case 3: configWorkspace(); break;
                case 4: configProbe(); break;
                case 5: configSleep(); break;
                case 6: configBrightness(); break;
                case 7: configBattery(); break;
                case 8: configInfo(); break;
                case 9: configSave(); TFTPrepare(); EncoderTicker.start(); return; // CLEAR TFT, restart Encoder, exit Menu.
                }
        }
}

void configConnection() {

        const byte ContentNum = 4;
        const String Content[ContentNum] = {
                "Connection",
                "Bluetooth only",
                "Connection Settings",
                "Back"
        };
        while(true) {
                switch (TFTConfigMenu(Content, ContentNum)) {
                case 1: break;
                case 2: configConnectionSettings(); break;
                case 3: return;
                }
        }
}



void configConnectionSettings() {

        const byte ContentNumBluetooth = 4;
        const String ContentBluetooth[ContentNumBluetooth] = {
                "Bluetooth",
                "Bluetooth SSID",
                "Bluetooth Host",
                // "Bluetooth Pin",
                "Back"
        };

                   while(true) {
                        switch (TFTConfigMenu(ContentBluetooth, ContentNumBluetooth)) {
                                case 1: configBLE(); break;
                                case 2: configBluetoothAddress(); break;
                                // case 3: configBluetoothPin(); break;
                                case 3: return;
                        }
                   }
}





void configBluetoothAddress() {

        // for (int i = 0; i < 6; i++) {
        //         BluetoothHost[i] = EEPROM.read(EEBluetoothHost + i);
        // }

        byte activeByte = 0;

        char strAddress[20];
        TFTConfigPrepare();
        TFTConfigPrint(0, "Bluetooth Addr", TFT_COLOR_CNF_STD);
        TFTSetFontSize(2);
        tft.setCursor(ConfigFields[2][0]+7, ConfigFields[2][1]+4);

        for (int i = 0; i < activeByte; i++) {
                sprintf(strAddress,"%02x:", BluetoothHost[i]);
                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                tft.print(strAddress);
        }
        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
        sprintf(strAddress,"%02x", BluetoothHost[activeByte]);
        tft.print(strAddress);
        for (int i = (activeByte + 1); i < 6; i++) {
                sprintf(strAddress,":%02x", BluetoothHost[i]);
                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                tft.print(strAddress);
        }

        while (true) {
                delay(ENCODER_TICKER_TIME);
                if (checkEnter()) {
                        activeByte++;

                        if (activeByte >= 6) {
                                copyArray(BluetoothHost, configDoc[EEBluetoothHost]);
                                // for (int i = 0; i < 6; i++) { EEPROM.write(EEBluetoothHost + i, BluetoothHost[i]); }
                                // EEPROM.commit();
                                sprintf(strAddress,"%02x:%02x:%02x:%02x:%02x:%02x",BluetoothHost[0],BluetoothHost[1],BluetoothHost[2],BluetoothHost[3],BluetoothHost[4],BluetoothHost[5]);
                                // debugf(DEBUG_FLAG_BOTH, "BLUETOOTH HOST: %s", strAddress);
                                return;
                        }

                        tft.fillRect(ConfigFields[2][0], ConfigFields[2][1], ConfigFields[2][2], ConfigFields[2][3], TFT_COLOR_FRM_BGR);
                        tft.setCursor(ConfigFields[2][0] + 7, ConfigFields[2][1] + 4);
                        for (int i = 0; i < activeByte; i++) {
                                sprintf(strAddress,"%02x:", BluetoothHost[i]);
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(strAddress);
                        }
                        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        sprintf(strAddress,"%02x", BluetoothHost[activeByte]);
                        tft.print(strAddress);
                        for (int i = (activeByte + 1); i < 6; i++) {
                                sprintf(strAddress,":%02x", BluetoothHost[i]);
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(strAddress);
                        }
                }

                int count = readEncoder();
                resetEncoder();

                if (count != 0) {
                        BluetoothHost[activeByte] = BluetoothHost[activeByte] + count;
                        tft.fillRect(ConfigFields[2][0], ConfigFields[2][1], ConfigFields[2][2], ConfigFields[2][3], TFT_COLOR_FRM_BGR);
                        tft.setCursor(ConfigFields[2][0] + 7, ConfigFields[2][1] + 4);
                        for (int i = 0; i < activeByte; i++) {
                                sprintf(strAddress,"%02x:", BluetoothHost[i]);
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(strAddress);
                        }
                        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        sprintf(strAddress,"%02x", BluetoothHost[activeByte]);
                        tft.print(strAddress);
                        for (int i = (activeByte + 1); i < 6; i++) {
                                sprintf(strAddress,":%02x", BluetoothHost[i]);
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(strAddress);
                        }
                }
        }
}


String BLEDeviceList[11]; // 10 (+1 for "BACK")
uint8_t BLEAddressList[10][6];
bool BLEDeviceFound = false;
int BLEDeviceNum;

class MyConfigAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {

        void onResult(BLEAdvertisedDevice advertisedDevice) {

                BLEAdvertisedDevice FoundDevice = advertisedDevice;

                BLEDeviceNum++;

                if (BLEDeviceNum < 10) {

                        if (strcmp(FoundDevice.getName().c_str(), "") != 0) {
                                BLEDeviceList[BLEDeviceNum] = FoundDevice.getName().c_str();
                                BLEDeviceList[BLEDeviceNum+1] = "Back";
                        }
                        else {
                                BLEDeviceList[BLEDeviceNum] = FoundDevice.getAddress().toString().c_str();
                                BLEDeviceList[BLEDeviceNum+1] = "Back";
                        }
                        
                        memcpy(BLEAddressList[BLEDeviceNum], FoundDevice.getAddress().getNative(), 6);

                        // debugf(DEBUG_FLAG_BOTH, "NAME : %d :   ADDRESS : %s", BLEDeviceList[BLEDeviceNum], FoundDevice.getAddress().toString());

                        TFTConfigPrint(BLEDeviceNum+1, "  " + BLEDeviceList[BLEDeviceNum], TFT_COLOR_CNF_STD);
                        TFTConfigPrint(BLEDeviceNum+2, "> Scanning...", TFT_COLOR_CNF_STD);

                }
    }
};


BLEScan* pBLEScan;

void configBLE() {

        const uint16_t BLEScanTime = 10; //In seconds
        byte activeMenu = 1;
        BLEDeviceNum = -1;
        char strAddress[20];

        TFTConfigPrepare();
        TFTConfigPrint(0, "Bluetooth SSID", TFT_COLOR_CNF_STD);
        TFTSetFontSize(2);
        TFTConfigPrint(1, "> Scanning...", TFT_COLOR_CNF_STD);
        // TFTConfigPrint(11, "  Back", TFT_COLOR_CNF_STD);

        BLEDevice::init("");
        pBLEScan = BLEDevice::getScan(); //create new scan
        pBLEScan->setAdvertisedDeviceCallbacks(new MyConfigAdvertisedDeviceCallbacks());
        // pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
        pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
        pBLEScan->setInterval(100);
        pBLEScan->setWindow(99);  // less or equal setInterval value
        // pBLEScan->setWindow(50);  // less or equal setInterval value
        
        BLEScanResults foundDevices = pBLEScan->start(BLEScanTime, false);

        // for (int i = 0; i < BLEDeviceNum; i++) {
        //         TFTConfigPrint(BLEDeviceNum+1, "  " + BLEDeviceList[i], TFT_COLOR_CNF_STD);
        // }

        TFTConfigPrint(BLEDeviceNum+2, BLEDeviceList[BLEDeviceNum+1], TFT_COLOR_CNF_STD);

        pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory

        while (true) {

                int count = readEncoder();
                resetEncoder();

                if (count != 0) {

                        TFTConfigPrint(activeMenu, "  " + BLEDeviceList[activeMenu-1], TFT_COLOR_CNF_STD);
                        activeMenu = activeMenu + count;
                        if (activeMenu > (BLEDeviceNum + 2)) { activeMenu = 1; }
                        if (activeMenu < 1) { activeMenu = (BLEDeviceNum + 2); }

                        TFTConfigPrint(activeMenu, "> " + BLEDeviceList[activeMenu-1], TFT_COLOR_CNF_STD);
                }

                if (checkEnter()) {
                        if (activeMenu == (BLEDeviceNum + 2)) {
                                return;
                        }
                        else {
                                memcpy(BluetoothHost, BLEAddressList[activeMenu-1], sizeof(BluetoothHost));
                                copyArray(BluetoothHost, configDoc[EEBluetoothHost]);
                                sprintf(strAddress,"%02x:%02x:%02x:%02x:%02x:%02x",BluetoothHost[0],BluetoothHost[1],BluetoothHost[2],BluetoothHost[3],BluetoothHost[4],BluetoothHost[5]);
                                // debugf(DEBUG_FLAG_BOTH, "Bluetooth Name: %s  Bluetooth Host: %s", BLEDeviceList[activeMenu-1], strAddress);
                                return;
                        }
                }
                delay(ENCODER_TICKER_TIME);
        }
}

/*
void configBluetoothPin() {
        BluetoothPin = EepromReadInt(EEBluetoothPin);
        BluetoothPin = (int)TFTConfigValue("Bluetooth Pin", 0, 9999, BluetoothPin, 0, "", 4);
        debug("CONFIG : BluetoothPin : " + String(BluetoothPin));
        EepromWriteInt(BluetoothPin, EEBluetoothPin);
}
*/

void configJogging() {
        const byte ContentNum = 6;
        const String Content[ContentNum] = {
                "Jogging",
                "X Speed",
                "Y Speed",
                "Z Speed",
                "A Speed",
                "Back",
        };
        while(true) {
                switch (TFTConfigMenu(Content, ContentNum)) {
                case 1: configJoggingX(); break;
                case 2: configJoggingY(); break;
                case 3: configJoggingZ(); break;
                case 4: configJoggingA(); break;
                case 5: return;
                }
        }
}

void configJoggingX() {
        // JogSpeed[0] = EepromReadInt(EEJogSpeed);
        JogSpeed[0] = (int)TFTConfigValue("Jogging X", 0, 10000, JogSpeed[0], 2, " mm/min", 0);
        // debugf(DEBUG_FLAG_BOTH, "CONFIG : JogSpeedX : %d", JogSpeed[0]);
        configDoc[EEJogSpeed][0] = JogSpeed[0];
        // EepromWriteInt(JogSpeed[0], EEJogSpeed);
}
void configJoggingY() {
        // JogSpeed[1] = EepromReadInt(EEJogSpeed+2);
        JogSpeed[1] = (int)TFTConfigValue("Jogging Y", 0, 10000, JogSpeed[1], 2, " mm/min", 0);
        // debugf(DEBUG_FLAG_BOTH, "CONFIG : JogSpeedY : %d", JogSpeed[1]);
        configDoc[EEJogSpeed][1] = JogSpeed[1];
        // EepromWriteInt(JogSpeed[1], EEJogSpeed+2);
}
void configJoggingZ() {
        // JogSpeed[2] = EepromReadInt(EEJogSpeed+4);
        JogSpeed[2] = (int)TFTConfigValue("Jogging Z", 0, 10000, JogSpeed[2], 2, " mm/min", 0);
        // debugf(DEBUG_FLAG_BOTH, "CONFIG : JogSpeedZ : %d", JogSpeed[2]);
        configDoc[EEJogSpeed][2] = JogSpeed[2];
        // EepromWriteInt(JogSpeed[2], EEJogSpeed+4);
}
void configJoggingA() {
        // JogSpeed[3] = EepromReadInt(EEJogSpeed+6);
        JogSpeed[3] = (int)TFTConfigValue("Jogging A", 0, 32000, JogSpeed[3], 2, " mm/min", 0);
        // debugf(DEBUG_FLAG_BOTH, "CONFIG : JogSpeedA : %d", JogSpeed[3]);
        configDoc[EEJogSpeed][3] = JogSpeed[3];
        // EepromWriteInt(JogSpeed[3], EEJogSpeed+6);
}


void configWorkspace() {
        const byte ContentNum = 8;
        const String Content[ContentNum] = {
                "Workspace",
                "G54 (Standard)",
                "G55",
                "G56",
                "G57",
                "G58",
                "G59",
                "Back"
        };

        while(true) {
                switch (TFTConfigMenu(Content, ContentNum)) {
                case 1: WorkSpace = 54; break;
                case 2: WorkSpace = 55; break;
                case 3: WorkSpace = 56; break;
                case 4: WorkSpace = 57; break;
                case 5: WorkSpace = 58; break;
                case 6: WorkSpace = 59; break;
                case 7: return;
                }
                bluetoothSend("gcode", "G" + String(WorkSpace), "G" + String(WorkSpace));
                return;
        }
}

void configProbe() {
        const byte ContentNum = 7;
        const String Content[ContentNum] = {
                "Probe",
                "Offset",
                "Depth",
                "Speed",
                "Return Height",
                "Probe Time",
                "Back"
        };

        while(true) {
                switch (TFTConfigMenu(Content, ContentNum)) {
                case 1: configProbeOffset(); break;
                case 2: configProbeDepth(); break;
                case 3: configProbeSpeed(); break;
                case 4: configProbeBackHeight(); break;
                case 5: configProbeTime(); break;
                case 6: return;
                }
        }
}


void configProbeOffset() {
        ProbeOffset = TFTConfigValue("Probe Offset", 0, 50, ProbeOffset, -2, " mm", 0);
        // debugf(DEBUG_FLAG_BOTH, "CONFIG : ProbeOffset : %.3f", ProbeOffset);
        configDoc[EEProbeOffset] = ProbeOffset;
}


void configProbeDepth() {
        if (ProbeDepth > 32767) { ProbeDepth = ProbeDepth - 65536; }  // 65535
        ProbeDepth = (int)TFTConfigValue("Probe Depth", -100, 100, ProbeDepth, 0, " mm", 0);
        // debugf(DEBUG_FLAG_BOTH, "CONFIG : ProbeDepth : %d", ProbeDepth);
        configDoc[EEProbeDepth] = ProbeDepth;
}


void configProbeSpeed() {
        ProbeSpeed = (int)TFTConfigValue("Probe Speed", 0, 1000, ProbeSpeed, 0, " mm / min", 0);
        // debugf(DEBUG_FLAG_BOTH, "CONFIG : ProbeSpeed : %d", ProbeSpeed);
        configDoc[EEProbeSpeed] = ProbeSpeed;
}


void configProbeBackHeight() {
        ProbeBackHeight = (byte)TFTConfigValue("Probe Rise", 0, 100, ProbeBackHeight, 0, " mm", 0);
        // debugf(DEBUG_FLAG_BOTH, "CONFIG : ProbeBackHeight : %d", ProbeBackHeight);
        configDoc[EEProbeBackHeight] = ProbeBackHeight;
}

void configProbeTime() {
        ProbeTime = (byte)TFTConfigValue("Probe Time", 1, 20, ProbeTime, 0, " sec", 0);
        // debugf(DEBUG_FLAG_BOTH, "CONFIG : ProbeTime : %d", ProbeTime);
        configDoc[EEProbeTime] = ProbeTime;
}

void configSleep() {
        SleepTime = (byte)TFTConfigValue("Sleep Time", 0, 20, SleepTime, 0, " min", 0);
        // debugf(DEBUG_FLAG_BOTH, "CONFIG : SleepTime : %d", SleepTime);
        configDoc[EESleepTime] = SleepTime;
        SleepTicker.interval(SleepTime * 60000);
        if (SleepTime == 0) { SleepTicker.stop(); }
}

void configBrightness() {
        TFT_BRIGHTNESS = (byte)TFTConfigValue("Brightness", 0, 255, TFT_BRIGHTNESS, 0, "", 0);
        // debugf(DEBUG_FLAG_BOTH, "CONFIG : TftBrightness : %d", TFT_BRIGHTNESS);
        configDoc[EEBrightness] = TFT_BRIGHTNESS;
        analogWrite(TFT_LED, TFT_BRIGHTNESS);
}

void configBattery() {
        float V = readBattery();
        const byte ContentNum = 6;
        const String Content[ContentNum] = {
                "Battery",
                "",
                "Voltage: " + String(V) + "V",
                "Fill:    " + String (percentageBattery(V)) + "%",
                "",
                "Back"
        };
        TFTConfigInfo(Content, ContentNum);
}

void configInfo() {
        const byte ContentNum = 7;
        const String Content[ContentNum] = {
                "Info",
                "(c) 2022",
                "Paul Schwaderer",
                "github.com/agent-r",
                "/grblhal-pendant",
                "",
                "Back"
        };
        TFTConfigInfo(Content, ContentNum);
}
