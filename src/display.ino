
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
        if (wachange || axischange) {
                char strwa[7];
                dtostrf(wa,7,2,strwa);
                if (activeAxis == 4) { TFTPrint(ZField, "> A:" + String(strwa), TFT_COLOR_XYZ); tft.drawRect(Fields[3][0],Fields[3][1],Fields[3][2],Fields[3][3],TFT_COLOR_FRM_LIN); }
                else { TFTPrint(AField, "  A:" + String(strwa), TFT_COLOR_XYZ_INACTIVE); }
                wachange = false;
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
                if (SERIAL_DEBUG) {Serial.println("GO TO SLEEP...");}

                // digitalWrite(TFT_LED, LOW);
                analogWrite(TFT_LED, 0);
                tft.fillRect(0,0,240,320, ILI9341_BLACK);
                while (configClick == false && rot_clicks == 0) {
                        Config_Read_Btn();
                        delay(50);
                }

                if (SERIAL_DEBUG) { Serial.println("... WAKE UP"); }
                wxchange = true; wychange = true; wzchange = true, statechange = true;
                TFTPrepare();
                // digitalWrite(TFT_LED, HIGH);
                analogWrite(TFT_LED, TFT_BRIGHTNESS);
                delay(200);
                configClick = false;
                rot_clicks = 0;
                SleepCounter = 0;
        }
        else { SleepCounter++; }
        if (SERIAL_DEBUG) { Serial.println("SLEEP COUNTER: " + String(SleepCounter)); }
}



void TFTPrint(byte Aim, String Content, int Color) {

        Content = Content.substring(0,17);

        tft.fillRect(Fields[Aim][0],Fields[Aim][1],Fields[Aim][2],Fields[Aim][3], TFT_COLOR_FRM_BGR);
        TFTSetFontSize(Fields[Aim][4]);
        tft.setTextColor(Color, TFT_COLOR_FRM_BGR);
        tft.setCursor(Fields[Aim][0]+7, Fields[Aim][1]+6); // was 7/7
        tft.print(Content);

        if (Aim == 5) { MessageCounter = 0; }

}


void TFTSetFontSize(int size) {
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
        wxchange = true; wychange = true; wzchange = true; factorchange = true; statechange = true;
}
