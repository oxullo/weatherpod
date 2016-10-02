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


#include <TCM2.h>

#include "wifimanager.h"
#include "epdstreamer.h"
#include "localsensor.h"
#include "config.h"

#define TRIGGER_IN_PIN          5

#define TCM2_BUSY_PIN       2
#define TCM2_ENABLE_PIN     3
#define TCM2_SPI_CS         7


WiFiManager wifiManager;
TCM2 tcm(TCM2_BUSY_PIN, TCM2_ENABLE_PIN, TCM2_SPI_CS);
EPDStreamer epdStreamer;
LocalSensor localSensor;

uint32_t tsNextPoll = 0;
uint8_t buffer[TCM2_MAX_CHUNK_SIZE];
uint8_t bufferPtr = 0;
uint16_t totalBodySize = 0;

void processBodyChunk(const uint8_t *buffer, uint8_t length)
{
    #ifdef DEBUG
    Serial.print("uploading ");
    Serial.print(length);
    Serial.print("B buffer[0]=0x");
    Serial.println(buffer[0], HEX);
    #endif

    tcm.uploadImageData(buffer, length);
}

void onStreamingStarting()
{
    tcm.resetDataPointer();
    Serial.println("Preparing for TCM image upload");
}

void onBodyByteRead(char c)
{
    buffer[bufferPtr++] = c;
    ++totalBodySize;

    if (bufferPtr == TCM2_MAX_CHUNK_SIZE) {
        processBodyChunk(buffer, bufferPtr);
        bufferPtr = 0;
    }
}

void onStreamingCompleted()
{
    processBodyChunk(buffer, bufferPtr);
    bufferPtr = 0;

    Serial.print("Downloaded ");
    Serial.print(totalBodySize);
    Serial.println("B");

    totalBodySize = 0;

    Serial.println("TCM upload terminated, refreshing screen");
    tcm.displayUpdate();
}

void triggerUpdateRequest()
{
    if (epdStreamer.connect(SERVER_HOST, SERVER_PORT)) {
        char buffer[64];
        localSensor.getDataAsPostPayload(buffer);
        strcat(buffer, "&id=" MY_AUTHID);
        epdStreamer.post(SERVER_HOST, SERVER_PATH, buffer);
    }
}

void setup()
{
    // Initialize serial and wait for port to open
    Serial.begin(115200);

    #ifdef DEBUG
    while (!Serial);
    #endif

    wifiManager.setup(WIFI_SSID, WIFI_PSK);
    epdStreamer.setCallbacks(onStreamingStarting, onBodyByteRead, onStreamingCompleted);

    tcm.begin();

    char buffer[24];
    tcm.getDeviceInfo((uint8_t *)buffer);

    Serial.print("TCM2 getDeviceInfo(): ");
    Serial.println(buffer);

    localSensor.begin();
    localSensor.printData();

    localSensor.getDataAsPostPayload(buffer);
    Serial.println(buffer);

    pinMode(TRIGGER_IN_PIN, INPUT_PULLUP);
}

void loop()
{
    if (digitalRead(TRIGGER_IN_PIN) == LOW || millis() > tsNextPoll) {
        if (epdStreamer.connect(SERVER_HOST, SERVER_PORT)) {
            char buffer[64];
            localSensor.getDataAsPostPayload(buffer);
            strcat(buffer, "&id=" MY_AUTHID);
            epdStreamer.post(SERVER_HOST, SERVER_PATH, buffer);
        }
        tsNextPoll = millis() + POLL_PERIOD * 1000;
    }
}
