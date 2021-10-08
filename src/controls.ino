
///////////////////////////////////////////////////////////////////////////
/////////////////////      CONTROLS      //////////////////////////////////
///////////////////////////////////////////////////////////////////////////


IRAM_ATTR void rotate() {
        unsigned char result = rotary.process();
        if (result == DIR_CW) {
                rot_clicks++;
        } else if (result == DIR_CCW) {
                rot_clicks--;
        }
        SleepCounter = 0;
}


void SendUpdate() {

        if (rot_clicks != 0) {

                if ((activeAxis == 1) || (activeAxis == 2)) {
                        sendCmd("G", "G91G0" + AxisName[activeAxis] + (factor[activeFactor] * rot_clicks));
                        sendCmd("G", "G90");
                }
                else if (activeAxis == 3) {
                        sendCmd("G", "G91G0" + AxisName[activeAxis] + (factor[activeFactor] * rot_clicks * -1));
                        sendCmd("G", "G90");
                }
                else { TFTPrint(MessageField, "No Axis selected !", TFT_COLOR_MSG_ERR); }
                rot_clicks = 0;
        }

}


/////////////////////////////////////////////////////////////////


void Read_Key_Btn() {

        keypad_current2 = keypad_current1;
        keypad_current1 = readButton();
        if (keypad_current1 == keypad_current2) {
                if (keypad_current1 != keypad_last) {
                        SleepCounter = 0;
                        switch (keypad_current1) {
                        case -1: break;      // no button pressed !
                        case 0: setX(); break;
                        case 1: setY(); break;
                        case 2: setZ(); break;
                        case 3: gotoZero(); break;
                        case 4: probeZ(); break;
                        case 5: config(); break;
                        case 6: decreaseFactor(); break;
                        case 7: increaseFactor(); break;
                        case 8: homeAll(); break;
                        case 9: run(); break;
                        case 10: stop(); break;
                        case 11: reset_unlock(); break;
                        default: break;
                        }
                }
                keypad_last = keypad_current1;
        }

        btn_current2 = btn_current1;
        btn_current1 = (digitalRead(BUTTON_PIN) == LOW);
        if (btn_current1 == btn_current2) {
                if ((btn_current1 != btn_last) && (btn_current1 == true)) {
                        if (activeAxis != 0) { sendCmd("G", "G10L20P1" + AxisName[activeAxis] + "0"); }
                        else { TFTPrint(MessageField, "No Axis selected !", TFT_COLOR_MSG_ERR); }
                        SleepCounter = 0;
                }
                btn_last = btn_current1;
        }
}


int readButton() {
        int analogValue = analogRead(KEYPAD_PIN);
        if (analogValue < 450) {
                return(-1);
        } else if (analogValue < 520) {
                return(0);
        } else if (analogValue < 540) {
                return(1);
        } else if (analogValue < 570) {
                return(2);
        } else if (analogValue < 600) {
                return(3);
        } else if (analogValue < 650) {
                return(4);
        } else if (analogValue < 700) {
                return(5);
        } else if (analogValue < 750) {
                return(6);
        } else if (analogValue < 800) {
                return(7);
        } else if (analogValue < 850) {
                return(8);
        } else if (analogValue < 930) {
                return(9);
        } else if (analogValue < 1000) {
                return(10);
        } else {
                return(11);
        }
}
