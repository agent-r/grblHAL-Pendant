
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
        //-> P1=G54 P2=G55 P3=G56 P4=G57 P5=G58 P6=G59 ... P0=ActiveWorkspace
        // sendCmd("gcode", "G10L20P1" + AxisName[activeAxis] + "0", "Set " + AxisName[activeAxis] + "0");  //-> P1=G54 P2=G55 P3=G56
        sendCmd("gcode", "G10L20P0" + AxisName[activeAxis] + "0", "Set " + AxisName[activeAxis] + "0");
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
                // {"gcode":"G38.2Z-1F100"}
                sendCmd("gcode", "G38.2Z" + String(ProbeDepth) + "F" + String(ProbeSpeed), "PROBE: GO DOWN"); delay(100);

                for (int i = 0; i < (ProbeTime * 50); i++) {
                        // CAN THIS BE DONE WIHT A "DWELL"-COMMAND (= WAIT FOR BUFFER EMPTY) ???
                        TftTicker.update();
                        if (state == "Run") { run_check = true; }
                        if ((state == "Idle") && run_check) { break; }

                        if (Probe_Alarm) {
                                break;
                        }
                        delay(20);
                }

                delay(100);
                sendCmd("gcode", "G90", "");
                delay(100);
                if (!Probe_Alarm) {
                        // sendCmd("gcode", "G10L20Z" + String(ProbeOffset), "PROBE: OK");
                        sendCmd("gcode", "G10L20P0Z" + String(ProbeOffset), "PROBE: OK");
                        delay(100);
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
        if (checkEnterConfirm()) {
                sendCmd("gcode", "$H", "HOME ALL");
                return;
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
        if (activeFactor >= 4) {activeFactor = 1;}
        else {activeFactor++;}
        factorchange = true;
}
void decreaseFactor(){
        if (activeFactor <= 0) {activeFactor = 4;}
        else {activeFactor--;}
        factorchange = true;
}

void enter() {
        sendCmd("cmd", "START", "START");
}

// {"msg":"O"}
