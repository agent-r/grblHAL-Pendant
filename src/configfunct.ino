
/////////////////////// GENERAL CONFIG FUNCTIONS /////////////////////////////

void TFTConfigPrepare() {
        tft.fillRect(0,0,240,320,TFT_COLOR_BGR);
        tft.drawRect(ConfigForms[0][0]-1,ConfigForms[0][1]-1,ConfigForms[0][2]+2,ConfigForms[0][3]+2,TFT_COLOR_FRM_LIN);
        tft.fillRect(ConfigForms[0][0],ConfigForms[0][1],ConfigForms[0][2],ConfigForms[0][3], TFT_COLOR_FRM_BGR);
        tft.drawRect(ConfigForms[1][0]-1,ConfigForms[1][1]-1,ConfigForms[1][2]+2,ConfigForms[1][3]+2,TFT_COLOR_FRM_LIN);
        tft.fillRect(ConfigForms[1][0],ConfigForms[1][1],ConfigForms[1][2],ConfigForms[1][3], TFT_COLOR_FRM_BGR);
}


void TFTConfigPrint(const byte Aim, String Content, const int Color) {
        Content = Content.substring(0,20);
        tft.fillRect(ConfigFields[Aim][0],ConfigFields[Aim][1],ConfigFields[Aim][2],ConfigFields[Aim][3], TFT_COLOR_FRM_BGR);
        TFTSetFontSize(ConfigFields[Aim][4]);
        tft.setTextColor(Color, TFT_COLOR_FRM_BGR);
        tft.setCursor(ConfigFields[Aim][0]+7, ConfigFields[Aim][1]+4); // was 7/4
        tft.print(Content);
}


byte TFTConfigMenu(const String* Content, const int Length) {

        byte activeMenu = 1;
        TFTConfigPrepare();
        TFTConfigPrint(0, Content[0], TFT_COLOR_CNF_STD);
        TFTConfigPrint(1, "> " + Content[1], TFT_COLOR_CNF_STD);

        for (int i = 2; i < Length; i++) {
                TFTConfigPrint(i, "  " + Content[i], TFT_COLOR_CNF_STD);
        }

        while(true) {
                delay(20);
                if (rotaryEncoder.encoderChanged()) {
                        TFTConfigPrint(activeMenu, "  " + Content[activeMenu], TFT_COLOR_CNF_STD);
                        activeMenu = activeMenu + rotaryEncoder.readEncoder();
                        if (activeMenu > (Length - 1)) {activeMenu = 1;}
                        if (activeMenu < 1) {activeMenu = (Length - 1);}
                        TFTConfigPrint(activeMenu, "> " + Content[activeMenu], TFT_COLOR_CNF_STD);
                        rotaryEncoder.reset();
                }

                if (checkEnter()) {
                        return(activeMenu);
                }
        }
}


float TFTConfigValue(const String Title, const int Min, const int Max, float Value, const float Factor, const String Unit) {

        TFTConfigPrepare();
        TFTConfigPrint(0, Title, TFT_COLOR_CNF_STD);
        if (Factor == (int)Factor) { TFTConfigPrint(2, " " + String((int)Value) + " " + Unit, TFT_COLOR_CNF_STD); }
        else { TFTConfigPrint(2, " " + String(Value) + " " + Unit, TFT_COLOR_CNF_STD); }

        while(true) {
                delay(20);
                if (rotaryEncoder.encoderChanged()) {
                        Value = Value + (rotaryEncoder.readEncoder() * Factor);
                        if (Value > Max) {Value = Min;}
                        if (Value < Min) {Value = Max;}

                        if (Factor == (int)Factor) { TFTConfigPrint(2, " " + String((int)Value) + " " + Unit, TFT_COLOR_CNF_STD); }
                        else { TFTConfigPrint(2, " " + String(Value) + " " + Unit, TFT_COLOR_CNF_STD); }
                        rotaryEncoder.reset();
                }

                if (checkEnter()) {
                        return(Value);
                }
        }

}

String TFTConfigString(const String Title, const String oldString) {

        String newString = oldString;
        int activeChar = 32;
        TFTConfigPrepare();
        TFTConfigPrint(0, Title, TFT_COLOR_CNF_STD);
        TFTSetFontSize(2); tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);

        for (int h = 0; h < 10; h++) { // ROWS
                for (int i = 0; i < 10; i++) { // COLS
                        tft.setCursor(25 + (20 * i), 67 + (20 * h));
                        if ((33 + (10*h) + i) <= 126) { tft.print(char(33 + (10*h) + i)); }
                }
        }

        tft.setCursor(105, 247);
        tft.print("SPACE");
        tft.setCursor(175, 247);
        tft.print("<DEL");
        tft.setCursor(25, 267);
        tft.print("SAVE");
        tft.setCursor(175, 267);
        tft.print("BACK");
        tft.setCursor(25,287); tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
        tft.print(oldString);

        while(true) {
                delay(20);
                if (rotaryEncoder.encoderChanged()) {

                        int change = rotaryEncoder.readEncoder();
                        rotaryEncoder.reset();
                        if (change > 0) { activeChar++; }
                        else if (change < 0) { activeChar--; }
                        // activeChar = activeChar + rotaryEncoder.readEncoder();

                        if (activeChar < 33) {activeChar = 130;}
                        if (activeChar > 130) {activeChar = 33;}

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
                                        tft.print("BACK");
                                }

                                if (activeChar == 126) {
                                        tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                        tft.setCursor(105, 247);
                                        tft.print("SPACE");
                                }

                                tft.setCursor(25 + (20 * ((activeChar-33) % 10)), 67 + (20 * ((activeChar - 33) / 10)));
                                tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                                tft.print(char(activeChar));
                        }
                        else if (activeChar == 127) {
                                tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                                tft.setCursor(105, 247);
                                tft.print("SPACE");

                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.setCursor(85, 247);
                                tft.print("~");
                                tft.setCursor(175, 247);
                                tft.print("<DEL");
                        }
                        else if (activeChar == 128) {
                                tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                                tft.setCursor(175, 247);
                                tft.print("<DEL");

                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.setCursor(105, 247);
                                tft.print("SPACE");
                                tft.setCursor(25, 267);
                                tft.print("SAVE");
                        }
                        else if (activeChar == 129) {
                                tft.setTextColor(TFT_COLOR_MSG_ERR, TFT_COLOR_FRM_BGR);
                                tft.setCursor(25, 267);
                                tft.print("SAVE");

                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.setCursor(175, 247);
                                tft.print("<DEL");
                                tft.setCursor(175, 267);
                                tft.print("BACK");
                        }
                        else if (activeChar == 130) {
                                tft.setTextColor(TFT_COLOR_MSG_ERR, TFT_COLOR_FRM_BGR);
                                tft.setCursor(175, 267);
                                tft.print("BACK");

                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.setCursor(25, 267);
                                tft.print("SAVE");
                                tft.setCursor(25, 67);
                                tft.print("!");
                        }
                }

                if (checkEnter()) {
                        switch (activeChar) {
                        case 127:
                                newString = newString + " ";
                                break;
                        case 128:
                                if (newString.length() > 0) {
                                        newString = newString.substring(0, (newString.length() - 1));
                                }
                                break;
                        case 129:
                                return(newString);
                                break;
                        case 130:
                                return(oldString);
                                break;
                        default:
                                newString = newString + char(activeChar);
                        }
                        tft.fillRect(10, 285, 220, 25, TFT_COLOR_FRM_BGR);
                        tft.setCursor(25,287); tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        tft.print(newString);
                }
        }
}


IPAddress TFTConfigIP (const String Title, IPAddress newIP) {

        byte activeByte = 0;

        TFTConfigPrepare();
        TFTConfigPrint(0, Title, TFT_COLOR_CNF_STD);
        TFTSetFontSize(2);
        tft.setCursor(ConfigFields[2][0]+7, ConfigFields[2][1]+4);

        for (int i = 0; i < activeByte; i++) {
                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                tft.print(newIP[i]); tft.print(".");
        }
        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
        tft.print(newIP[activeByte]);
        for (int i = (activeByte + 1); i < 4; i++) {
                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                tft.print("."); tft.print(newIP[i]);
        }

        while (true) {
                delay(20);

                if (checkEnter()) {
                        activeByte++;

                        if (activeByte >= 4) { return(newIP); }

                        tft.fillRect(ConfigFields[2][0], ConfigFields[2][1], ConfigFields[2][2], ConfigFields[2][3], TFT_COLOR_FRM_BGR);
                        tft.setCursor(ConfigFields[2][0] + 7, ConfigFields[2][1] + 4);
                        for (int i = 0; i < activeByte; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(newIP[i]); tft.print(".");
                        }
                        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        tft.print(newIP[activeByte]);
                        for (int i = (activeByte + 1); i < 4; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print("."); tft.print(newIP[i]);
                        }
                }

                if (rotaryEncoder.encoderChanged()) {
                        newIP[activeByte] = newIP[activeByte] + rotaryEncoder.readEncoder();
                        rotaryEncoder.reset();
                        WifiHost[0] = newIP[0];  WifiHost[1] = newIP[1]; WifiHost[2] = newIP[2]; WifiHost[3] = newIP[3];
                        tft.fillRect(ConfigFields[2][0], ConfigFields[2][1], ConfigFields[2][2], ConfigFields[2][3], TFT_COLOR_FRM_BGR);
                        tft.setCursor(ConfigFields[2][0] + 7, ConfigFields[2][1] + 4);

                        for (int i = 0; i < activeByte; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print(newIP[i]); tft.print(".");
                        }

                        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        tft.print(newIP[activeByte]);

                        for (int i = (activeByte + 1); i < 4; i++) {
                                tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                                tft.print("."); tft.print(newIP[i]);
                        }
                }
        }
}


void TFTConfigInfo(const String* Content, const int Length) {
        TFTConfigPrepare();
        for (int i = 0; i < Length; i++) {
                TFTConfigPrint(i, Content[i], TFT_COLOR_CNF_STD);
        }
        while(!checkEnter()) {
                delay(20);
        }
}
