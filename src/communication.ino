
///////////////////////////////////////////////////////////////////////////
///////////////////////      COMMUNICATION       //////////////////////////
///////////////////////////////////////////////////////////////////////////

void ConnectionSetup() {
        // bool connected;

        if (SERIAL_DEBUG) { Serial.println("Setting up Connection..."); }

        switch(ConnectionMode) {
        case 0:                         // WIFI SLAVE
                btSerial.end();
                WiFi.mode(WIFI_STA);
                break;
        case 1:                         // WIFI AP
                btSerial.end();
                WiFi.mode(WIFI_AP);
                break;
        case 2:                         // BLUETOOTH
                WiFi.disconnect();
                WiFi.mode(WIFI_OFF);
                btSerial.begin("PENDANT", true); // MASTER
                if (SERIAL_DEBUG) { Serial.println("... via Bluetooth"); }
                // btSerial.begin("PENDANT");  // SLAVE
                break;
        }
}


bool Connect() {

        switch (ConnectionMode) {
        case 0:
                if (ConnectWifi()) {
                        return(ConnectTCP(WifiHost, WifiPort));
                }
                break;
        case 1:
                if (ConnectWifiAP()) {
                        return(ConnectTCP(APHost, APPort));
                }
                break;
        case 2:
                return(ConnectBT());
                break;
        }
        return(false);
}


bool ConnectWifiAP() {

        if ((!WiFi.softAP(APSSID.c_str(), APPW.c_str())) || (WiFi.softAPgetStationNum() != 1)) {

                BlinkTicker.start();

                while (!WiFi.softAP(APSSID.c_str(), APPW.c_str())) {
                        delay(100);
                        if (SERIAL_DEBUG) {Serial.println("WIFI-AP: RESTART AP !");}
                        BlinkTicker.update(); SleepTicker.update();
                        if (blinker_change) {
                                if (blinker) { TFTPrint(StateField, "Starting AP..", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "Config?", TFT_COLOR_MSG_ERR); }
                                else { TFTPrint(StateField, "", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "", TFT_COLOR_MSG_ERR);  }
                                blinker_change = false;
                        }
                        if (checkConfig()) { config(); return(false); }  // Start config routine
                }
                while (WiFi.softAPgetStationNum() != 1) {
                        delay(100);
                        if (SERIAL_DEBUG) {Serial.println("WIFI-AP: WRONG NUMBER OF CLIENTS !");}
                        BlinkTicker.update(); SleepTicker.update();
                        if (blinker_change) {
                                if (blinker) { TFTPrint(StateField, "Waiting for Guest..", TFT_COLOR_STA_ERR); }
                                else { TFTPrint(StateField, "Config??", TFT_COLOR_STA_ERR); }
                                blinker_change = false;
                        }
                        if (checkConfig()) { config(); return(false); }  // Start config routine
                }
                BlinkTicker.stop();
                TFTPrint(StateField, "", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "", TFT_COLOR_MSG_ERR);
        }
        return(true);
}


bool ConnectWifi() {
        if (WiFi.status() != WL_CONNECTED) {

                WiFi.disconnect();
                char ssid_char[50];
                WifiSSID.toCharArray(ssid_char, sizeof(ssid_char));
                char password_char[50];
                WifiPW.toCharArray(password_char, sizeof(password_char));
                WiFi.begin(ssid_char, password_char);

                BlinkTicker.start();
                while (WiFi.status() != WL_CONNECTED) {
                        if (SERIAL_DEBUG) {Serial.println("WIFI CONNECTING ...");}
                        BlinkTicker.update(); SleepTicker.update();
                        if (blinker_change) {
                                if (blinker) { TFTPrint(StateField, "WIFI Connecting..", TFT_COLOR_STA_ERR); }
                                else { TFTPrint(StateField, "Config??", TFT_COLOR_STA_ERR); }
                                blinker_change = false;
                        }
                        delay(100);
                        if (checkConfig()) { config(); return(false); }  // Start config routine
                }
                BlinkTicker.stop();
                TFTPrint(StateField, "", TFT_COLOR_STA_ERR); TFTPrint(MessageField, "", TFT_COLOR_MSG_ERR);
        }
        return(true);
}


bool ConnectTCP(IPAddress tcpip, int tcpport) {

        while (!TCPClient.connected()) {

                TCPClient.connect(tcpip, tcpport);
                // TCPClient.setNoDelay(true);
                if (SERIAL_DEBUG) { Serial.println("TCP (RE)CONNECTING ..."); }
                BlinkTicker.update(); SleepTicker.update();

                if (blinker_change) {
                        if (blinker) { TFTPrint(MessageField, "TCP Connecting..", TFT_COLOR_STA_ERR);}
                        else { TFTPrint(StateField, "Config??", TFT_COLOR_STA_ERR); }
                        blinker_change = false;
                }
                delay(100);
                if (checkConfig()) { config(); return(false); }              // Start config routine
        }
        return(true);
}


bool ConnectBT() {

/*
        btSerial.setPin(String(BluetoothPin).c_str());
        bool success = btSerial.connect(BluetoothHost);

        if(btSerial.connect(BluetoothHost)) {
                Serial.println("Connected by Address Succesfully!");
                return(true);
        }

        if(btSerial.connect("GRBLHAL")) {
                Serial.println("Connected by SSID Succesfully!");
                return(true);
        }

        while(!btSerial.connected(1000)) {
                Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app.");
        }
        if (btSerial.disconnect()) {
                Serial.println("Disconnected Succesfully!");
        }
        // this would reconnect to the name(will use address, if resolved) or address used with connect(name/address).
        success = btSerial.connect();
        return(success);
   }
 */

        while(!btSerial.connected(1000)) {  // was 0
                TFTPrint(MessageField, "Starting BLUETOOTH..", TFT_COLOR_STA_ERR);
                SleepTicker.update();
                if (checkConfig()) { config(); return(false); }                      // Start config routine
                if (SERIAL_DEBUG) { Serial.println("connecting Bluetooth ... Device is available?"); }
                btSerial.setPin(String(BluetoothPin).c_str());
                btSerial.connect(BluetoothHost);
                delay(50);
        }
        return(true);

}

// BLUETOOTH HC-05 MODULE AT-SETTINGS:
/*
   AT+UART=115200,0,0
   AT+NAME=GRBLHAL
   AT+ROLE=0          // SLAVE
   AT+PSWD="1234"     // 1234
   AT+ADDR?           // 21:13:13C6F = 00:21:13:01:3C:6F -> CNC
                      // 21:13:12CC9 = 00:21:13:01:2C:C9 -> TEST 002113013C6F
 */

void getState() {

        static char TCPBuffer[128];
        // static bool buffer_active = false;
        static int buffer_idx = 0;
        char buffer_char;

        if (Connect()) { //Check WiFi connection status

                switch (ConnectionMode) {
                case 0:
                case 1:

                        // TCPClient.connect(WifiHost, WifiPort);

                        while (TCPClient.available() > 0) {
                                if (TCPClient.available() > 200) {
                                        if (SERIAL_DEBUG_IN) { Serial.println("WARNING: INBUFFER OVERFLOW");}
                                        while(TCPClient.available()) {
                                                TCPClient.read();
                                        }
                                        if (SERIAL_DEBUG_IN) { Serial.println("WARNING: OVERFLOW DELETED");}
                                        return;
                                }

                                buffer_char = TCPClient.read();

                                if (buffer_char == '{') {
                                        buffer_idx = 0;
                                        TCPBuffer[buffer_idx] = buffer_char;
                                        buffer_idx++;
                                }
                                else if (buffer_char == '}') {
                                        TCPBuffer[buffer_idx] = buffer_char;
                                        TCPBuffer[buffer_idx + 1] =  '\0';
                                        buffer_idx = 0;
                                        // if (SERIAL_DEBUG_IN) {Serial.println(String(TCPBuffer));}
                                        deserialize(String(TCPBuffer));
                                }
                                else {
                                        TCPBuffer[buffer_idx] = buffer_char;
                                        buffer_idx++;
                                }
                        }
                // TCPClient.stop();

                // Bluetooth
                case 2:
                        while (btSerial.available() > 0) {
                                if (btSerial.available() > 400) {
                                        while(btSerial.available()) {
                                                btSerial.read();
                                        }
                                        if (SERIAL_DEBUG_IN) { Serial.println("WARNING: INBUFFER OVERFLOW"); }
                                }
                                buffer_char = btSerial.read();

                                if (buffer_char == '{') {
                                        buffer_idx = 0;
                                        TCPBuffer[buffer_idx] = buffer_char;
                                        buffer_idx++;
                                }
                                else if (buffer_char == '}') {
                                        TCPBuffer[buffer_idx] = buffer_char;
                                        TCPBuffer[buffer_idx + 1] =  '\0';
                                        buffer_idx = 0;
                                        // if (SERIAL_DEBUG_IN) {Serial.println(String(TCPBuffer));}
                                        deserialize(String(TCPBuffer));
                                }
                                else {
                                        TCPBuffer[buffer_idx] = buffer_char;
                                        buffer_idx++;
                                }
                        }
                default: break;
                }

        }
}


void deserialize(String JsonString) {

        DynamicJsonDocument JsonIn(128);
        DeserializationError error = deserializeJson(JsonIn, JsonString);

        if (error) {
                if (SERIAL_DEBUG_IN) {Serial.print("WARNING: DESERIALIZATION ERROR: ");}
                if (SERIAL_DEBUG_IN) {Serial.println(error.f_str());}
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
                        if (state == "Alarm") { Probe_Alarm = true; }
                        if (state == "Run") { SleepTicker.start(); }
                        hold = false;
                        HoldTicker.start();
                }

                if (SERIAL_DEBUG_IN) {Serial.println("WX:" + String(wx) + "  WY:" + String(wy) + "  WZ:" + String(wz) + "  WA:" + String(wa) + "  STATE:" + state);}
        }
}

void sendCmd(const String type, const String cmd, const String cmd_info) {
        if (Connect() && !hold)
        {
                // TCPClient.connect(WifiHost, WifiPort);

                switch(ConnectionMode) {
                case 0:
                case 1:
                        TCPClient.println("{\"" + type + "\":\"" + cmd + "\"}");
                        break;
                case 2:
                        btSerial.println("{\"" + type + "\":\"" + cmd + "\"}");
                        break;
                }
                if (SERIAL_DEBUG) {Serial.println("{\"" + type + "\":\"" + cmd + "\"}");}
                TFTPrint(MessageField, cmd_info, TFT_COLOR_MSG_NRM);

                // TCPClient.stop();

        }
        else {
                TFTPrint(MessageField, "NO CONNECTION. HOLD!", TFT_COLOR_MSG_ERR);
        }
}


void Hold() {
        hold = true;
        if (SERIAL_DEBUG) {Serial.println("WARNING: HOLD!");}
        HoldTicker.stop();
}
