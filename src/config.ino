
///////////////////////////////////////////////////////////////////////////
///////////////////////      CONFIG      //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

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
        const byte ContentNum = 11;
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
                "Firmware Update",
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
                case 9: configOTA(); break;
                case 10: TFTPrepare(); ConnectionSetup(); return;
                }
        }
}

void configConnection() {
        const byte ContentNum = 4;
        const String Content[ContentNum] = {
                "Connection",
                "Connection Mode",
                "Connection Settings",
                "Back"
        };
        while(true) {
                switch (TFTConfigMenu(Content, ContentNum)) {
                case 1: configConnectionMode(); break;
                case 2: configConnectionSettings(); break;
                case 3: return;
                }
        }
}

void configConnectionMode() {
        const byte ContentNum = 5;
        String Content[ContentNum] = {
                "Connect Mode",
                "WIFI Slave",
                "WIFI AP",
                "BLUETOOTH Slave",
                "Back"
        };
        ConnectionMode = EEPROM.read(EEConnectionMode);
        // while(true) {
        switch (TFTConfigMenu(Content, ContentNum)) {
        case 1: ConnectionMode = 0; break;                // EEPROM.write(112, ConnectionMode); EEPROM.commit(); return;
        case 2: ConnectionMode = 1; break;                // EEPROM.write(112, ConnectionMode); EEPROM.commit(); return;
        case 3: ConnectionMode = 2; break;                // EEPROM.write(112, ConnectionMode); EEPROM.commit(); return;
        case 4: return;
        }
        if (SERIAL_DEBUG) { Serial.println("CONFIG : ConnectionMode : " + String(ConnectionMode)); }
        EEPROM.write(EEConnectionMode, ConnectionMode); EEPROM.commit();
        ConnectionSetup();
        // }
}

void configConnectionSettings() {
        const byte ContentNumWifi = 6;
        const String ContentWifi[ContentNumWifi] = {
                "WiFi",
                "WiFi SSID",
                "WiFi Password",
                "WiFi Host IP",
                "WiFi Host Port",
                "Back"
        };
        const byte ContentNumAP = 6;
        const String ContentAP[ContentNumAP] = {
                "WiFi AP",
                "AP SSID",
                "AP Password",
                "AP Host IP",
                "AP Host Port",
                "Back"
        };
        const byte ContentNumBluetooth = 5;
        const String ContentBluetooth[ContentNumBluetooth] = {
                "Bluetooth",
                "Bluetooth SSID",
                "Bluetooth Host",
                "Bluetooth Pin",
                "Back"
        };

        switch (ConnectionMode) {
        case 0:
                while(true) {
                        switch (TFTConfigMenu(ContentWifi, ContentNumWifi)) {
                        case 1: configWifiSSID(); break;
                        case 2: configWifiPW(); break;
                        case 3: configWifiHost(); break;
                        case 4: configWifiPort(); break;
                        case 5: return;
                        }
                }
        case 1:
                while(true) {
                        switch (TFTConfigMenu(ContentAP, ContentNumAP)) {
                        case 1: configAPSSID(); break;
                        case 2: configAPPW(); break;
                        case 3: configAPHost(); break;
                        case 4: configAPPort(); break;
                        case 5: return;
                        }
                }
        case 2:
                while(true) {
                        switch (TFTConfigMenu(ContentBluetooth, ContentNumBluetooth)) {
                        case 1: configBluetoothSSID(); break;
                        case 2: configBluetoothAddress(); break;
                        case 3: configBluetoothPin(); break;
                        case 4: return;
                        }
                }
        }
}

void configWifiSSID() {
 #define MaxSSIDs 10
        String SSIDs[MaxSSIDs+1];
        int numberOfNetworks = WiFi.scanNetworks();
        if (numberOfNetworks > MaxSSIDs) { numberOfNetworks = MaxSSIDs; }
        int Selection;

        SSIDs[0] = "Wifi SSID";
        for(int i = 1; i <= numberOfNetworks; i++) {
                SSIDs[i] = WiFi.SSID(i-1);
        }
        SSIDs[numberOfNetworks + 1] = "Back";
        for (int i = numberOfNetworks + 2; i <= MaxSSIDs; i++) {
                SSIDs[i] = "";
        }

        Selection = TFTConfigMenu(SSIDs, numberOfNetworks + 2);

        if (Selection <= numberOfNetworks) {
                WifiSSID = WiFi.SSID(Selection - 1);
                if (SERIAL_DEBUG) { Serial.println("CONFIG : WifiSSID : " + WifiSSID); }
                EepromWriteString(WifiSSID, EEWifiSSID, 30);
        }
        else if (Selection == numberOfNetworks+1) {
                return;
        }
}

void configWifiPW() {
        WifiPW = EepromReadString(EEWifiPW, 30);
        WifiPW = TFTConfigString("Wifi Password", WifiPW);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : WifiPW : \"" + WifiPW + "\""); }
        EepromWriteString(WifiPW, EEWifiPW, 30);
}

void configWifiHost() {
        WifiHost = EepromReadIP(EEWifiHost);
        WifiHost = TFTConfigIP("Wifi Host", WifiHost);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : WifiHost : " + String(WifiHost[0]) + "." + String(WifiHost[1]) + "." + String(WifiHost[2]) + "." + String(WifiHost[3])); }
        EepromWriteIP(WifiHost, EEWifiHost);
}

void configWifiPort() {
        WifiPort = EepromReadInt(EEWifiPort);
        WifiPort = TFTConfigValue("Wifi Port", 0, 9999, WifiPort, 0, "", 4);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : WifiPort : " + String(WifiPort)); }
        EepromWriteInt(WifiPort, EEWifiPort);
}

void configAPSSID() {
        APSSID = EepromReadString(EEAPSSID, 30);
        APSSID = TFTConfigString("AP SSID", APSSID);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : APSSID : \"" + APSSID + "\""); }
        EepromWriteString(APSSID, EEAPSSID, 30);
}

void configAPPW() {
        APPW = EepromReadString(EEAPPW, 30);
        APPW = TFTConfigString("AP Password", APPW);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : APPassword : \"" + APPW + "\""); }
        EepromWriteString(APPW, EEAPPW, 30);
}

void configAPHost() {
        APHost = EepromReadIP(EEAPHost);
        APHost = TFTConfigIP("AP Host IP", APHost);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : APHost : " + String(APHost[0]) + "." + String(APHost[1]) + "." + String(APHost[2]) + "." + String(APHost[3])); }
        EepromWriteIP(APHost, EEAPHost);
}

void configAPPort() {
        APPort = EepromReadInt(EEAPPort);
        APPort = TFTConfigValue("AP Port", 0, 9999, APPort, 0, "", 4);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : APPort : " + String(APPort)); }
        EepromWriteInt(APPort, EEAPPort);
}

void configBluetoothAddress() {
        for (int i = 0; i < 6; i++) {
                BluetoothHost[i] = EEPROM.read(EEBluetoothHost + i);
        }
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
                delay(20);
                if (checkEnter()) {
                        activeByte++;

                        if (activeByte >= 6) {
                                for (int i = 0; i < 6; i++) { EEPROM.write(EEBluetoothHost + i, BluetoothHost[i]);                                 }
                                if(SERIAL_DEBUG) {
                                        Serial.print("Bluetooth Host: ");
                                        sprintf(strAddress,"%02x:%02x:%02x:%02x:%02x:%02x",BluetoothHost[0],BluetoothHost[1],BluetoothHost[2],BluetoothHost[3],BluetoothHost[4],BluetoothHost[5]);
                                        Serial.println(strAddress);
                                        Serial.println();
                                }
                                EEPROM.commit();
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

                // int count = rotaryEncoder.getCount();
                int count = rotaryEncoder.readEncoder();
                if (count != 0) {
                        BluetoothHost[activeByte] = BluetoothHost[activeByte] + count;
                        // rotaryEncoder.clearCount();
                        rotaryEncoder.reset();
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

#ifdef USE_NEW_ARDUINO_ESP
String BluetoothSSIDNameList[10];
uint8_t BluetoothAddressList[10][6];
bool BluetoothSSIDFound = false;
int BluetoothSSIDNum = -1;
#endif

void configBluetoothDeviceFound(
    #ifdef USE_NEW_ARDUINO_ESP
        BTAdvertisedDevice* pDevice
    #endif
        ) {

   #ifdef USE_NEW_ARDUINO_ESP

        String strBluetoothSSID;
        char strBluetoothAddress[20];
        BluetoothSSIDFound = true;
        BluetoothSSIDNum++;
        if (BluetoothSSIDNum < 10) {
                // BluetoothSSIDNameList[BluetoothSSIDNum] = pDevice->toString().c_str();
                // BluetoothSSIDNameList[BluetoothSSIDNum] = pDevice->getName().c_str();
                strBluetoothSSID = pDevice->getName().c_str();
                memcpy(BluetoothAddressList[BluetoothSSIDNum], pDevice->getAddress().getNative(), 6);

                if (strBluetoothSSID == "") {
                        sprintf(strBluetoothAddress,"%02x:%02x:%02x:%02x:%02x:%02x",BluetoothAddressList[BluetoothSSIDNum][0],BluetoothAddressList[BluetoothSSIDNum][1],BluetoothAddressList[BluetoothSSIDNum][2],BluetoothAddressList[BluetoothSSIDNum][3],BluetoothAddressList[BluetoothSSIDNum][4],BluetoothAddressList[BluetoothSSIDNum][5]);
                        BluetoothSSIDNameList[BluetoothSSIDNum] = String(strBluetoothAddress);
                }
                else {
                        BluetoothSSIDNameList[BluetoothSSIDNum] = strBluetoothSSID;
                }
                // BluetoothAddressListString[BluetoothSSIDNum] = pDevice->getAddress().toString().c_str();
        }
        #endif
}

void configBluetoothSSID() {

#ifdef USE_NEW_ARDUINO_ESP

        char strAddress[20];
        byte activeMenu = 1;

        TFTConfigPrepare();
        TFTConfigPrint(0, "Bluetooth SSID", TFT_COLOR_CNF_STD);
        TFTSetFontSize(2);
        // btSerial.disconnect();
        // btSerial.end();
        // btSerial.begin("BTMaster", true);      // true = master
        // delay(100);

        TFTConfigPrint(1, "> ", TFT_COLOR_CNF_STD);
        TFTConfigPrint(11, "  Back", TFT_COLOR_CNF_STD);

        if (btSerial.discoverAsync(configBluetoothDeviceFound)) {
                if (SERIAL_DEBUG) { Serial.println("Start to look for BT devices..."); }
        } else {
                if (SERIAL_DEBUG) { Serial.println("Error on discoverAsync f.e. not workin after a \"connect\""); }
        }

        while (true) {
                if (BluetoothSSIDFound) {
                        if (SERIAL_DEBUG) { Serial.println("... found something !"); }
                        if (BluetoothSSIDNameList[BluetoothSSIDNum] != "") {
                                if (SERIAL_DEBUG) {
                                        Serial.print("... it has a name :");
                                        Serial.println(BluetoothSSIDNameList[BluetoothSSIDNum]);
                                }
                                TFTConfigPrint(BluetoothSSIDNum+1, BluetoothSSIDNameList[BluetoothSSIDNum], TFT_COLOR_CNF_STD);
                        }
                        else {

                                sprintf(strAddress,"  %02x:%02x:%02x:%02x:%02x:%02x",BluetoothAddressList[BluetoothSSIDNum][0],BluetoothAddressList[BluetoothSSIDNum][1],BluetoothAddressList[BluetoothSSIDNum][2],BluetoothAddressList[BluetoothSSIDNum][3],BluetoothAddressList[BluetoothSSIDNum][4],BluetoothAddressList[BluetoothSSIDNum][5]);
                                if (SERIAL_DEBUG) {
                                        Serial.print("... it has a number :");
                                        Serial.println(strAddress);
                                }
                                TFTConfigPrint(BluetoothSSIDNum+1, strAddress, TFT_COLOR_CNF_STD);
                                // TFTConfigPrint(BluetoothSSIDNum+1, String(BluetoothAddressListString[BluetoothSSIDNum]), TFT_COLOR_CNF_STD);
                        }
                        BluetoothSSIDFound = false;
                }

                int count = rotaryEncoder.readEncoder();
                rotaryEncoder.reset();

                if ((count != 0) || BluetoothSSIDFound) {

                        for (int i = 0; i < BluetoothSSIDNum; i++) {
                                TFTConfigPrint(i+1, "  " + BluetoothSSIDNameList[i], TFT_COLOR_CNF_STD);
                        }
                        // TFTConfigPrint(activeMenu, "  " + Content[activeMenu], TFT_COLOR_CNF_STD);
                        activeMenu = activeMenu + count;
                        // if (activeMenu > (Length - 1)) {activeMenu = 1;}
                        // if (activeMenu < 1) {activeMenu = (Length - 1);}
                        // TFTConfigPrint(activeMenu, "> " + Content[activeMenu], TFT_COLOR_CNF_STD);
                        TFTConfigPrint(BluetoothSSIDNum+2, "  Back", TFT_COLOR_CNF_STD);

                        // rotaryEncoder.clearCount();
                        rotaryEncoder.reset();
                        BluetoothSSIDFound = false;
                }

                if (checkEnter()) {
                        // return(activeMenu);
                        return;
                }
                delay(20);
        }

        Serial.print("Stop Search");
        btSerial.discoverAsyncStop();

#endif
}


void configBluetoothPin() {
        BluetoothPin = EepromReadInt(EEBluetoothPin);
        BluetoothPin = (int)TFTConfigValue("Bluetooth Pin", 0, 9999, BluetoothPin, 0, "", 4);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : BluetoothPin : " + String(BluetoothPin)); }
        EepromWriteInt(BluetoothPin, EEBluetoothPin);
}

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
        JogSpeed[0] = EepromReadInt(EEJogSpeed);
        JogSpeed[0] = (int)TFTConfigValue("Jogging X", 0, 10000, JogSpeed[0], 2, " mm/min", 0);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : JogSpeedX : " + String(JogSpeed[0])); }
        EepromWriteInt(JogSpeed[0], EEJogSpeed);
}
void configJoggingY() {
        JogSpeed[1] = EepromReadInt(EEJogSpeed+2);
        JogSpeed[1] = (int)TFTConfigValue("Jogging Y", 0, 10000, JogSpeed[1], 2, " mm/min", 0);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : JogSpeedY : " + String(JogSpeed[1])); }
        EepromWriteInt(JogSpeed[1], EEJogSpeed+2);
}
void configJoggingZ() {
        JogSpeed[2] = EepromReadInt(EEJogSpeed+4);
        JogSpeed[2] = (int)TFTConfigValue("Jogging Z", 0, 10000, JogSpeed[2], 2, " mm/min", 0);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : JogSpeedZ : " + String(JogSpeed[2])); }
        EepromWriteInt(JogSpeed[2], EEJogSpeed+4);
}
void configJoggingA() {
        JogSpeed[3] = EepromReadInt(EEJogSpeed+6);
        JogSpeed[3] = (int)TFTConfigValue("Jogging A", 0, 10000, JogSpeed[3], 2, " mm/min", 0);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : JogSpeedA : " + String(JogSpeed[3])); }
        EepromWriteInt(JogSpeed[3], EEJogSpeed+6);
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
                sendCmd("gcode", "G" + String(WorkSpace), "G" + String(WorkSpace));
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
        ProbeOffset = EepromReadFloat(EEProbeOffset);
        // float ProbeOffsetFloat = ProbeOffset / 100.0;
        ProbeOffset = TFTConfigValue("Probe Offset", 0, 50, ProbeOffset, -2, " mm", 0);
        // ProbeOffset = (int)(ProbeOffsetFloat * 100);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : ProbeOffset : " + String(ProbeOffset)); }
        EepromWriteFloat(ProbeOffset, EEProbeOffset);
}


void configProbeDepth() {
        ProbeDepth = EepromReadInt(EEProbeDepth);
        if (ProbeDepth > 32767) { ProbeDepth = ProbeDepth - 65536; }  // 65535
        ProbeDepth = (int)TFTConfigValue("Probe Depth", -100, 100, ProbeDepth, 0, " mm", 0);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : ProbeDepth : " + String(ProbeDepth)); }
        EepromWriteInt(ProbeDepth, EEProbeDepth);
}


void configProbeSpeed() {
        ProbeSpeed = EepromReadInt(EEProbeSpeed);
        ProbeSpeed = (int)TFTConfigValue("Probe Speed", 0, 1000, ProbeSpeed, 0, " mm / min", 0);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : ProbeSpeed : " + String(ProbeSpeed)); }
        EepromWriteInt(ProbeSpeed, EEProbeSpeed);
}


void configProbeBackHeight() {
        ProbeBackHeight = EEPROM.read(EEProbeBackHeight);
        ProbeBackHeight = (byte)TFTConfigValue("Probe Rise", 0, 100, ProbeBackHeight, 0, " mm", 0);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : ProbeBackHeight : " + String(ProbeBackHeight)); }
        EEPROM.write(EEProbeBackHeight, ProbeBackHeight);
        EEPROM.commit();
}

void configProbeTime() {
        ProbeTime = EEPROM.read(EEProbeTime);
        ProbeTime = (byte)TFTConfigValue("Probe Time", 1, 20, ProbeTime, 0, " sec", 0);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : ProbeTime : " + String(ProbeTime)); }
        EEPROM.write(EEProbeTime, ProbeTime);
        EEPROM.commit();
}

void configSleep() {
        SleepTime = EEPROM.read(EESleepTime);
        SleepTime = (byte)TFTConfigValue("Sleep Time", 0, 20, SleepTime, 0, " min", 0);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : SleepTime : " + String(SleepTime)); }
        EEPROM.write(EESleepTime, SleepTime);
        EEPROM.commit();
        SleepTicker.interval(SleepTime * 60000);
        if (SleepTime == 0) { SleepTicker.stop(); }
}

void configBrightness() {
        TFT_BRIGHTNESS = EEPROM.read(EEBrightness);
        TFT_BRIGHTNESS = (byte)TFTConfigValue("Brightness", 0, 255, TFT_BRIGHTNESS, 0, "", 0);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : TftBrightness : " + String(TFT_BRIGHTNESS)); }
        EEPROM.write(EEBrightness, TFT_BRIGHTNESS);
        EEPROM.commit();
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


void configOTA() {
        const char * otassid = "OTA";       // You will connect your penant to this Access Point
        const char * otapw = "OTAOTA00";        // and this is the password
        const IPAddress otaip(192, 168, 0, 1);       // this is the ip
        const IPAddress otanetmask(255, 255, 255, 0);
        // const int otaport = 8880;

        const byte ContentNum = 10;
        String Content[ContentNum] = {
                "Firmware",
                "OTA-Upload:",
                "",
                "NOT WORKING YET!",
                "",
                "SSID: \"OTA\"",
                "PW:   \"OTAOTA00\"",
                "IP:   192.168.0.1",
                "",
                "Back"
        };

        WiFi.disconnect();
        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(otaip, otaip, otanetmask);
        WiFi.softAP(otassid, otapw);
        // WiFi.softAP(otassid);
        ArduinoOTA.begin();

        // Content[5] = "IP:   " + WiFi.softAPIP().toString();

        TFTConfigPrepare();
        for (int i = 0; i < ContentNum; i++) {
                TFTConfigPrint(i, Content[i], TFT_COLOR_CNF_STD);
        }

        while(!checkEnter()) {
                ArduinoOTA.handle();
        }

        ArduinoOTA.end();
        WiFi.softAPdisconnect (true);

}
