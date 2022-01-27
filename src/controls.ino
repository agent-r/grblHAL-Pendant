
///////////////////////////////////////////////////////////////////////////
/////////////////////      CONTROLS      //////////////////////////////////
///////////////////////////////////////////////////////////////////////////



// IRAM_ATTR void rotate() {
/*
   void IRAM_ATTR rotate() {
        unsigned char result = rotary.process();
        if (result == DIR_CW) {
                rot_clicks++;
        } else if (result == DIR_CCW) {
                rot_clicks--;
        }
        SleepCounter = 0;
   }
 */


void SendUpdate() {

        // Serial.println("alive");

        if (rotaryEncoder.encoderChanged()) {

                if ((activeAxis == 1) || (activeAxis == 2)) {
                        sendCmd("gcode", "$J=G91" + AxisName[activeAxis] + (factor[activeFactor] * rotaryEncoder.readEncoder()) + "F" + JogSpeed[activeAxis], "JOG " + AxisName[activeAxis] + (factor[activeFactor] * rotaryEncoder.readEncoder()));
                }
                else if (activeAxis == 3) {
                        sendCmd("gcode", "$J=G91" + AxisName[activeAxis] + (factor[activeFactor] * rotaryEncoder.readEncoder() * -1) + "F" + JogSpeed[activeAxis], "JOG " + AxisName[activeAxis] + (factor[activeFactor] * rotaryEncoder.readEncoder()));
                }
                else if (activeAxis == 4) {
                        sendCmd("gcode", "$J=G91" + AxisName[activeAxis] + (factor[activeFactor] * rotaryEncoder.readEncoder()) + "F" + JogSpeed[activeAxis], "JOG " + AxisName[activeAxis] + (factor[activeFactor] * rotaryEncoder.readEncoder()));
                }
                else {
                        TFTPrint(MessageField, "No Axis selected !", TFT_COLOR_MSG_ERR);
                }
                // rot_clicks = 0;
                rotaryEncoder.reset();
                SleepCounter = 0;
        }
}


/////////////////////////////////////////////////////////////////


void Read_Key_Btn() {

        keypad_current2 = keypad_current1;
        keypad_current1 = readButton();

        if (keypad_current1 == keypad_current2) {
                if (keypad_current1 != keypad_last) {
                        if (SERIAL_DEBUG) {Serial.println("KEYPAD PRESS: " + String(keypad_current1));}
                        SleepCounter = 0;
                        switch (keypad_current1) {
                        case -1: break;          // no button pressed !
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
                        case 11: unlock(); break;
                        default: break;
                        }

                }
                keypad_last = keypad_current1;
        }


        btn_current2 = btn_current1;
        btn_current1 = (digitalRead(BUTTON_PIN) == LOW);
        if (btn_current1 == btn_current2) {
                if ((btn_current1 != btn_last) && (btn_current1 == true)) {
                        if (activeAxis != 0) { sendCmd("gcode", "G10L20P1" + AxisName[activeAxis] + "0", "Set " + AxisName[activeAxis] + "0"); }
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
        } else if (analogValue < 1800) {
                return(0);
        } else if (analogValue < 1900) {
                return(1);
        } else if (analogValue < 2000) {
                return(2);
        } else if (analogValue < 2100) {
                return(3);
        } else if (analogValue < 2250) {
                return(4);
        } else if (analogValue < 2450) {
                return(5);
        } else if (analogValue < 2600) {
                return(6);
        } else if (analogValue < 2800) {
                return(7);
        } else if (analogValue < 3100) {
                return(8);
        } else if (analogValue < 3400) {
                return(9);
        } else if (analogValue < 3800) {
                return(10);
        } else {
                return(11);
        }
}
