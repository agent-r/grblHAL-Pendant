
/*
   TODO: SERIAL-Eingänge verwerfen, wenn keine Verbindung besteht !!!
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ArduinoOTA.h> // for Over-The-Air programming

// config: ////////////////////////////////////////////////////////////

#define buffer_size 128     // better 256?

const char *ssid = "grblHAL";           // You will connect your penant to this Access Point
const char *pw = "grblHAL1";            // and this is the password
IPAddress ip(192, 168, 0, 1);           // this is the ip
IPAddress netmask(255, 255, 255, 0);
#define TCP_PORT 8880

WiFiServer Server(TCP_PORT);
WiFiClient TCPClient;

#define DEBUG 0

//////////////////////////////////////////////////////////////////////////


void setup() {

        ArduinoOTA.begin();

        delay(500);
        Serial.begin(115200);

        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(ip, ip, netmask);
        WiFi.softAP(ssid, pw);

        Server.begin();
        // Server.setNoDelay(true); // ? TRUE > NO NAGLE ALGORITHM

        // delay(4000);    // wait a few seconds //
        if (DEBUG) { SerialMessage("[WIFI] Module started"); }

}


void loop()
{

        ArduinoOTA.handle();    // OTA-Handler

        TCPClient = Server.available();
        // TCPClient.setNoDelay(true);

        if (TCPClient) {

                if(TCPClient.connected()) {
                        if (DEBUG) { SerialMessage("[WIFI] Client Connected"); }
                        // TCPClient.setNoDelay(true);
                }
                else {
                        if (DEBUG) { SerialMessage("[WIFI] No Client Connected"); }
                }

                while(TCPClient.connected()) {

                        // check OTA from time to time...
                        ArduinoOTA.handle();

                        // read data from the connected client
                        while (TCPClient.available() > 0) {
                                Serial.write(TCPClient.read());
                        }

                        //Send Data to connected client
                        while (Serial.available() > 0) {
                                // usually max 1071 bytes -> only use around 400!
                                // if (TCPClient.availableForWrite() > 600) {
                                TCPClient.write(Serial.read());
                                // }
                                // else {
                                //        Serial.read();  // discard Data to prevent overflow!
                                // }
                        }
                }

                TCPClient.stop();
                if (DEBUG) { SerialMessage("[WIFI] Client disconnected"); }
        }

}


void SerialMessage(String Message) {
        Serial.println("{\"msg\":\"" + Message + "\"}");
}
