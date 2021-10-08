
///////////////////////////////////////////////////////////////////////////
///////////////////////       DISPLAY       ///////////////////////////////
///////////////////////////////////////////////////////////////////////////

const int Fields[6][5] = {
        {10, 25, 220, 35, 3},  // Field X
        {10, 65, 220, 35, 3}, // Field Y
        {10, 105, 220, 35, 3}, // Field Z
        {10, 160, 220, 35, 3}, // Field F
        {10, 235, 220, 28, 2},       // Field Status
        {10, 273, 220, 28, 2}        // Field Message
};

void TFTUpdate() {

        getState();

        if (wxchange || axischange) {
                char strwx[7];
                dtostrf(wx,7,2,strwx);
                if (activeAxis == 1) { TFTPrint(XField, "> X:" + String(strwx), TFT_COLOR_XYZ); tft.drawRect(Fields[0][0],Fields[0][1],Fields[0][2],Fields[0][3],TFT_COLOR_FRM_LIN); }
                else { TFTPrint(XField, "  X:" + String(strwx), TFT_COLOR_XYZ_INACTIVE);}
                wxchange = false;
        }
        if (wychange || axischange) {
                char strwy[7];
                dtostrf(wy,7,2,strwy);
                if (activeAxis == 2) { TFTPrint(YField, "> Y:" + String(strwy), TFT_COLOR_XYZ); tft.drawRect(Fields[1][0],Fields[1][1],Fields[1][2],Fields[1][3],TFT_COLOR_FRM_LIN); }
                else { TFTPrint(YField, "  Y:" + String(strwy), TFT_COLOR_XYZ_INACTIVE); }
                wychange = false;
        }
        if (wzchange || axischange) {
                char strwz[7];
                dtostrf(wz,7,2,strwz);
                if (activeAxis == 3) { TFTPrint(ZField, "> Z:" + String(strwz), TFT_COLOR_XYZ); tft.drawRect(Fields[2][0],Fields[2][1],Fields[2][2],Fields[2][3],TFT_COLOR_FRM_LIN); }
                else { TFTPrint(ZField, "  Z:" + String(strwz), TFT_COLOR_XYZ_INACTIVE); }
                wzchange = false;
                axischange = false;
        }
        if (factorchange) {
                TFTPrint(FField, "  F: " + strFactor[activeFactor], TFT_COLOR_XYZ_INACTIVE);
                factorchange = false;
        }
        if (statechange) {
                if (state == "Run") { TFTPrint(StateField, state, TFT_COLOR_STA_NRM); SleepCounter = 0;}
                else if (state == "Idle") { TFTPrint(StateField, state, TFT_COLOR_STA_NRM); }
                else if (state == "Hold:0") { TFTPrint(StateField, state, TFT_COLOR_STA_NRM); }
                else if (state == "Hold:1") { TFTPrint(StateField, state, TFT_COLOR_STA_NRM); }
                else if (state == "Alarm") { TFTPrint(StateField, state, TFT_COLOR_STA_ERR); }
                else { TFTPrint(StateField, state, TFT_COLOR_STA_ERR); }
                statechange = false;
        }

        // if (StatusCounter == MessageTime) { tft.fillRect(Fields[4][0],Fields[4][1],Fields[4][2],Fields[4][3], TFT_COLOR_FRM_BGR); }
        // StatusCounter++;
        if (MessageCounter == MessageTime) { tft.fillRect(Fields[5][0],Fields[5][1],Fields[5][2],Fields[5][3], TFT_COLOR_FRM_BGR); }
        MessageCounter++;
}


void TFTSleep() {
        if (SleepCounter >= SleepTime) {
                // Serial.println("SLEEP !");
                // detachInterrupt(digitalPinToInterrupt(ROTARY_PIN_CLK));
                // detachInterrupt(digitalPinToInterrupt(ROTARY_PIN_DT));
                analogWrite(TFT_LED, 0);
                tft.fillRect(0,0,240,320, ILI9341_BLACK);
                while (configClick == false && rot_clicks == 0) {
                        Config_Read_Btn();
                        delay(50);
                }
                // attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_CLK), rotate, CHANGE);
                // attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_DT), rotate, CHANGE);
                wxchange = true; wychange = true; wzchange = true, statechange = true;
                TFTPrepare();
                analogWrite(TFT_LED, 255);
                delay(200);
                configClick = false;
                rot_clicks = 0;
                SleepCounter = 0;
        }
        else { SleepCounter++; }
        // Serial.println(SleepCounter);
}


void TFTPrint(byte Aim, String Content, int Color) {

        Content = Content.substring(0,17);

        tft.fillRect(Fields[Aim][0],Fields[Aim][1],Fields[Aim][2],Fields[Aim][3], TFT_COLOR_FRM_BGR);
        tft.setTextSize(Fields[Aim][4]);
        tft.setTextColor(Color);
        tft.setCursor(Fields[Aim][0]+7, Fields[Aim][1]+7);
        tft.print(Content);

        // if (Aim == 4) { StatusCounter = 0; }
        if (Aim == 5) { MessageCounter = 0; }

}

void TFTPrepare() {
        tft.fillRect(0,0,240,320,TFT_COLOR_BGR);
        tft.drawRect(Fields[0][0]-1,Fields[0][1]-1,Fields[0][2]+2,Fields[0][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawRect(Fields[1][0]-1,Fields[1][1]-1,Fields[1][2]+2,Fields[1][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawRect(Fields[2][0]-1,Fields[2][1]-1,Fields[2][2]+2,Fields[2][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawRect(Fields[3][0]-1,Fields[3][1]-1,Fields[3][2]+2,Fields[3][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawRect(Fields[4][0]-1,Fields[4][1]-1,Fields[4][2]+2,Fields[4][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawRect(Fields[5][0]-1,Fields[5][1]-1,Fields[5][2]+2,Fields[5][3]+2,TFT_COLOR_FRM_LIN);
        tft.drawFastHLine(0,215,240,TFT_COLOR_FRM_LIN);
        TFTClear();
}

void TFTClear() {
        tft.fillRect(Fields[0][0],Fields[0][1],Fields[0][2],Fields[0][3],TFT_COLOR_FRM_BGR);
        tft.fillRect(Fields[1][0],Fields[1][1],Fields[1][2],Fields[1][3],TFT_COLOR_FRM_BGR);
        tft.fillRect(Fields[2][0],Fields[2][1],Fields[2][2],Fields[2][3],TFT_COLOR_FRM_BGR);
        tft.fillRect(Fields[3][0],Fields[3][1],Fields[3][2],Fields[3][3],TFT_COLOR_FRM_BGR);
        tft.fillRect(Fields[4][0],Fields[4][1],Fields[4][2],Fields[4][3],TFT_COLOR_FRM_BGR);
        tft.fillRect(Fields[5][0],Fields[5][1],Fields[5][2],Fields[5][3],TFT_COLOR_FRM_BGR);
        wxchange = true; wychange = true; wzchange = true; factorchange = true; statechange = true;
}
