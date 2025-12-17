
/////////////////////// GENERAL CONFIG FUNCTIONS /////////////////////////////

#include <Arduino.h>
#include "configfunct.h"
#include "configmenu.h"
#include "display/display.h"
#include "controls/controls.h"
#include "global.h"


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
        tft.setCursor(ConfigFields[Aim][0]+7, ConfigFields[Aim][1]+4);
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
                int count = readEncoder(); // rotaryEncoder.readEncoder();
                if (count != 0) {
                        TFTConfigPrint(activeMenu, "  " + Content[activeMenu], TFT_COLOR_CNF_STD);
                        activeMenu = activeMenu + count;
                        if (activeMenu > (Length - 1)) {activeMenu = 1;}
                        if (activeMenu < 1) {activeMenu = (Length - 1);}
                        TFTConfigPrint(activeMenu, "> " + Content[activeMenu], TFT_COLOR_CNF_STD);
                        resetEncoder(); // rotaryEncoder.reset();
                }

                if (checkEnter()) {
                        return(activeMenu);
                }
        }
}


float TFTConfigValue(const String Title, const int Min, const int Max, float Value, const int Factor, const char* Unit, const byte leadingZeros) {
        // Factor -2=100, -1=10, 0=1, 1=0.1 2=0.01 etc.
        TFTConfigPrepare();
        TFTConfigPrint(0, Title, TFT_COLOR_CNF_STD);
        char strValue[20];
        int Decimals = Factor* -1; if (Decimals < 0) { Decimals = 0; }
        sprintf(strValue, "%0*.*f %s", leadingZeros, Decimals, Value, Unit);
        TFTConfigPrint(2, String(strValue), TFT_COLOR_CNF_STD);

        while (true) {
                delay(20);
                int count = readEncoder(); // rotaryEncoder.readEncoder();
                if (count != 0) {
                        Value = Value + (count * pow(10, Factor));
                        if (Value > Max) {Value = Min;}
                        if (Value < Min) {Value = Max;}
                        sprintf(strValue, "%0*.*f %s", leadingZeros, Decimals, Value, Unit);
                        TFTConfigPrint(2, String(strValue), TFT_COLOR_CNF_STD);
                        resetEncoder(); // rotaryEncoder.reset();
                }

                if (checkEnter()) {
                        return(Value);
                }
        }
}

#define COLWIDTH 18
#define COLNUM 11
#define ROWHEIGHT 19
#define ROWNUM 9

String TFTConfigString(const String Title, const String oldString) {

        String newString = oldString;
        int activeChar = 33;
        int oldChar = 33;
        TFTConfigPrepare();
        TFTConfigPrint(0, Title, TFT_COLOR_CNF_STD);
        TFTSetFontSize(2); tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);

        for (int i = 33; i <= 130; i++) {
                tft.setCursor(TFTConfigStringCharPosX(i), TFTConfigStringCharPosY(i));
                tft.print(TFTConfigStringChar(i));
        }
        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
        tft.setCursor(25,265);
        tft.print(newString.substring(0, 18));
        tft.setCursor(25,284);
        tft.print(newString.substring(18, 36));

        do {
                delay(20);
                int count = readEncoder(); // rotaryEncoder.readEncoder();
                if (count != 0) {

                        oldChar = activeChar;
                        activeChar = activeChar + count;
                        resetEncoder(); // rotaryEncoder.reset();

                        if (activeChar < 33) {activeChar = 130;}
                        if (activeChar > 130) {activeChar = 33;}

                        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        tft.setCursor(TFTConfigStringCharPosX(activeChar), TFTConfigStringCharPosY(activeChar));
                        tft.print(TFTConfigStringChar(activeChar));
                        tft.setTextColor(TFT_COLOR_CNF_STD, TFT_COLOR_FRM_BGR);
                        tft.setCursor(TFTConfigStringCharPosX(oldChar), TFTConfigStringCharPosY(oldChar));
                        tft.print(TFTConfigStringChar(oldChar));

                }

                if (checkEnter()) {
                        switch (activeChar) {
                        case 127:
                                if (newString.length() < 36) {
                                        newString = newString + " ";
                                }
                                break;
                        case 128:
                                if (newString.length() > 0) {
                                        newString = newString.substring(0, (newString.length() - 1));
                                }
                                break;
                        case 129:
                                return(oldString);
                                break;
                        case 130:
                                return(newString);
                                break;
                        default:
                                if (newString.length() < 36) {
                                        newString = newString + char(activeChar);
                                }
                        }
                        tft.fillRect(10, 265, 220, 45, TFT_COLOR_FRM_BGR);
                        tft.setTextColor(TFT_COLOR_CNF_HIL, TFT_COLOR_FRM_BGR);
                        tft.setCursor(25,265);
                        tft.print(newString.substring(0, 18));
                        tft.setCursor(25,284);
                        tft.print(newString.substring(18, 36));
                }
        } while(true);
}

String TFTConfigStringChar(int Position) {
        if (Position < 33) { Position = 130; }
        if (Position > 130) { Position = 33; }
        if (Position <= 126) { return(String(char(Position))); }
        else if (Position == 127) { return("SPACE"); }
        else if (Position == 128) { return("<DEL"); }
        else if (Position == 129) { return("BACK"); }
        else if (Position == 130) { return("SAVE"); }
}

int TFTConfigStringCharPosX(int Position) {
        if (Position < 33) { Position = 130; }
        if (Position > 130) { Position = 33; }
        if (Position <= 126) { return(25 + (COLWIDTH * ((Position-33) % COLNUM))); }
        else if (Position == 127) { return(25+(COLWIDTH*8)); }
        else if (Position == 128) { return(25+(COLWIDTH*0)); }
        else if (Position == 129) { return(25+(COLWIDTH*4)); }
        else if (Position == 130) { return(25+(COLWIDTH*8)); }
}

int TFTConfigStringCharPosY(int Position) {
        if (Position < 33) { Position = 130; }
        if (Position > 130) { Position = 33; }
        if (Position <= 126) { return(67 + (ROWHEIGHT * ((Position - 33) / COLNUM))); }
        else if (Position == 127) { return(67 + (ROWHEIGHT*8)); }
        else if (Position == 128) { return(67 + (ROWHEIGHT*9)); }
        else if (Position == 129) { return(67 + (ROWHEIGHT*9)); }
        else if (Position == 130) { return(67 + (ROWHEIGHT*9)); }
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
