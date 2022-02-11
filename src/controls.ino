
///////////////////////////////////////////////////////////////////////////
/////////////////////      CONTROLS      //////////////////////////////////
///////////////////////////////////////////////////////////////////////////

/*
   void checkEncoder() {
        if (rotaryEncoder.encoderChanged()) {
                sendCmd("gcode", "$J=G91" + AxisName[activeAxis] + (factor[activeFactor] * rotaryEncoder.readEncoder() * AxisDir[activeAxis]) + "F" + JogSpeed[activeAxis], "JOG " + AxisName[activeAxis] + (factor[activeFactor] * rotaryEncoder.readEncoder() * AxisDir[activeAxis]));
                rotaryEncoder.reset();
                if (SleepTime > 0) {SleepTicker.start();}
        }
   }
 */

void checkEncoder() {
        int count = rotaryEncoder.getCount();
        if (count != 0) {
                sendCmd("gcode", "$J=G91" + AxisName[activeAxis] + (factor[activeFactor] * count * AxisDir[activeAxis]) + "F" + JogSpeed[activeAxis], "JOG " + AxisName[activeAxis] + (factor[activeFactor] * count * AxisDir[activeAxis]));
                rotaryEncoder.clearCount();
                if (SleepTime > 0) {SleepTicker.start();}
        }
}


/////////////////////////////////////////////////////////////////


void checkKeypad() {

        keypad_old = keypad_current;
        keypad_current = readKeypad();

        if (keypad_current != keypad_old) {
                if (SERIAL_DEBUG) {Serial.println("KEYPAD PRESS: " + String(keypad_current));}
                switch (keypad_current) {
                case -1: break;                  // no button pressed !
                case 0: enter(); break;                      // setX()
                case 1: unlock(); break;                      // setY
                case 2: stop(); break;                      // setZ
                case 3: homeAll(); break;                  // gotoZero()
                case 4: increaseFactor(); break;                    // probeZ()
                case 5: decreaseFactor(); break;                    // config()
                case 6: config(); break;            // decreaseFactor()
                case 7: probeZ(); break;            // increaseFactor()
                case 8: gotoZero(); break;                   // homeAll()
                case 9: setZero(); break;                       // run()
                case 10: increaseAxis(); break;                     // stop()
                case 11: decreaseAxis(); break;                   // unlock()
                }
        }
}


int readKeypad() {
        int analogValue = analogRead(KEYPAD_PIN);
        if (analogValue < 450) { return(-1); }

        if (SleepTime > 0 ) {SleepTicker.start();}

        if (analogValue < 1800) { return(0); }
        else if (analogValue < 1890) { return(1); } // was 1900
        else if (analogValue < 2000) { return(2); }
        else if (analogValue < 2150) { return(3); }
        else if (analogValue < 2300) { return(4); }
        else if (analogValue < 2450) { return(5); }
        else if (analogValue < 2600) { return(6); }
        else if (analogValue < 2800) { return(7); }
        else if (analogValue < 3100) { return(8); }
        else if (analogValue < 3400) { return(9); }
        else if (analogValue < 3850) { return(10); }
        else { return(11); }
}


bool checkEnter() {
        if (readKeypad() == 0) {
                while (readKeypad() != -1) { delay(50);}
                return(true);
        }
        else {
                return(false);
        }
}

bool checkEnterConfirm() {
        for (int i = 0; i < 50; i++) {
                if (checkEnter()) { return(true); }
                TftTicker.update();
                delay(40);
        }
        return(false);
}

bool checkConfig() {
        if (readKeypad() == 6) {
                while (readKeypad() != -1) { delay(50);}
                return(true);
        }
        else {
                return(false);
        }
}

////////////////////////    BATTERY    ////////////////////////


void checkBattery() {
        float battery_state = readBattery();
        int battery_percentage = percentageBattery(battery_state);
        String batteryMessage = "Battery: " + String(battery_state) + "V / " + String(battery_percentage) + "%";
        if (battery_percentage < 25) { TFTPrint(MessageField, batteryMessage, TFT_COLOR_MSG_ERR); }
        else { TFTPrint(MessageField, batteryMessage, TFT_COLOR_MSG_NRM); }
}

float readBattery() {
        int batteryValue = analogRead(BATTERY_PIN);
        float batteryVoltage = (batteryValue * BATTERY_FACTOR);
        return(batteryVoltage);
}

int percentageBattery(const float Voltage) {
        int Percentage = ((Voltage - BATTERY_LOW) * 100) / (BATTERY_HIGH - BATTERY_LOW);
        return(Percentage);
}
