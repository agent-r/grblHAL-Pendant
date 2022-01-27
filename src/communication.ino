
///////////////////////////////////////////////////////////////////////////
///////////////////////      COMMUNICATION       //////////////////////////
///////////////////////////////////////////////////////////////////////////


bool WifiConnect() {

        bool Blinker = true;

        if (APOn >= 1) {
                if ((!WiFi.softAP("bCNC-Pend", "bCNC!Pend")) || (WiFi.softAPgetStationNum() != 1)) {
                        while (!WiFi.softAP("bCNC-Pend", "bCNC!Pend")) {
                                delay(500);
                                if (SERIAL_DEBUG) {Serial.println("WIFI-AP: RESTART AP !");}
                                if (Blinker) { TFTPrint(StateField, "Starting AP..", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "Config? Button!", TFT_COLOR_MSG_ERR); }
                                else { TFTPrint(StateField, "", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "", TFT_COLOR_MSG_ERR);  }
                                Blinker = !Blinker;
                        }
                        while (WiFi.softAPgetStationNum() != 1) {
                                delay(500);
                                if (SERIAL_DEBUG) {Serial.println("WIFI-AP: WRONG NUMBER OF CLIENTS !");}
                                if (Blinker) { TFTPrint(StateField, "Waiting for Guest..", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "Config? Button!", TFT_COLOR_MSG_ERR); }
                                else { TFTPrint(StateField, "", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "", TFT_COLOR_MSG_ERR);  }
                                Blinker = !Blinker;
                        }
                        TFTPrint(StateField, "", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "", TFT_COLOR_MSG_ERR);
                }
        }
        else {
                if (WiFi.status() != WL_CONNECTED) {

                        WiFi.disconnect();
                        char ssid_char[50];
                        ssid.toCharArray(ssid_char, sizeof(ssid_char));
                        char password_char[50];
                        password.toCharArray(password_char, sizeof(password_char));
                        WiFi.begin(ssid_char, password_char);

                        while (WiFi.status() != WL_CONNECTED) {
                                if (SERIAL_DEBUG) {Serial.println("WIFI CONNECTING ...");}
                                delay(500);
                                if (Blinker) { TFTPrint(StateField, "WIFI Connecting..", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "Config? Button!", TFT_COLOR_MSG_ERR); }
                                else { TFTPrint(StateField, "", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "", TFT_COLOR_MSG_ERR);  }
                                Blinker = !Blinker;

                                if (digitalRead(BUTTON_PIN) == LOW) {
                                        while (digitalRead(BUTTON_PIN) == LOW) {delay(20);}
                                        config(); // Start config routine
                                        return(false);
                                }

                        }
                        TFTPrint(StateField, "", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "", TFT_COLOR_MSG_ERR);
                }

        }

        // CHECK TCP !
        while(!TCPClient.connected()) {
                TCPClient.connect(gethost(), port);
                if (SERIAL_DEBUG) { Serial.println("TCP (RE)CONNECTING ..."); }
                // TCPClient.setNoDelay(true);
        }

        return(true);
}


IPAddress gethost() {
        if (APOn >= 1) { return APhost; }
        else { return host; }
}




////  TESTCODE:     {"state":"TESTSTATE*1","wx":1.000000,"wy":2.000000,"wz":3.000000,"wa":360.00}
/// 7.000000}*{"state":"TESTSTATE*1","wx":2.000000,"wy":3.000000,"wz":4.000000}?{"state":"TESTSTATE*1","wx":5.000000,"wy":6.000000,"wz":
void getState() {

        if (WifiConnect()) { //Check WiFi connection status

                while (TCPClient.available() > 0)
                {

                        if(buffer_idx > 126) {
                                buffer_idx = 0;
                                buffer_active = false;
                        }
                        buffer_char = TCPClient.read();

                        if (buffer_char == '{') {
                                buffer_active = true;
                                buffer_idx = 0;
                                TCPBuffer[buffer_idx] = buffer_char;
                                buffer_idx++;
                        }
                        else if (buffer_active && (buffer_char == '}')) {

                                buffer_active = false;
                                TCPBuffer[buffer_idx] = buffer_char;
                                TCPBuffer[buffer_idx + 1] =  '\0';
                                buffer_idx = 0;
                                if (SERIAL_DEBUG) {Serial.println(String(TCPBuffer));}

                                DeserializationError error = deserializeJson(JsonIn, String(TCPBuffer));

                                if (error) {
                                        if (SERIAL_DEBUG) {Serial.print("DESERIALIZATION ERROR: ");}
                                        if (SERIAL_DEBUG) {Serial.println(error.f_str());}
                                }
                                else {
                                        if (JsonIn.containsKey("wx")) {
                                                wx = JsonIn["wx"];
                                                if (wx != wxold) { wxchange = true; wxold = wx; }
                                        }
                                        if (JsonIn.containsKey("wy")) {
                                                wy = JsonIn["wy"];
                                                if (wy != wyold) { wychange = true; wyold = wy; }
                                        }
                                        if (JsonIn.containsKey("wz")) {
                                                wz = JsonIn["wz"];
                                                if (wz != wzold) { wzchange = true; wzold = wz; }
                                        }
                                        if (JsonIn.containsKey("wa")) {
                                                wa = JsonIn["wa"];
                                                if (wa != waold) { wachange = true; waold = wa; }
                                        }
                                        if (JsonIn.containsKey("state")) {
                                                const char* statechar = JsonIn["state"];
                                                state = String(statechar);
                                                if (state != stateold) { statechange = true; stateold = state; }
                                        }

                                        if (SERIAL_DEBUG) {Serial.println("WX:" + String(wx) + "  WY:" + String(wy) + "  WZ:" + String(wz) + "  WA:" + String(wa) + "  STATE:" + state);}
                                }

                        }
                        else if (buffer_active) {
                                TCPBuffer[buffer_idx] = buffer_char;
                                buffer_idx++;
                        }
                }
        }
}


void sendCmd(String type, String cmd, String cmd_info) {
        if (WifiConnect())
        {
                TCPClient.println("{\"" + type + "\":\"" + cmd + "\"}");
                if (SERIAL_DEBUG) {Serial.println("{\"" + type + "\":\"" + cmd + "\"}");}
                TFTPrint(MessageField, cmd_info, TFT_COLOR_MSG_NRM);
        }
}
