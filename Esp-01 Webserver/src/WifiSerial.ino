
#include <ESP8266WiFi.h>
#include <ArduinoOTA.h> // for Over-The-Air programming
#include <TickTwo.h>

////////////////////////////  config:  //////////////////////////////////

const char *ssid = "grblHAL";           // You will connect your penant to this Access Point
const char *pw = "grblHAL1";            // and this is the password
IPAddress ip(192, 168, 0, 1);           // this is the ip
IPAddress netmask(255, 255, 255, 0);
#define TCP_PORT 8880

WiFiServer Server(TCP_PORT);
WiFiClient TCPClient;

#define ALIVE_TIME 2              // Update Rate for Sending Code
void Alive();
TickTwo AliveTicker(Alive, (1000 * ALIVE_TIME));

#define DEBUG 0

//////////////////////////////////////////////////////////////////////////


void setup() {

        ArduinoOTA.begin();

        delay(1000);
        Serial.begin(115200);

        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(ip, ip, netmask);
        WiFi.softAP(ssid, pw);

        Server.begin();
        AliveTicker.start();
        // Server.setNoDelay(true); // ? TRUE > NO NAGLE ALGORITHM

        if (DEBUG) { SerialMessage("[WIFI] Module started"); }
}


void loop()
{
        ArduinoOTA.handle();    // OTA-Handler
        AliveTicker.update();

        TCPClient = Server.available();
        // TCPClient.setNoDelay(true);

        if (TCPClient.connected() && DEBUG) { SerialMessage("[WIFI] Client Connected"); }

        while(TCPClient.connected()) {

                // check OTA from time to time...
                ArduinoOTA.handle();

                // read data from the connected client
                while (TCPClient.available() > 0) {
                        Serial.write(TCPClient.read());
                        AliveTicker.start();
                }
                // send data to connected client
                while (Serial.available() > 0) {
                        TCPClient.write(Serial.read());
                }

                AliveTicker.update();

        }

        TCPClient.stop();
        // if (DEBUG) { SerialMessage("[WIFI] Client disconnected"); }

        // read away serial data!
        while (Serial.available()) {
                Serial.read();
                AliveTicker.update();
        }
}


void Alive() {
        if (DEBUG) { SerialMessage("[WIFI] Timeout: Client killed"); }
        TCPClient.stop();
        AliveTicker.stop();
}


void SerialMessage(String Message) {
        Serial.println("{\"msg\":\"" + Message + "\"}");
}
