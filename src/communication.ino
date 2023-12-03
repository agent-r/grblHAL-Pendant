
///////////////////////////////////////////////////////////////////////////
///////////////////////      BLE CALLBACKS       //////////////////////////
///////////////////////////////////////////////////////////////////////////

//When the BLE Server sends a new state reading with the notify property
static void Rx_State_NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
        char* RxState = (char*)pData;
        state = String(RxState);
        state = state.substring(0, state.indexOf('\n'));
        #ifdef SERIAL_DEBUG_IN
                Serial.println("STATE : " + state);
        #endif
        if (state != stateold) { statechange = true; stateold = state; }
        if (state == "Alarm") { Probe_Alarm = true; }
        if (state == "Run") { SleepTicker.start(); }
}

static void Rx_Wx_NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
        char* RxWx = (char*)pData;
        String wxstr = String(RxWx);
        wxstr = wxstr.substring(0, wxstr.indexOf('\n'));
        #ifdef SERIAL_DEBUG_IN
                Serial.println("WX : " + wxstr);
        #endif
        wx = wxstr.toFloat();
        if (wx != wxold) { wxchange = true; wxold = wx; }
}

static void Rx_Wy_NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
        char* RxWy = (char*)pData;
        String wystr = String(RxWy);
        wystr = wystr.substring(0, wystr.indexOf('\n'));
        #ifdef SERIAL_DEBUG_IN
                Serial.println("WY : " + wystr);
        #endif
        wy = wystr.toFloat();
        if (wy != wyold) { wychange = true; wyold = wy; }
}

static void Rx_Wz_NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
        char* RxWz = (char*)pData;
        String wzstr = String(RxWz);
        wzstr = wzstr.substring(0, wzstr.indexOf('\n'));
        #ifdef SERIAL_DEBUG_IN
                Serial.println("WZ : " + wzstr);
        #endif
        wz = wzstr.toFloat();
        if (wz != wzold) { wzchange = true; wzold = wz; }
}

static void Rx_Wa_NotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
        char* RxWa = (char*)pData;
        String wastr = String(RxWa);
        wastr = wastr.substring(0, wastr.indexOf('\n'));
        #ifdef SERIAL_DEBUG_IN
                Serial.println("WA : " + wastr);
        #endif
        wa = wastr.toFloat();
        if (wa != waold) { wachange = true; waold = wa; }
}

class MyClientCallback : public BLEClientCallbacks {

        void onConnect(BLEClient* pclient) {
                BLEconnected = true;
        }

        void onDisconnect(BLEClient* pclient) {
                BLEconnected = false;
                #ifdef SERIAL_DEBUG
                        Serial.println("[PENDANT] BLE disconnected");
                #endif
        }

};


///////////////////////////////////////////////////////////////////////////
///////////////////////      COMMUNICATION       //////////////////////////
///////////////////////////////////////////////////////////////////////////


void ConnectionSetup() {
        // bool connected;
#ifndef BLE_ONLY
        switch(ConnectionMode) {
        case 0:                         // WIFI SLAVE
                Disconnect(false, true);
                WiFi.mode(WIFI_STA);
                AliveTicker.start();
                #ifdef SERIAL_DEBUG
                        Serial.println("[PENDANT] Setting up Connection via WIFI SLA");
                #endif
                break;
        case 1:                         // WIFI AP
                Disconnect(false, true);
                WiFi.mode(WIFI_AP);
                AliveTicker.start();
                #ifdef SERIAL_DEBUG
                        Serial.println("[PENDANT] Setting up Connection via WIFI AP");
                #endif
                break;
        case 2:                         // BLUETOOTH
                Disconnect(true, false);
#endif
                #ifdef SERIAL_DEBUG
                        Serial.println("[PENDANT] Setting up Connection via BLE");
                #endif

                BLEDevice::init("");
                // BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);     // ??
#ifndef BLE_ONLY
                HoldTicker.stop();
                AliveTicker.stop();
                hold = false;
                break;
        }
#endif
}


bool Connect() {
#ifndef BLE_ONLY
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
#endif
                return(ConnectBT());
#ifndef BLE_ONLY
                break;
        }
        return(false);
#endif
}


void Disconnect(bool WIFI, bool BLE) {

if (WIFI) {
#ifndef BLE_ONLY
        WiFi.disconnect();
        WiFi.mode(WIFI_OFF);
#endif
}

if (BLE) {
        if (pClient != nullptr) {
                pClient->disconnect();
        }
}
}


#ifndef BLE_ONLY
bool ConnectWifiAP() {

        if ((!WiFi.softAP(APSSID.c_str(), APPW.c_str())) || (WiFi.softAPgetStationNum() != 1)) {

                BlinkTicker.start();

                while (!WiFi.softAP(APSSID.c_str(), APPW.c_str())) {
                        delay(100);
                        #ifdef SERIAL_DEBUG
                                Serial.println("WIFI-AP: RESTART AP !");
                        #endif
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
                        #ifdef SERIAL_DEBUG
                                {Serial.println("WIFI-AP: WRONG NUMBER OF CLIENTS !");
                        #endif
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
#endif


#ifndef BLE_ONLY
bool ConnectWifi() {
        if (WiFi.status() != WL_CONNECTED) {

                Disconnect(true, true);

                char ssid_char[50];
                WifiSSID.toCharArray(ssid_char, sizeof(ssid_char));
                char password_char[50];
                WifiPW.toCharArray(password_char, sizeof(password_char));
                WiFi.begin(ssid_char, password_char);

                BlinkTicker.start();
                while (WiFi.status() != WL_CONNECTED) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("WIFI CONNECTING ...");
                        #endif
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
#endif


#ifndef BLE_ONLY
bool ConnectTCP(IPAddress tcpip, int tcpport) {

        BlinkTicker.start();

        while (!TCPClient.connected()) {

                TCPClient.connect(tcpip, tcpport);
                // TCPClient.setNoDelay(true);
                #ifdef SERIAL_DEBUG
                        Serial.println("TCP (RE)CONNECTING ...");
                #endif
                BlinkTicker.update(); SleepTicker.update();

                if (blinker_change) {
                        if (blinker) { TFTPrint(MessageField, "TCP Connecting..", TFT_COLOR_STA_ERR);}
                        else { TFTPrint(StateField, "Config??", TFT_COLOR_STA_ERR); }
                        blinker_change = false;
                }
                delay(100);
                if (checkConfig()) { config(); return(false); }              // Start config routine
        }

        BlinkTicker.stop();
        return(true);
}
#endif


bool ConnectBT() {

        // BlinkTicker.start();
        // BlinkTicker.stop();

        if (BLEconnected) {
                return true;
        }
        else {

                if (checkConfig()) { config; return false; };

                TFTPrint(MessageField, "BLE Connecting..", TFT_COLOR_STA_ERR);

                pClient = BLEDevice::createClient();
        
                // Set Callbacks for BLE Client
                pClient->setClientCallbacks(new MyClientCallback());

                // Scan for GRBLHAL Server first ??

                // connect to GRBLHAL Server
                if (pClient->connect(BluetoothHost) == false) { 
                        if (checkConfig()) { config(); }        // chance to enter config
                        return(false); 
                }

                BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
                if (pRemoteService == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find our service UUID: ");
                                Serial.println("[PENDANT] " + String(SERVICE_UUID.toString().c_str()));
                        #endif
                        return(false);
                }
        
                Rx_State_Characteristic = pRemoteService->getCharacteristic(RX_STATE_CHARACTERISTIC_UUID);
                if (Rx_State_Characteristic == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find RX STATE characteristic UUID");
                        #endif
                        return false;
                }
                Rx_State_Characteristic->registerForNotify(Rx_State_NotifyCallback); //Assign callback functions for the Characteristics

                Rx_Wx_Characteristic = pRemoteService->getCharacteristic(RX_WX_CHARACTERISTIC_UUID);
                if (Rx_Wx_Characteristic == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find RX WX characteristic UUID");
                        #endif
                        return false;
                }
                Rx_Wx_Characteristic->registerForNotify(Rx_Wx_NotifyCallback); //Assign callback functions for the Characteristics

                Rx_Wy_Characteristic = pRemoteService->getCharacteristic(RX_WY_CHARACTERISTIC_UUID);
                if (Rx_Wy_Characteristic == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find RX WY characteristic UUID");
                        #endif
                        return false;
                }
                Rx_Wy_Characteristic->registerForNotify(Rx_Wy_NotifyCallback); //Assign callback functions for the Characteristics

                Rx_Wz_Characteristic = pRemoteService->getCharacteristic(RX_WZ_CHARACTERISTIC_UUID);
                if (Rx_Wz_Characteristic == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find RX WZ characteristic UUID");
                        #endif
                        return false;
                }
                Rx_Wz_Characteristic->registerForNotify(Rx_Wz_NotifyCallback); //Assign callback functions for the Characteristics

                Rx_Wa_Characteristic = pRemoteService->getCharacteristic(RX_WA_CHARACTERISTIC_UUID);
                if (Rx_Wa_Characteristic == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find RX WA characteristic UUID");
                        #endif
                        return false;
                }
                Rx_Wa_Characteristic->registerForNotify(Rx_Wa_NotifyCallback); //Assign callback functions for the Characteristics
                
                Tx_Cmd_Characteristic = pRemoteService->getCharacteristic(TX_CMD_CHARACTERISTIC_UUID);
                if (Tx_Cmd_Characteristic == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find TX CMD characteristic UUID");
                        #endif
                        return false;
                }

                Tx_GCode_Characteristic = pRemoteService->getCharacteristic(TX_GCODE_CHARACTERISTIC_UUID);
                if (Tx_GCode_Characteristic == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find TX GCODE characteristic UUID");
                        #endif
                        return false;
                }

                Tx_Msg_Characteristic = pRemoteService->getCharacteristic(TX_MSG_CHARACTERISTIC_UUID);
                if (Tx_Msg_Characteristic == nullptr) {
                        #ifdef SERIAL_DEBUG
                                Serial.println("[PENDANT] Failed to find TX MSG characteristic UUID");
                        #endif
                        return false;
                }

                resetEncoder(); // needed?

                return true;
        }
}


#ifndef BLE_ONLY
void getState() {

        if (Connect()) { //Check WiFi connection status

                switch (ConnectionMode) {
                case 0:
                case 1:
                        // TCPClient.connect(WifiHost, WifiPort);

                        #define TCPBufferSize 128
                        static char TCPBuffer[TCPBufferSize];
                        // static bool buffer_active = false;
                        static int buffer_idx = 0;
                        char buffer_char;

                        while (TCPClient.available() > 0) {

                                if (TCPClient.available() > 500) {
                                        while (TCPClient.available() > 128) {
                                                TCPClient.read();
                                        }
                                        debug("[PENDANT] WARNING: TCP OVERFLOW DELETED");

                                        while (TCPClient.peek() != '{') {
                                                TCPClient.read();
                                        }
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
                                        deserialize(String(TCPBuffer));
                                }
                                else if ((buffer_idx + 2) < TCPBufferSize) {
                                        TCPBuffer[buffer_idx] = buffer_char;
                                        buffer_idx++;
                                }


                        }
                // TCPClient.stop();

                // Bluetooth
                case 2:
                        break;
                default: 
                        break;
                }

        }
}
#endif


#ifndef BLE_ONLY
void deserialize(String JsonString) {

        DynamicJsonDocument JsonIn(128);
        DeserializationError error = deserializeJson(JsonIn, JsonString);

        if (error) {
                debug("[PENDANT] WARNING: DESERIALIZATION ERROR");
                // debug("[PENDANT] " + String(error.f_str()));
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
                
                #ifdef SERIAL_DEBUG_IN
                        Serial.println("WX:" + String(wx) + "  WY:" + String(wy) + "  WZ:" + String(wz) + "  WA:" + String(wa) + "  STATE:" + state);
                #endif
        }
}
#endif

void sendCmd(const String type, const String cmd, const String cmd_info) {
#ifdef BLE_ONLY
        if (Connect())
#else
        if (Connect() && !hold)
#endif
        {
                // TCPClient.connect(WifiHost, WifiPort);
#ifndef BLE_ONLY
                switch(ConnectionMode) {
                case 0:
                case 1:
                        TCPClient.println("{\"" + type + "\":\"" + cmd + "\"}");
                        AliveTicker.start();
                        break;
                case 2:
#endif
                        if (type == "cmd") {
                                Tx_Cmd_Characteristic->writeValue(cmd.c_str(), cmd.length());
                        }
                        else if (type == "gcode") {
                                Tx_GCode_Characteristic->writeValue(cmd.c_str(), cmd.length());
                        }
                        else if (type == "msg") {
                                Tx_Msg_Characteristic->writeValue(cmd.c_str(), cmd.length());
                        }
#ifndef BLE_ONLY
                        break;
                }
#endif

                #ifdef SERIAL_DEBUG_OUT
                        Serial.println("{\"" + type + "\":\"" + cmd + "\"}");
                #endif

                TFTPrint(MessageField, cmd_info, TFT_COLOR_MSG_NRM);

                // TCPClient.stop();

        }
        else {
                TFTPrint(MessageField, "NO CONNECTION. HOLD!", TFT_COLOR_MSG_ERR);
        }
}


void debug(String msg) {

        #ifdef SERIAL_DEBUG
                Serial.println(msg);
        #endif
        #ifdef MESSAGE_DEBUG
                if (BLEconnected) {
                        sendCmd("msg", msg, ""); 
                }
        #endif
}


#ifndef BLE_ONLY
void Hold() {
        hold = true;
        #ifdef SERIAL_DEBUG 
                Serial.println("WARNING: HOLD!"); 
        #endif
        HoldTicker.stop();
}
#endif


#ifndef BLE_ONLY
void Alive() {
        if (Connect())
        {
                switch(ConnectionMode) {
                case 0:
                case 1:
                        TCPClient.println("{\"OK\"}");
                        break;
                case 2:
                        break;
                }
        }
}
#endif