
///////////////////////////////////////////////////////////////////////////
///////////////////////      COMMUNICATION       //////////////////////////
///////////////////////////////////////////////////////////////////////////



bool WifiConnect() {

        bool Blinker = true;

        if (WiFi.status() != WL_CONNECTED) {
                TFTClear();
                TFTPrint(StateField, "WIFI Connecting..", TFT_COLOR_STA_ERR);
                TFTPrint(MessageField, "Config? Button!", TFT_COLOR_MSG_ERR);
                WiFi.disconnect();
                WiFi.begin(ssid, password);
                while (WiFi.status() != WL_CONNECTED) {
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
                TFTClear();
        }
        return(true);
}


void getState() {
        if (WifiConnect()) { //Check WiFi connection status
                WiFiClient client;
                HTTPClient http; //Declare an object of class HTTPClient
                http.setTimeout(httpTimeout);
                http.begin(client, "http://" + host + ":" + httpPort + stateUrl);
                int httpCode = http.GET();
                if (httpCode > 0) { statePayload = http.getString(); http.end(); }
                else { http.end(); HTTPErrorHandler(); }

                deserializeJson(JsonIn, statePayload);
                wz = JsonIn["wz"]; // 0.1
                wy = JsonIn["wy"]; // 0.1
                wx = JsonIn["wx"]; // 0.1
                const char* statechar = JsonIn["state"];
                state = String(statechar);

                if (wx != wxold) { wxchange = true; wxold = wx; }
                if (wy != wyold) { wychange = true; wyold = wy; }
                if (wz != wzold) { wzchange = true; wzold = wz; }
                if (state != stateold) { statechange = true; stateold = state; }
        }
}


void sendCmd(String type, String cmd) {
        if (WifiConnect()) { //Check WiFi connection status
                WiFiClient client;
                HTTPClient http; //Declare an object of class HTTPClient
                http.setTimeout(httpTimeout);

                if (type == "C") { http.begin(client, "http://" + host + ":" + httpPort + cmdUrl + cmd); }
                else if (type == "G") { http.begin(client, "http://" + host + ":" + httpPort + gCodeUrl + cmd); }

                int httpCode = http.GET();
                if (httpCode > 0) { statePayload = http.getString(); }
                else { HTTPErrorHandler(); }
                http.end(); //Close connection

                if ((cmd == "G90") || (cmd == "UNLOCK")) { LastSentCommand = LastSentCommand + " " + cmd; }
                else { LastSentCommand = cmd; }
                TFTPrint(MessageField, LastSentCommand, TFT_COLOR_MSG_NRM);
        }
}


void HTTPErrorHandler() {
        bool Blinker = true;
        TFTClear();
        while (WifiConnect()) {
                WiFiClient client;
                HTTPClient http; //Declare an object of class HTTPClient
                http.setTimeout(httpTimeout);
                http.begin(client, "http://" + host + ":" + httpPort + stateUrl);
                while(http.GET() < 0) {
                        if (Blinker) { TFTPrint(StateField, "Missing HOST!", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "Config? Button!", TFT_COLOR_MSG_ERR); }
                        else { TFTPrint(StateField, "", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "", TFT_COLOR_MSG_ERR); }
                        Blinker = !Blinker;
                        delay(500);
                        if (digitalRead(BUTTON_PIN) == LOW) {
                                while (digitalRead(BUTTON_PIN) == LOW) {delay(20);}
                                config(); // Start config routine
                                http.end();
                                return;
                        }
                }
                http.end();
                TFTClear();
                return;
        }
}
