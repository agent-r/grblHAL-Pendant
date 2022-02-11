
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
        const byte ContentNum = 9;
        const String Content[ContentNum] = {
                "Config",
                "Connection",
                "Jogging",
                "Probe",
                "Sleep Mode",
                "Brightness",
                "Battery",
                "Info",
                "Exit"
        };
        while(true) {
                switch (TFTConfigMenu(Content, ContentNum)) {
                case 1: configConnection(); break;
                case 2: configJogging(); break;
                case 3: configProbe(); break;
                case 4: configSleep(); break;
                case 5: configBrightness(); break;
                case 6: configBattery(); break;
                case 7: configInfo(); break;
                case 8: TFTPrepare(); ConnectionSetup(); return;
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
                "SSID",
                "Password",
                "Host IP",
                "Host Port",
                "Back"
        };
        const byte ContentNumAP = 6;
        const String ContentAP[ContentNumAP] = {
                "WiFi AP",
                "SSID",
                "Password",
                "Host IP",
                "Host Port",
                "Back"
        };
        const byte ContentNumBluetooth = 4;
        const String ContentBluetooth[ContentNumBluetooth] = {
                "Bluetooth",
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
                        case 1: configBluetoothHost(); break;
                        case 2: configBluetoothPin(); break;
                        case 3: return;
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
        WifiPort = TFTConfigValue("Wifi Port", 0, 9999, WifiPort, 1, "");
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
        APPort = TFTConfigValue("AP Port", 0, 9999, APPort, 1, "");
        if (SERIAL_DEBUG) { Serial.println("CONFIG : APPort : " + String(APPort)); }
        EepromWriteInt(APPort, EEAPPort);
}


void configBluetoothHost() {
        for (int i = 0; i < 6; i++) {
                BluetoothHost[i] = EEPROM.read(EEBluetoothHost + i);
        }
        byte activeByte = 0;

        TFTConfigPrepare();
        TFTConfigPrint(0, "Bluetooth Addr,", TFT_COLOR_CNF_STD);
        TFTSetFontSize(2);
        tft.setCursor(ConfigFields[2][0]+7, ConfigFields[2][1]+4);

        for (int i = 0; i < activeByte; i++) {
                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                tft.print(BluetoothHost[i], HEX); tft.print(":");
        }
        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
        tft.print(BluetoothHost[activeByte], HEX);
        for (int i = (activeByte + 1); i < 6; i++) {
                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                tft.print(":"); tft.print(BluetoothHost[i], HEX);
        }

        while (true) {
                delay(20);
                if (checkEnter()) {
                        activeByte++;

                        if (activeByte >= 6) {
                                Serial.print("Bluetooth Host: ");
                                for (int i = 0; i < 6; i++) {
                                        EEPROM.write(EEBluetoothHost + i, BluetoothHost[i]);
                                        Serial.print(BluetoothHost[i], HEX); Serial.print(":");
                                }
                                EEPROM.commit();
                                Serial.println();
                                return;
                        }

                        tft.fillRect(ConfigFields[2][0], ConfigFields[2][1], ConfigFields[2][2], ConfigFields[2][3], TFT_COLOR_FRM_BGR);
                        tft.setCursor(ConfigFields[2][0] + 7, ConfigFields[2][1] + 4);
                        for (int i = 0; i < activeByte; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(BluetoothHost[i], HEX); tft.print(":");
                        }
                        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        tft.print(BluetoothHost[activeByte], HEX);
                        for (int i = (activeByte + 1); i < 6; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(":"); tft.print(BluetoothHost[i], HEX);
                        }
                }

                int count = rotaryEncoder.getCount();
                if (count != 0) {
                        BluetoothHost[activeByte] = BluetoothHost[activeByte] + count;
                        rotaryEncoder.clearCount();
                        tft.fillRect(ConfigFields[2][0], ConfigFields[2][1], ConfigFields[2][2], ConfigFields[2][3], TFT_COLOR_FRM_BGR);
                        tft.setCursor(ConfigFields[2][0] + 7, ConfigFields[2][1] + 4);

                        for (int i = 0; i < activeByte; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(BluetoothHost[i], HEX); tft.print(":");
                        }

                        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        tft.print(BluetoothHost[activeByte], HEX);

                        for (int i = (activeByte + 1); i < 6; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(":"); tft.print(BluetoothHost[i], HEX);
                        }
                }
        }
}

void configBluetoothPin() {
        BluetoothPin = EepromReadInt(EEBluetoothPin);
        BluetoothPin = (int)TFTConfigValue("BluetoothPin", 0, 9999, BluetoothPin, 1, "");
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
        JogSpeed[0] = (int)TFTConfigValue("Jogging X", 0, 10000, JogSpeed[0], 100, "mm/min");
        if (SERIAL_DEBUG) { Serial.println("CONFIG : JogSpeedX : " + String(JogSpeed[0])); }
        EepromWriteInt(JogSpeed[0], EEJogSpeed);
}
void configJoggingY() {
        JogSpeed[1] = EepromReadInt(EEJogSpeed+2);
        JogSpeed[1] = (int)TFTConfigValue("Jogging Y", 0, 10000, JogSpeed[1], 100, "mm/min");
        if (SERIAL_DEBUG) { Serial.println("CONFIG : JogSpeedY : " + String(JogSpeed[1])); }
        EepromWriteInt(JogSpeed[1], EEJogSpeed+2);
}
void configJoggingZ() {
        JogSpeed[2] = EepromReadInt(EEJogSpeed+4);
        JogSpeed[2] = (int)TFTConfigValue("Jogging Z", 0, 10000, JogSpeed[2], 100, "mm/min");
        if (SERIAL_DEBUG) { Serial.println("CONFIG : JogSpeedZ : " + String(JogSpeed[2])); }
        EepromWriteInt(JogSpeed[2], EEJogSpeed+4);
}
void configJoggingA() {
        JogSpeed[3] = EepromReadInt(EEJogSpeed+6);
        JogSpeed[3] = (int)TFTConfigValue("Jogging A", 0, 10000, JogSpeed[3], 100, "mm/min");
        if (SERIAL_DEBUG) { Serial.println("CONFIG : JogSpeedA : " + String(JogSpeed[3])); }
        EepromWriteInt(JogSpeed[3], EEJogSpeed+6);
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
        ProbeOffset = TFTConfigValue("Probe Offset", 0, 50, ProbeOffset, 0.01, "mm");
        // ProbeOffset = (int)(ProbeOffsetFloat * 100);
        if (SERIAL_DEBUG) { Serial.println("CONFIG : ProbeOffset : " + String(ProbeOffset)); }
        EepromWriteFloat(ProbeOffset, EEProbeOffset);
}


void configProbeDepth() {
        ProbeDepth = EepromReadInt(EEProbeDepth);
        if (ProbeDepth > 32767) { ProbeDepth = ProbeDepth - 65536; }  // 65535
        ProbeDepth = (int)TFTConfigValue("Probe Depth", -100, 100, ProbeDepth, 1, "mm");
        if (SERIAL_DEBUG) { Serial.println("CONFIG : ProbeDepth : " + String(ProbeDepth)); }
        EepromWriteInt(ProbeDepth, EEProbeDepth);
}


void configProbeSpeed() {
        ProbeSpeed = EepromReadInt(EEProbeSpeed);
        ProbeSpeed = (int)TFTConfigValue("Probe Speed", 0, 1000, ProbeSpeed, 1, "mm / min");
        if (SERIAL_DEBUG) { Serial.println("CONFIG : ProbeSpeed : " + String(ProbeSpeed)); }
        EepromWriteInt(ProbeSpeed, EEProbeSpeed);
}


void configProbeBackHeight() {
        ProbeBackHeight = EEPROM.read(EEProbeBackHeight);
        ProbeBackHeight = (byte)TFTConfigValue("Probe Rise", 0, 100, ProbeBackHeight, 1, "mm");
        if (SERIAL_DEBUG) { Serial.println("CONFIG : ProbeBackHeight : " + String(ProbeBackHeight)); }
        EEPROM.write(EEProbeBackHeight, ProbeBackHeight);
        EEPROM.commit();
}

void configProbeTime() {
        ProbeTime = EEPROM.read(EEProbeTime);
        ProbeTime = (byte)TFTConfigValue("Probe Time", 1, 20, ProbeTime, 1, "sec");
        if (SERIAL_DEBUG) { Serial.println("CONFIG : ProbeTime : " + String(ProbeTime)); }
        EEPROM.write(EEProbeTime, ProbeTime);
        EEPROM.commit();
}

void configSleep() {
        SleepTime = EEPROM.read(EESleepTime);
        SleepTime = (byte)TFTConfigValue("Sleep Time", 0, 20, SleepTime, 1, "min");
        if (SERIAL_DEBUG) { Serial.println("CONFIG : SleepTime : " + String(SleepTime)); }
        EEPROM.write(EESleepTime, SleepTime);
        EEPROM.commit();
        SleepTicker.interval(SleepTime * 60000);
        if (SleepTime == 0) { SleepTicker.stop(); }
}

void configBrightness() {
        TFT_BRIGHTNESS = EEPROM.read(EEBrightness);
        TFT_BRIGHTNESS = (byte)TFTConfigValue("Brightness", 0, 255, TFT_BRIGHTNESS, 1, "");
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
                "(c) 2021",
                "Paul Schwaderer",
                "github.com/agent",
                "-r/bCNC-PENDANT",
                "",
                "Back"
        };
        TFTConfigInfo(Content, ContentNum);
}
