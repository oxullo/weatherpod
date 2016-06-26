/*
Weatherpod - WiFi E-ink weather widget
Copyright (C) 2016  OXullo Intersecans <x@brainrapers.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include <WiFi101.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>

#include "tcm2.h"

#define DEBUG

#define TRIGGER_IN_PIN 5

#define STATE_CONN_IDLE         0
#define STATE_CONN_CONNECTING   1
#define STATE_CONN_ACTIVE       2

#define STATE_HTTP_INIT             0
#define STATE_HTTP_HEADER_STATUS    1
#define STATE_HTTP_HEADER_LINES     2
#define STATE_HTTP_BODY             3

const char *ssid = "SSID";
const char *pass = "PSK";
//const char *server = "www.123.org";
IPAddress server(192,168,20,13);

WiFiClient client;
uint8_t connState = STATE_CONN_IDLE;
uint8_t httpState = STATE_HTTP_INIT;

TCM2 tcm;
Adafruit_BME280 bme;

void printWifiStatus() {
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);

    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
}


void setup()
{
    // Initialize serial and wait for port to open
    Serial.begin(115200);

    while (!Serial);

    // check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("WiFi shield not present");
        // don't continue:
        while (true);
    }

    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // attempt to connect to Wifi network:
    while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
        Serial.println("Connection failed, trying again in 5s");
        delay(5000);
    }
    Serial.println("Connected to wifi");
    printWifiStatus();
    
    tcm.begin();
    
    char buffer[24];
    tcm.getDeviceInfo(buffer);
    
    Serial.print("TCM2 getDeviceInfo(): ");
    Serial.println(buffer);
    
    bme.begin();

    Serial.print("T=");
    Serial.print(bme.readTemperature(), 2);
    Serial.print("C P=");
    Serial.print(bme.readPressure() / 100.0, 2);
    Serial.print("Pa H=");
    Serial.print(bme.readHumidity(), 2);
    Serial.println("%");
    
    pinMode(TRIGGER_IN_PIN, INPUT_PULLUP);
}

void processLine(const char *line)
{
    switch (httpState) {
        case STATE_HTTP_HEADER_STATUS:
            Serial.print("STATUS=");
            httpState = STATE_HTTP_HEADER_LINES;
            break;

        case STATE_HTTP_HEADER_LINES:
            Serial.print("HDR=");
            if (line[0] == 0) {
                httpState = STATE_HTTP_BODY;

                tcm.resetDataPointer();
            }
            break;

        case STATE_HTTP_BODY:
            Serial.print("BODY=");
            break;
    }
    Serial.println(line);
}

void processBodyChunk(const char *buffer, uint8_t length)
{
    Serial.print("uploading ");
    Serial.print(length);
    Serial.print("B buffer[0]=0x");
    Serial.println(buffer[0], HEX);
    
    tcm.uploadImageData(buffer, length);
}

void connect()
{
    connState = STATE_CONN_CONNECTING;
    Serial.println("\nConnecting to server...");

    if (client.connect(server, 5000)) {
        Serial.println("connected to server");
        // Make a HTTP request:
        client.println("GET /weatherpod/v1/testbitmap HTTP/1.1");
        client.println("Host: 192.168.20.13");
        client.println("Connection: close");
        client.println();
        httpState = STATE_HTTP_HEADER_STATUS;
        connState = STATE_CONN_ACTIVE;
    } else {
        Serial.println("Connection failed");
        connState = STATE_CONN_IDLE;
    }
}

void loop()
{
    static char buffer[MAX_CHUNK_SIZE];
    static uint8_t i = 0;

    if (connState == STATE_CONN_IDLE and digitalRead(TRIGGER_IN_PIN) == LOW) {
        connect();
    } else if (connState == STATE_CONN_ACTIVE) {
        // if there are incoming bytes available
        // from the server, read them and print them:
        while (client.available()) {
            char c = client.read();
            
            if (httpState == STATE_HTTP_BODY) {
                buffer[i++] = c;
                if (i == MAX_CHUNK_SIZE) {
                    processBodyChunk(buffer, i);
                    i = 0;
                }
            } else {
                if (c == 13) {
                    continue;
                } else if (c == 10) {
                    buffer[i] = 0;
                    processLine(buffer);
                    i = 0;
                } else {
                    buffer[i++] = c;
                }
            }
        }

        // if the server's disconnected, stop the client:
        if (!client.connected()) {
            for (uint8_t j = 0 ; j < i ; ++j) {
                Serial.print("v[");
                Serial.print(j);
                Serial.print("]=0x");
                Serial.println(buffer[j], HEX);
            }
            processBodyChunk(buffer, i);
            i = 0;
            
            tcm.displayUpdate();

            Serial.println();
            Serial.println("disconnecting from server.");
            client.stop();

            connState = STATE_CONN_IDLE;
        }
    }
}

