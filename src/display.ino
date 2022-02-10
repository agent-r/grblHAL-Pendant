
///////////////////////////////////////////////////////////////////////////
///////////////////////       DISPLAY       ///////////////////////////////
///////////////////////////////////////////////////////////////////////////



const int Fields[7][5] = {
        {10, 20, 220, 35, 3},       // Field X
        {10, 60, 220, 35, 3},       // Field Y
        {10, 100, 220, 35, 3},      // Field Z
        {10, 140, 220, 35, 3},      // Field A
        {10, 190, 220, 35, 3},      // Field F
        {10, 245, 220, 28, 2},      // Field Status
        {10, 278, 220, 28, 2}       // Field Message
};
const int Line[4] = {0,234,240};


void TFTUpdate() {

        getState();

        if (wxchange || axischange) {
                char strwx[7];
                dtostrf(wx,7,2,strwx);
                if (activeAxis == 0) { TFTPrint(XField, "> X: " + String(strwx), TFT_COLOR_XYZ); tft.drawRect(Fields[XField][0],Fields[XField][1],Fields[XField][2],Fields[XField][3],TFT_COLOR_FRM_LIN); }
                else { TFTPrint(XField, "  X: " + String(strwx), TFT_COLOR_XYZ_INACTIVE);}
                wxchange = false;
        }
        if (wychange || axischange) {
                char strwy[7];
                dtostrf(wy,7,2,strwy);
                if (activeAxis == 1) { TFTPrint(YField, "> Y: " + String(strwy), TFT_COLOR_XYZ); tft.drawRect(Fields[YField][0],Fields[YField][1],Fields[YField][2],Fields[YField][3],TFT_COLOR_FRM_LIN); }
                else { TFTPrint(YField, "  Y: " + String(strwy), TFT_COLOR_XYZ_INACTIVE); }
                wychange = false;
        }
        if (wzchange || axischange) {
                char strwz[7];
                dtostrf(wz,7,2,strwz);
                if (activeAxis == 2) { TFTPrint(ZField, "> Z: " + String(strwz), TFT_COLOR_XYZ); tft.drawRect(Fields[ZField][0],Fields[ZField][1],Fields[ZField][2],Fields[ZField][3],TFT_COLOR_FRM_LIN); }
                else { TFTPrint(ZField, "  Z: " + String(strwz), TFT_COLOR_XYZ_INACTIVE); }
                wzchange = false;
        }
        if (wachange || axischange) {
                char strwa[7];
                dtostrf(wa,7,2,strwa);
                if (activeAxis == 3) { TFTPrint(AField, "> A: " + String(strwa), TFT_COLOR_XYZ); tft.drawRect(Fields[AField][0],Fields[AField][1],Fields[AField][2],Fields[AField][3],TFT_COLOR_FRM_LIN); }
                else { TFTPrint(AField, "  A: " + String(strwa), TFT_COLOR_XYZ_INACTIVE); }
                wachange = false;
                axischange = false;
        }
        if (factorchange) {
                TFTPrint(FField, "  F:    " + strFactor[activeFactor], TFT_COLOR_XYZ_INACTIVE);
                factorchange = false;
        }
        if (statechange) {
                if (state == "Run") {
                        TFTPrint(StateField, state, TFT_COLOR_STA_NRM);
                        if (SleepTime > 0 ) {SleepTicker.start();}      // DONT SLEEP !
                }
                else if (state == "Jog") { TFTPrint(StateField, state, TFT_COLOR_STA_NRM); }
                else if (state == "Idle") { TFTPrint(StateField, state, TFT_COLOR_STA_NRM); }
                else if (state == "Hold") { TFTPrint(StateField, state, TFT_COLOR_STA_NRM); }
                else if (state == "Home") { TFTPrint(StateField, state, TFT_COLOR_STA_NRM); }
                else if (state == "Door") { TFTPrint(StateField, state, TFT_COLOR_STA_ERR); }
                else if (state == "Check") { TFTPrint(StateField, state, TFT_COLOR_STA_ERR); }
                else if (state == "Sleep") { TFTPrint(StateField, state, TFT_COLOR_STA_NRM); }
                else if (state == "Tool") { TFTPrint(StateField, state, TFT_COLOR_STA_ERR); }
                else if (state == "Alarm") { TFTPrint(StateField, state, TFT_COLOR_STA_ERR); }
                else if (state == "Endstop") { TFTPrint(StateField, state, TFT_COLOR_STA_ERR); }
                else { TFTPrint(StateField, state, TFT_COLOR_STA_ERR); }
                statechange = false;
        }

}



void TFTSleep() {
        SleepTicker.stop();
        if (SERIAL_DEBUG) {Serial.println("GO TO SLEEP...");}
        // Turn off TFT
        analogWrite(TFT_LED, 0);
        tft.fillRect(0,0,240,320, ILI9341_WHITE);
        // Stop WIFI & Bluetooth
        adc_power_off();
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        esp_wifi_stop();
        btSerial.end();
        esp_bt_controller_disable();
        esp_bt_controller_deinit();
        // Set Wakeup Pins
        bitSet64(SleepPinMask, ROTARY_ENCODER_A_PIN);
        bitSet64(SleepPinMask, ROTARY_ENCODER_B_PIN);
        bitSet64(SleepPinMask, KEYPAD_PIN);
        esp_sleep_enable_ext1_wakeup(SleepPinMask, ESP_EXT1_WAKEUP_ANY_HIGH);
        // GoTo Sleep
        esp_light_sleep_start();
        // esp_deep_sleep_start();  // DOES REBOOT... Why? Normal=
        // WakeUp Routine:
        adc_power_on();
        ConnectionSetup();
        if (SERIAL_DEBUG) { Serial.println("... WAKE UP"); }
        wxchange = true; wychange = true; wzchange = true, wachange = true; statechange = true;
        rotaryEncoder.reset();
        analogWrite(TFT_LED, TFT_BRIGHTNESS);
        TFTPrepare();
        if (SleepTime > 0 ) {SleepTicker.start();}
}


void TFTPrint(const byte Aim, String Content, const int Color) {
        Content = Content.substring(0,20);
        tft.fillRect(Fields[Aim][0],Fields[Aim][1],Fields[Aim][2],Fields[Aim][3], TFT_COLOR_FRM_BGR);
        TFTSetFontSize(Fields[Aim][4]);
        tft.setTextColor(Color, TFT_COLOR_FRM_BGR);
        tft.setCursor(Fields[Aim][0]+7, Fields[Aim][1]+6);
        tft.print(Content);
        if (Aim == MessageField) { MessageTicker.start(); }
}


void TFTSetFontSize(const int size) {
        if (size == 2) {
                tft.unloadFont();
                tft.loadFont(TFT_FONT_SMALL); // Must load the font first
        }
        else if (size == 3) {
                tft.unloadFont();
                tft.loadFont(TFT_FONT_LARGE); // Must load the font first
        }
}


void TFTPrepare() {
        tft.fillRect(0,0,240,320,TFT_COLOR_BGR);
        tft.drawRect(Fields[0][0]-1,Fields[0][1]-1,Fields[0][2]+2,Fields[0][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawRect(Fields[1][0]-1,Fields[1][1]-1,Fields[1][2]+2,Fields[1][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawRect(Fields[2][0]-1,Fields[2][1]-1,Fields[2][2]+2,Fields[2][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawRect(Fields[3][0]-1,Fields[3][1]-1,Fields[3][2]+2,Fields[3][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawRect(Fields[4][0]-1,Fields[4][1]-1,Fields[4][2]+2,Fields[4][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawRect(Fields[5][0]-1,Fields[5][1]-1,Fields[5][2]+2,Fields[5][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawRect(Fields[6][0]-1,Fields[6][1]-1,Fields[6][2]+2,Fields[6][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawFastHLine(Line[0],Line[1],Line[2],TFT_COLOR_FRM_LIN);
        TFTClear();
}

void TFTClear() {
        tft.fillRect(Fields[0][0],Fields[0][1],Fields[0][2],Fields[0][3],TFT_COLOR_FRM_BGR);
        tft.fillRect(Fields[1][0],Fields[1][1],Fields[1][2],Fields[1][3],TFT_COLOR_FRM_BGR);
        tft.fillRect(Fields[2][0],Fields[2][1],Fields[2][2],Fields[2][3],TFT_COLOR_FRM_BGR);
        tft.fillRect(Fields[3][0],Fields[3][1],Fields[3][2],Fields[3][3],TFT_COLOR_FRM_BGR);
        tft.fillRect(Fields[4][0],Fields[4][1],Fields[4][2],Fields[4][3],TFT_COLOR_FRM_BGR);
        tft.fillRect(Fields[5][0],Fields[5][1],Fields[5][2],Fields[5][3],TFT_COLOR_FRM_BGR);
        tft.fillRect(Fields[6][0],Fields[6][1],Fields[6][2],Fields[6][3],TFT_COLOR_FRM_BGR);
        wxchange = true; wychange = true; wzchange = true; wachange = true; factorchange = true; statechange = true;
}


void TFTMessage() {
        tft.fillRect(Fields[MessageField][0],Fields[MessageField][1],Fields[MessageField][2],Fields[MessageField][3], TFT_COLOR_FRM_BGR);
        MessageTicker.stop();
}


void TFTBlink() {
        blinker = !blinker;
        blinker_change = true;
}
