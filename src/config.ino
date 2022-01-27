
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
        WiFi.disconnect();
 #define MainContentNum 8
        String MainContent[MainContentNum] = {
                "Config",
                "WiFi",
                "Jogging",
                "Probe",
                "Sleep Mode",
                "Brightness",
                "Info",
                "Exit"
        };
        while(true) {
                switch (TFTConfigMenu(MainContent, MainContentNum)) {
                case 1: configWIFI(); break;
                case 2: configJogging(); break;
                case 3: configProbe(); break;
                case 4: configSleep(); break;
                case 5: configBrightness(); break;
                case 6: configInfo(); break;
                case 7: TFTPrepare(); return;
                default: TFTPrepare(); return;
                }
        }
}

void configWIFI() {
 #define WifiContentNum 7
        String WifiContent[WifiContentNum] = {
                "WiFi",
                "SSID",
                "Password",
                "IP",
                "Port",
                "Access Point",
                "Back"
        };
        while(true) {
                switch (TFTConfigMenu(WifiContent, WifiContentNum)) {
                case 1: configSSID(); break;
                case 2: configPW(); break;
                case 3: configIP(); break;
                case 4: configPort(); break;
                case 5: configAP(); break;
                case 6: return;
                default: return;
                }
        }
}

void configSSID() {
 #define MaxSSIDs 10
        String SSIDs[MaxSSIDs+1];
        int numberOfNetworks = WiFi.scanNetworks();
        if (numberOfNetworks > MaxSSIDs) { numberOfNetworks = MaxSSIDs; }
        int Selection;

        SSIDs[0] = "SSID";
        for(int i = 1; i <= numberOfNetworks; i++) {
                // SSIDs[i] = WiFi.SSID(i-1); // 18?
                SSIDs[i] = WiFi.SSID(i-1); // .substring(0, 15); // 18?
        }
        SSIDs[numberOfNetworks + 1] = "Back";
        for (int i = numberOfNetworks + 2; i <= MaxSSIDs; i++) {
                SSIDs[i] = "";
        }

        Selection = TFTConfigMenu(SSIDs, numberOfNetworks + 2);

        if (Selection <= numberOfNetworks) {
                ssid = WiFi.SSID(Selection -1);
                while (ssid.length() < 50) {
                        ssid = ssid + " ";
                }
                for (int i = 0; i < 50; i++) {
                        EEPROM.write(i + 4, ssid.charAt(i));
                }
                EEPROM.commit(); return;
        }
        else if (Selection == numberOfNetworks+1) {
                return;
        }
}

void configPW() {
        int activeChar = 32;
        String newPassword = "";

        TFTConfigPrepare();
        TFTConfigPrint(0, "Password", TFT_COLOR_CNF_STD);

        TFTSetFontSize(2); tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
        for (int h = 0; h < 10; h++) {  // ROWS
                for (int i = 0; i < 10; i++) { // COLS
                        tft.setCursor(25 + (20 * i), 67 + (20 * h));
                        if ((33 + (10*h) + i) <= 126) { tft.print(char(33 + (10*h) + i)); }
                }
                tft.setCursor(25, 267);
                tft.print("SPACE");
                tft.setCursor(115, 267);
                tft.print("DEL");
                tft.setCursor(175, 267);
                tft.print("SET");
        }

        while(true) {

                Config_Read_Btn();
                delay(20);

                if (rot_clicks != 0) {

                        if(rot_clicks > 0) { activeChar++; }
                        if(rot_clicks < 0) { activeChar--; }
                        // activeChar = activeChar + rot_clicks;

                        rot_clicks = 0;
                        if (activeChar < 33) {activeChar = 129;}
                        if (activeChar > 129) {activeChar = 33;}

                        if ((activeChar >= 33) && (activeChar <= 126)) {

                                if (activeChar < 126) {
                                        tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                        tft.setCursor(25 + (20 * ((activeChar-32) % 10)), 67 + (20 * ((activeChar - 32) / 10)));
                                        tft.print(char(activeChar + 1));
                                }

                                if (activeChar > 33) {
                                        tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                        tft.setCursor(25 + (20 * ((activeChar-34) % 10)), 67 + (20 * ((activeChar - 34) / 10)));
                                        tft.print(char(activeChar - 1));
                                }

                                if (activeChar == 33) {
                                        tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                        tft.setCursor(175, 267);
                                        tft.print("SET");
                                }

                                if (activeChar == 126) {
                                        tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                        tft.setCursor(25, 267);
                                        tft.print("SPACE");
                                }

                                tft.setCursor(25 + (20 * ((activeChar-33) % 10)), 67 + (20 * ((activeChar - 33) / 10)));
                                tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                                tft.print(char(activeChar));
                        }
                        else if (activeChar == 127) {
                                tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                                tft.setCursor(25, 267);
                                tft.print("SPACE");

                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.setCursor(85, 247);
                                tft.print("~");
                                tft.setCursor(115, 267);
                                tft.print("DEL");
                        }
                        else if (activeChar == 128) {
                                tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                                tft.setCursor(115, 267);
                                tft.print("DEL");

                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.setCursor(25, 267);
                                tft.print("SPACE");
                                tft.setCursor(175, 267);
                                tft.print("SET");
                        }
                        else if (activeChar == 129) {
                                tft.setTextColor(TFT_COLOR_MSG_ERR, TFT_COLOR_FRM_BGR);
                                tft.setCursor(175, 267);
                                tft.print("SET");

                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.setCursor(115, 267);
                                tft.print("DEL");
                                tft.setCursor(25, 67);
                                tft.print("!");
                        }
                }

                if (configClick == true) {
                        configClick = false;
                        if (activeChar == 129) {
                                password = newPassword;
                                while (newPassword.length() < 50) {
                                        newPassword = newPassword + " ";
                                }
                                for (int i = 0; i < 50; i++) {
                                        EEPROM.write(i + 54, newPassword.charAt(i));
                                }
                                EEPROM.commit();
                                break;
                        }
                        else if (activeChar == 128) {
                                if (password.length() > 0) {
                                        newPassword = newPassword.substring(0, (newPassword.length() - 1));
                                }
                        }
                        else if (activeChar == 127) {
                                newPassword = newPassword + " ";
                        }
                        else {
                                newPassword = newPassword + char(activeChar);
                        }

                        tft.fillRect(10, 285, 220, 25, TFT_COLOR_FRM_BGR);
                        tft.setCursor(25,287); tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        tft.print(newPassword);
                }
        }
}

void configIP() {

        byte newip[4] = {0, 0, 0, 0};
        newip[0] = EEPROM.read(0);
        newip[1] = EEPROM.read(1);
        newip[2] = EEPROM.read(2);
        newip[3] = EEPROM.read(3);
        byte activeip = 0;

        TFTConfigPrepare();
        TFTConfigPrint(0, "IP", TFT_COLOR_CNF_STD);
        TFTSetFontSize(2);
        tft.setCursor(ConfigFields[2][0]+7, ConfigFields[2][1]+4);

        for (int i = 0; i < activeip; i++) {
                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                tft.print(newip[i]); tft.print(".");
        }
        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
        tft.print(newip[activeip]);
        for (int i = (activeip + 1); i < 4; i++) {
                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                tft.print("."); tft.print(newip[i]);
        }


        while (true) {

                Config_Read_Btn();
                delay(20);

                if (configClick == true) {
                        configClick = false;
                        activeip++;
                        if (activeip >= 4) {break;}
                        tft.fillRect(ConfigFields[2][0], ConfigFields[2][1], ConfigFields[2][2], ConfigFields[2][3], TFT_COLOR_FRM_BGR);
                        tft.setCursor(ConfigFields[2][0] + 7, ConfigFields[2][1] + 4);
                        for (int i = 0; i < activeip; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(newip[i]); tft.print(".");
                        }
                        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        tft.print(newip[activeip]);
                        for (int i = (activeip + 1); i < 4; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print("."); tft.print(newip[i]);
                        }
                }

                if (rot_clicks != 0) {
                        newip[activeip] = newip[activeip] + rot_clicks;
                        rot_clicks = 0;
                        host[0] = newip[0];  host[1] = newip[1]; host[2] = newip[2]; host[3] = newip[3];
                        tft.fillRect(ConfigFields[2][0], ConfigFields[2][1], ConfigFields[2][2], ConfigFields[2][3], TFT_COLOR_FRM_BGR);
                        tft.setCursor(ConfigFields[2][0] + 7, ConfigFields[2][1] + 4);

                        for (int i = 0; i < activeip; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(newip[i]); tft.print(".");
                        }

                        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        tft.print(newip[activeip]);

                        for (int i = (activeip + 1); i < 4; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print("."); tft.print(newip[i]);
                        }

                }
        }

        host[0] = newip[0];  host[1] = newip[1]; host[2] = newip[2]; host[3] = newip[3];

        EEPROM.write(0, newip[0]);
        EEPROM.write(1, newip[1]);
        EEPROM.write(2, newip[2]);
        EEPROM.write(3, newip[3]);
        EEPROM.commit();
}

void configPort() {

        port = (EEPROM.read(114) << 8) + EEPROM.read(115);
        port = TFTConfigValue("Port", 0, 9999, port, 1, "");
        EEPROM.write(114, port >> 8);
        EEPROM.write(115, port & 0xFF);
        EEPROM.commit();
}

void configAP() {

 #define APContentNum 7
        String APContent[APContentNum] = {
                "AccessPoint",
                "AP On/Off",
                "",
                "SSID: bCNC-Pend",
                "PW: bCNC!Pend",
                "",
                "Back"
        };

        APOn = EEPROM.read(112);
        if (APOn >= 1) { APContent[1] = "AP On"; }
        else { APContent[1] = "AP Off"; }

        while(true) {
                switch (TFTConfigMenu(APContent, APContentNum)) {
                case 1: if (APOn == 0) { APOn = 1; }
                        else { APOn = 0; }
                        EEPROM.write(112, APOn);
                        EEPROM.commit();
                        if (APOn >= 1) { APContent[1] = "AP On"; }
                        else { APContent[1] = "AP Off"; }
                        WiFi.disconnect();
                        // TFTConfigPrint(1, "  " + APContent[1], TFT_COLOR_CNF_STD);
                        break;
                case 2: break;
                case 3: break;
                case 4: break;
                case 5: break;
                case 6: return;
                default: return;
                }
        }
}


void configSleep() {
        SleepTime = EEPROM.read(104);
        SleepTime = (byte)TFTConfigValue("Sleep Time", 0, 20, SleepTime, 1, "min");
        EEPROM.write(104, SleepTime);
        EEPROM.commit();
}


void configBrightness() {
        TFT_BRIGHTNESS = EEPROM.read(113);
        TFT_BRIGHTNESS = (byte)TFTConfigValue("Brightness", 0, 255, TFT_BRIGHTNESS, 1, "");
        EEPROM.write(113, TFT_BRIGHTNESS);
        EEPROM.commit();
        analogWrite(TFT_LED, TFT_BRIGHTNESS);
}


void configJogging() {
 #define JoggingContentNum 6
        String JoggingContent[JoggingContentNum] = {
                "Jogging",
                "X Speed",
                "Y Speed",
                "Z Speed",
                "A Speed",
                "Back",
        };
        while(true) {
                switch (TFTConfigMenu(JoggingContent, JoggingContentNum)) {
                case 1: configJoggingX(); break;
                case 2: configJoggingY(); break;
                case 3: configJoggingZ(); break;
                case 4: configJoggingA(); break;
                case 5: return;
                default: return;
                }
        }
}

void configJoggingX() {
        JogSpeed[1] = (EEPROM.read(116) << 8) + EEPROM.read(117);
        JogSpeed[1] = (int)TFTConfigValue("X Jogging", 0, 10000, JogSpeed[1], 100, "mm/min");
        EEPROM.write(116, JogSpeed[1] >> 8);
        EEPROM.write(117, JogSpeed[1] & 0xFF);
        EEPROM.commit();
}
void configJoggingY() {
        JogSpeed[2] = (EEPROM.read(118) << 8) + EEPROM.read(119);
        JogSpeed[2] = (int)TFTConfigValue("Y Jogging", 0, 10000, JogSpeed[2], 100, "mm/min");
        EEPROM.write(118, JogSpeed[2] >> 8);
        EEPROM.write(119, JogSpeed[2] & 0xFF);
        EEPROM.commit();
}
void configJoggingZ() {
        JogSpeed[3] = (EEPROM.read(120) << 8) + EEPROM.read(121);
        JogSpeed[3] = (int)TFTConfigValue("Z Jogging", 0, 10000, JogSpeed[3], 100, "mm/min");
        EEPROM.write(120, JogSpeed[3] >> 8);
        EEPROM.write(121, JogSpeed[3] & 0xFF);
        EEPROM.commit();
}
void configJoggingA() {
        JogSpeed[4] = (EEPROM.read(122) << 8) + EEPROM.read(123);
        JogSpeed[4] = (int)TFTConfigValue("A Jogging", 0, 10000, JogSpeed[4], 100, "mm/min");
        EEPROM.write(122, JogSpeed[4] >> 8);
        EEPROM.write(123, JogSpeed[4] & 0xFF);
        EEPROM.commit();
}

void configProbe() {
 #define ProbeContentNum 6
        String ProbeContent[ProbeContentNum] = {
                "Probe",
                "Offset",
                "Depth",
                "Speed",
                "Return Height",
                "Back"
        };

        while(true) {
                switch (TFTConfigMenu(ProbeContent, ProbeContentNum)) {
                case 1: configProbeOffset(); break;
                case 2: configProbeDepth(); break;
                case 3: configProbeSpeed(); break;
                case 4: configProbeBackHeight(); break;
                case 5: return;
                default: return;
                }
        }
}




void configProbeOffset() {
        ProbeOffset = (EEPROM.read(105) << 8) + EEPROM.read(106);
        float ProbeOffsetFloat = ProbeOffset / 100;
        ProbeOffsetFloat = TFTConfigValue("Offset", 0, 50, ProbeOffsetFloat, 0.01, "mm");
        ProbeOffset = ProbeOffsetFloat * 100;
        EEPROM.write(105, ProbeOffset >> 8);
        EEPROM.write(106, ProbeOffset & 0xFF);
        EEPROM.commit();
}


void configProbeDepth() {
        ProbeDepth = (EEPROM.read(107) << 8) + EEPROM.read(108);
        if (ProbeDepth > 32767) { ProbeDepth = ProbeDepth - 65536; }  // 65535
        ProbeDepth = (int)TFTConfigValue("Depth", -100, 100, ProbeDepth, 1, "mm");
        EEPROM.write(107, ProbeDepth >> 8);
        EEPROM.write(108, ProbeDepth & 0xFF);
        EEPROM.commit();
}


void configProbeSpeed() {
        ProbeSpeed = (EEPROM.read(109) << 8) + EEPROM.read(110);
        ProbeSpeed = (int)TFTConfigValue("Speed", 0, 1000, ProbeSpeed, 1, "mm / min");
        EEPROM.write(109, ProbeSpeed >> 8);
        EEPROM.write(110, ProbeSpeed & 0xFF);
        EEPROM.commit();
}


void configProbeBackHeight() {
        ProbeBackHeight = EEPROM.read(111);
        ProbeBackHeight = (byte)TFTConfigValue("Rise", 0, 100, ProbeBackHeight, 1, "mm");
        EEPROM.write(111, ProbeBackHeight);
        EEPROM.commit();
}

void configInfo() {
 #define InfoContentNum 7
        String InfoContent[InfoContentNum] = {
                "Info",
                "(c) 2021",
                "Paul Schwaderer",
                "github.com/agent",
                "-r/bCNC-PENDANT",
                "",
                "Back"
        };
        TFTConfigPrepare();
        for (int i = 0; i < InfoContentNum; i++) {
                TFTConfigPrint(i, InfoContent[i], TFT_COLOR_CNF_STD);
        }
        while(true) {
                delay(20);
                Config_Read_Btn();
                if (configClick == true) { configClick = false; return; }
        }
}

void Config_Read_Btn() {
        btn_current2 = btn_current1;
        btn_current1 = (digitalRead(BUTTON_PIN) == LOW);
        if (btn_current1 == btn_current2) {
                if ((btn_current1 != btn_last) && (btn_current1 == true)) { configClick = true; }
                btn_last = btn_current1;
        }
}



void TFTConfigPrepare() {
        tft.fillRect(0,0,240,320,TFT_COLOR_BGR);
        tft.drawRect(ConfigForms[0][0]-1,ConfigForms[0][1]-1,ConfigForms[0][2]+2,ConfigForms[0][3]+2,TFT_COLOR_FRM_LIN);
        tft.fillRect(ConfigForms[0][0],ConfigForms[0][1],ConfigForms[0][2],ConfigForms[0][3], TFT_COLOR_FRM_BGR);
        tft.drawRect(ConfigForms[1][0]-1,ConfigForms[1][1]-1,ConfigForms[1][2]+2,ConfigForms[1][3]+2,TFT_COLOR_FRM_LIN);
        tft.fillRect(ConfigForms[1][0],ConfigForms[1][1],ConfigForms[1][2],ConfigForms[1][3], TFT_COLOR_FRM_BGR);
}


byte TFTConfigMenu(String* Content, int Length) {

        byte activeMenu = 1;
        TFTConfigPrepare();
        TFTConfigPrint(0, Content[0], TFT_COLOR_CNF_STD);
        TFTConfigPrint(1, "> " + Content[1], TFT_COLOR_CNF_STD);

        for (int i = 2; i < Length; i++) {
                TFTConfigPrint(i, "  " + Content[i], TFT_COLOR_CNF_STD);
        }

        while(true) {
                Config_Read_Btn();
                delay(20);
                if (rot_clicks != 0) {
                        TFTConfigPrint(activeMenu, "  " + Content[activeMenu], TFT_COLOR_CNF_STD);
                        activeMenu = activeMenu + rot_clicks;
                        if (activeMenu > (Length - 1)) {activeMenu = 1;}
                        if (activeMenu < 1) {activeMenu = (Length - 1);}
                        TFTConfigPrint(activeMenu, "> " + Content[activeMenu], TFT_COLOR_CNF_STD);
                        rot_clicks = 0;
                }

                if (configClick == true) {
                        configClick = false;
                        return(activeMenu);
                }
        }
}


float TFTConfigValue(String Name, int Min, int Max, float Value, float Factor, String Unit) {

        TFTConfigPrepare();
        TFTConfigPrint(0, Name, TFT_COLOR_CNF_STD);
        if (Factor == (int)Factor) { TFTConfigPrint(2, " " + String((int)Value) + " " + Unit, TFT_COLOR_CNF_STD); }
        else { TFTConfigPrint(2, " " + String(Value) + " " + Unit, TFT_COLOR_CNF_STD); }

        while(true) {
                Config_Read_Btn();
                delay(20);
                if (rot_clicks != 0) {
                        Value = Value + (rot_clicks * Factor);
                        if (Value > Max) {Value = Min;}
                        if (Value < Min) {Value = Max;}

                        if (Factor == (int)Factor) { TFTConfigPrint(2, " " + String((int)Value) + " " + Unit, TFT_COLOR_CNF_STD); }
                        else { TFTConfigPrint(2, " " + String(Value) + " " + Unit, TFT_COLOR_CNF_STD); }
                        rot_clicks = 0;
                }

                if (configClick == true) {
                        configClick = false;
                        return(Value);
                }
        }

}



void TFTConfigPrint(byte Aim, String Content, int Color) {
        Content = Content.substring(0,17);
        tft.fillRect(ConfigFields[Aim][0],ConfigFields[Aim][1],ConfigFields[Aim][2],ConfigFields[Aim][3], TFT_COLOR_FRM_BGR);
        TFTSetFontSize(ConfigFields[Aim][4]);
        tft.setTextColor(Color, TFT_COLOR_FRM_BGR);
        tft.setCursor(ConfigFields[Aim][0]+7, ConfigFields[Aim][1]+4); // was 7/4
        tft.print(Content);
}
