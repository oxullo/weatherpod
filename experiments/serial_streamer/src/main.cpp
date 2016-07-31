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

#include "tcm2.h"

#define TIMEOUT                 1000
#define TOTAL_TRANSFER_SIZE     15000 + 16

typedef enum State {
    STATE_IDLE,
    STATE_READING
} State;

TCM2 tcm;
State state = STATE_IDLE;

char buffer[MAX_CHUNK_SIZE];
uint8_t bufferPtr = 0;
uint32_t tsLastActivity = 0;
uint32_t totalBytesRead = 0;

void processChunk(const char *buffer, uint8_t length)
{
    tcm.uploadImageData(buffer, length);
}

void reset()
{
    totalBytesRead = 0;
    tcm.resetDataPointer();
}

void addByte(char c)
{
    buffer[bufferPtr++] = c;
    ++totalBytesRead;

    if (bufferPtr == MAX_CHUNK_SIZE) {
        processChunk(buffer, bufferPtr);
        bufferPtr = 0;
    }
}

void streamingCompleted()
{
    processChunk(buffer, bufferPtr);
    bufferPtr = 0;

    tcm.displayUpdate();
}

void setup()
{
    // Initialize serial and wait for port to open
    Serial.begin(115200);

    tcm.begin();
}

void loop()
{
    if (Serial.available()) {
        char data = Serial.read();

        if (state == STATE_IDLE) {
            reset();
            state = STATE_READING;
        }

        addByte(data);

        if (totalBytesRead == TOTAL_TRANSFER_SIZE) {
            streamingCompleted();
            state = STATE_IDLE;
            Serial.print("ACK");
        }

        tsLastActivity = millis();
    }

    if (state == STATE_READING && millis() - tsLastActivity > TIMEOUT) {
        state = STATE_IDLE;
        Serial.print("TMO");
    }
}
