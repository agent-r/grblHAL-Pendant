
///////////////////////////////////////////////////////////////////////////
///////////////////////      COMMANDS       ///////////////////////////////
///////////////////////////////////////////////////////////////////////////


void setX() {
        activeAxis = 1;
        axischange = true;
}
void setY() {
        activeAxis = 2;
        axischange = true;
}
void setZ() {
        activeAxis = 3;
        axischange = true;
}
void setA() {
        activeAxis = 4;
        axischange = true;
}
void gotoZero() {
        if ((activeAxis >= 1) && (activeAxis <= 4)) {
                sendCmd("gcode", "$J=G90" + AxisName[activeAxis] + "0F" + JogSpeed[activeAxis], "GOTO " + AxisName[activeAxis] + "0");
        }
        else { TFTPrint(MessageField, "No Axis selected !", TFT_COLOR_MSG_NRM); }
}
void probeZ() {
        TFTPrint(MessageField, "Confirm Z PROBE ?", TFT_COLOR_MSG_ERR);
        for (int i = 0; i < 50; i++) {
                if (digitalRead(BUTTON_PIN) == LOW) {
                        sendCmd("gcode", "G91", "");
                        sendCmd("gcode", "G38.2Z" + String(ProbeDepth) + "F" + String(ProbeSpeed), "");
                        sendCmd("gcode", "G10L20Z" + String(ProbeOffset/100.0), "");
                        sendCmd("gcode", "G0Z" + String(ProbeBackHeight), "");
                        sendCmd("gcode", "G90", "PROBE");
                        while (digitalRead(BUTTON_PIN) == LOW) {delay(50);}
                        return;
                }
                delay(50);
        }
        TFTPrint(MessageField, "", TFT_COLOR_MSG_NRM);
}


void homeAll() {
        TFTPrint(MessageField, "Confirm HOME ?", TFT_COLOR_MSG_ERR);
        for (int i = 0; i < 50; i++) {
                if (digitalRead(BUTTON_PIN) == LOW) {
                        sendCmd("gcode", "$H", "HOME ALL");
                        while (digitalRead(BUTTON_PIN) == LOW) {delay(50);}
                        return;
                }
                delay(50);
        }
        TFTPrint(MessageField, "", TFT_COLOR_MSG_NRM);
}

void stop() {
        sendCmd("cmd", "STOP", "STOP");
}

void unlock() {
        sendCmd("cmd", "UNLOCK", "UNLOCK");
}
void increaseFactor(){
        if (activeFactor >= 4) {activeFactor = 4;}
        else {activeFactor++;}
        factorchange = true;
}
void decreaseFactor(){
        if (activeFactor <= 0) {activeFactor = 0;}
        else {activeFactor--;}
        factorchange = true;
}

void run() {
        TFTPrint(MessageField, "Confirm RUN ?", TFT_COLOR_MSG_ERR);
        for (int i = 0; i < 50; i++) {
                if (digitalRead(BUTTON_PIN) == LOW) {
                        sendCmd("cmd", "START", "START");
                        while (digitalRead(BUTTON_PIN) == LOW) {delay(50);}
                        return;
                }
                delay(50);
        }
        TFTPrint(MessageField, "", TFT_COLOR_MSG_NRM);
}
