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


#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>

#include "epdstreamer.h"
#include "tcm2.h"

#define DEBUG

#define TRIGGER_IN_PIN          5

#define WIFI_SSID               "SSID"
#define WIFI_PSK                "PSK"
#define SERVER_HOST             "192.168.20.18"
#define SERVER_PORT             5000
#define SERVER_PATH             "/weatherpod/v1/testbitmap"


EPDStreamer epdStreamer;
TCM2 tcm;
Adafruit_BME280 bme;

char buffer[MAX_CHUNK_SIZE];
uint8_t bufferPtr = 0;

void processBodyChunk(const char *buffer, uint8_t length)
{
    Serial.print("uploading ");
    Serial.print(length);
    Serial.print("B buffer[0]=0x");
    Serial.println(buffer[0], HEX);

    tcm.uploadImageData(buffer, length);
}

void onStreamingStarting()
{
    tcm.resetDataPointer();
}

void onBodyByteRead(char c)
{
    buffer[bufferPtr++] = c;
    if (bufferPtr == MAX_CHUNK_SIZE) {
        processBodyChunk(buffer, bufferPtr);
        bufferPtr = 0;
    }
}

void onStreamingCompleted()
{
    processBodyChunk(buffer, bufferPtr);
    bufferPtr = 0;

    tcm.displayUpdate();
}

void setup()
{
    // Initialize serial and wait for port to open
    Serial.begin(115200);

    #ifdef DEBUG
    while (!Serial);
    #endif

    epdStreamer.begin(WIFI_SSID, WIFI_PSK);
    epdStreamer.setCallbacks(onStreamingStarting, onBodyByteRead, onStreamingCompleted);

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

void loop()
{
    epdStreamer.update();
    if (digitalRead(TRIGGER_IN_PIN) == LOW) {
        epdStreamer.connect(SERVER_HOST, SERVER_PORT, SERVER_PATH);
    }
}
