
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
void gotoZero() {
        if ((activeAxis >= 1) && (activeAxis <= 3)) {
                sendCmd("G", "G0" + AxisName[activeAxis] + "0");
        }
        else { TFTPrint(MessageField, "No Axis selected !", TFT_COLOR_MSG_NRM); }
}
void probeZ() {
        TFTPrint(MessageField, "Confirm Z PROBE ?", TFT_COLOR_MSG_ERR);
        for (int i = 0; i < 50; i++) {
                if (digitalRead(BUTTON_PIN) == LOW) {
                        sendCmd("G", "G91G38.2Z" + String(ProbeDepth) + "F" + String(ProbeSpeed));
                        sendCmd("G", "G10L20Z" + String(ProbeOffset/100.0));
                        sendCmd("G", "G0Z" + String(ProbeBackHeight) + "G90");
                        while (digitalRead(BUTTON_PIN) == LOW) {delay(50);}
                        return;
                }
                delay(50);
        }
        TFTPrint(MessageField, "", TFT_COLOR_MSG_NRM);
}
/*
   void homeX() {
        sendCmd("G", "G0X0");
   }
   void homeY() {
        sendCmd("G", "G0Y0");
   }
   void homeZ() {
        sendCmd("G", "G0Z0");
   }
 */
void homeAll() {
        TFTPrint(MessageField, "Confirm HOME ?", TFT_COLOR_MSG_ERR);
        for (int i = 0; i < 50; i++) {
                if (digitalRead(BUTTON_PIN) == LOW) {
                        sendCmd("C", "HOME");
                        while (digitalRead(BUTTON_PIN) == LOW) {delay(50);}
                        return;
                }
                delay(50);
        }
        TFTPrint(MessageField, "", TFT_COLOR_MSG_NRM);
}
void stop() {
        sendCmd("C", "STOP");
}
void reset_unlock() {
        sendCmd("C", "RESET");
        sendCmd("C", "UNLOCK");
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
                        sendCmd("C", "RUN");
                        while (digitalRead(BUTTON_PIN) == LOW) {delay(50);}
                        return;
                }
                delay(50);
        }
        TFTPrint(MessageField, "", TFT_COLOR_MSG_NRM);
}
