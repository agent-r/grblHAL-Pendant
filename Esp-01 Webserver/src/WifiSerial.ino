
/*
   TODO: SERIAL-Eingänge verwerfen, wenn keine Verbindung besteht !!!
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// config: ////////////////////////////////////////////////////////////

#define buffer_size 128     // better 256?

const char *ssid = "grblHAL";           // You will connect your penant to this Access Point
const char *pw = "grblHAL1";            // and this is the password
IPAddress ip(192, 168, 0, 1);           // this is the ip
IPAddress netmask(255, 255, 255, 0);
#define TCP_PORT 8880

#define UART_BAUD 115200

WiFiServer Server(TCP_PORT);
WiFiClient TCPClient;


//////////////////////////////////////////////////////////////////////////


void setup() {

        delay(500);
        Serial.begin(UART_BAUD);
        delay(100);

        WiFi.mode(WIFI_AP);
        WiFi.softAPConfig(ip, ip, netmask);
        WiFi.softAP(ssid, pw);

        Server.begin();
        // Server.setNoDelay(true); // ? TRUE > NO NAGLE ALGORITHM

}


void loop()
{

        TCPClient = Server.available();

        if (TCPClient) {
                if(TCPClient.connected())
                {
                        Serial.println("Client Connected");
                        // TCPClient.setNoDelay(true);
                }
                else {
                        Serial.println("No Client Connected");
                }

                while(TCPClient.connected()) {
                        while(TCPClient.available()>0) {
                                // read data from the connected client
                                Serial.write(TCPClient.read());
                        }
                        //Send Data to connected client
                        while(Serial.available()>0)
                        {
                                TCPClient.write(Serial.read());
                        }
                }

                TCPClient.stop();
                Serial.println("Client disconnected");
        }

        // delay(5);

}
