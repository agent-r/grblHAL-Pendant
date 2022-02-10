
///////////////////////////////////////////////////////////////////////////
///////////////////////      COMMANDS       ///////////////////////////////
///////////////////////////////////////////////////////////////////////////


void decreaseAxis() {
        if (activeAxis <= 0) { activeAxis = 3; }
        else { activeAxis--; }
        axischange = true;
}

void increaseAxis() {
        if (activeAxis >= 3) { activeAxis = 0; }
        else { activeAxis++; }
        axischange = true;
}

void setZero() {
        sendCmd("gcode", "G10L20P1" + AxisName[activeAxis] + "0", "Set " + AxisName[activeAxis] + "0");
}

void gotoZero() {
        sendCmd("gcode", "$J=G90" + AxisName[activeAxis] + "0F" + JogSpeed[activeAxis], "GOTO " + AxisName[activeAxis] + "0");
}

void probeZ() {

        bool run_check = false;

        Probe_Alarm = false;
        TFTPrint(MessageField, "Confirm Z PROBE ?", TFT_COLOR_MSG_ERR);
        if (checkEnterConfirm()) {

                sendCmd("gcode", "G91", ""); delay(50);
                sendCmd("gcode", "G38.2Z" + String(ProbeDepth) + "F" + String(ProbeSpeed), "PROBE: GO DOWN"); delay(100);


                for (int i = 0; i < (ProbeTime * 50); i++) {

                        TftTicker.update();
                        if (state == "Run") { run_check = true; }
                        if ((state == "Idle") && run_check) { break; }

                        if (Probe_Alarm) {
                                Serial.println("PROBE: ALARM");
                                break;
                        }
                        delay(20);
                }

                delay(100);     // was 200
                sendCmd("gcode", "G90", "");
                delay(100);     // was 200
                if (!Probe_Alarm) {
                        sendCmd("gcode", "G10L20Z" + String(ProbeOffset/100.0), "PROBE: OK");
                        delay(100);     // was 200
                        sendCmd("gcode", "$J=G91Z" + String(ProbeBackHeight) + "F" + String(JogSpeed[2]), "PROBE: OK");
                }
                else {
                        sendCmd("gcode", "$J=G91Z" + String(ProbeDepth* -1) + "F" + String(JogSpeed[2]), "");
                        TFTPrint(MessageField, "PROBE: ALARM", TFT_COLOR_MSG_ERR);
                }
        }
}


void homeAll() {
        TFTPrint(MessageField, "Confirm HOME ?", TFT_COLOR_MSG_ERR);
        // for (int i = 0; i < 50; i++) {
        if (checkEnterConfirm()) {
                sendCmd("gcode", "$H", "HOME ALL");
                return;
        }
        // TftTicker.update();
        // delay(50);
        // }
        TFTPrint(MessageField, "", TFT_COLOR_MSG_NRM);
}

void stop() {
        sendCmd("cmd", "STOP", "STOP");
}

void unlock() {
        sendCmd("cmd", "UNLOCK", "UNLOCK");
}
void increaseFactor(){
        if (activeFactor >= 4) {activeFactor = 1;}
        else {activeFactor++;}
        factorchange = true;
}
void decreaseFactor(){
        if (activeFactor <= 0) {activeFactor = 4;}
        else {activeFactor--;}
        factorchange = true;
}

/*
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
 */

void enter() {
        // do something like START // RESUME ??
}
