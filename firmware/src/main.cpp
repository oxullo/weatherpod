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
#include "mkrpm.h"
#include "config.h"

#define TCM2_BUSY_PIN       2
#define TCM2_ENABLE_PIN     3
#define TCM2_SPI_CS         7


WiFiManager wifiManager;
TCM2 tcm(TCM2_BUSY_PIN, TCM2_ENABLE_PIN, TCM2_SPI_CS);
EPDStreamer epdStreamer;
LocalSensor localSensor;
MkrPM pm;

uint32_t tsNextPoll = 0;
uint8_t buffer[TCM2_MAX_CHUNK_SIZE];
uint8_t bufferPtr = 0;
uint16_t totalBodySize = 0;

void blink()
{
    digitalWrite(LED_BUILTIN, HIGH);
    delay(30);
    digitalWrite(LED_BUILTIN, LOW);
}

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

void initPeripherals()
{
    tcm.begin();
    localSensor.begin();

    #ifdef DEBUG
    char buffer[24];
    tcm.getDeviceInfo((uint8_t *)buffer);

    Serial.print("TCM2 getDeviceInfo(): ");
    Serial.println(buffer);

    localSensor.printData();
    localSensor.getDataAsPostPayload(buffer);
    Serial.println(buffer);
    #endif
}

void setup()
{
    #ifdef DEBUG
    Serial.begin(115200);
    while (!Serial);
    #else
    USBDevice.detach();
    #endif

    pinMode(LED_BUILTIN, OUTPUT);

    epdStreamer.setCallbacks(onStreamingStarting, onBodyByteRead, onStreamingCompleted);

    pm.begin();
}

void loop()
{
    blink();

    initPeripherals();

    if (wifiManager.connect(WIFI_SSID, WIFI_PSK)) {
        triggerUpdateRequest();

        wifiManager.disconnect();
    }

    Serial.println("Going to sleep");
    #ifdef DEBUG
    USBDevice.detach();
    #endif

    pm.sleepForMinutes(POLL_PERIOD_MINS);

    #ifdef DEBUG
    USBDevice.attach();
    #endif
}
