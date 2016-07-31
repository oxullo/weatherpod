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

#include "epdstreamer.h"

#define RESPONSE_STATUS_CODE_VALID      200
#define CONTENT_TYPE_VALID              "image/edp"

EPDStreamer::EPDStreamer() :
    connState(STATE_CONN_DISCONNECTED),
    httpState(STATE_HTTP_INIT),
    bufferPtr(0),
    callbacksEnabled(true),
    onStreamingStartingCallback(0),
    onBodyByteReadCallback(0),
    onStreamingCompletedCallback(0)
{
}

void EPDStreamer::begin(const char *wifiSsid, const char *wifiPsk)
{
    // check for the presence of the shield
    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("WiFi shield not present");
        // don't continue:
        while (true);
    }

    Serial.print("Attempting to connect to SSID: ");
    Serial.println(wifiSsid);
    // attempt to connect to Wifi network:
    while (WiFi.begin(wifiSsid, wifiPsk) != WL_CONNECTED) {
        Serial.println("Connection failed, trying again in 5s");
        delay(5000);
    }
    Serial.println("Connected to wifi");
    printWifiStatus();
}

void EPDStreamer::printWifiStatus()
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    Serial.print("Local IP Address: ");
    Serial.println((IPAddress)WiFi.localIP());

    // print the received signal strength:
    Serial.print("Signal strength:");
    Serial.print(WiFi.RSSI());
    Serial.println("dBm");
}

void EPDStreamer::processBuffer()
{
    switch (httpState) {
        case STATE_HTTP_HEADER_STATUS:
            {
                Serial.print("STATUS=");
                Serial.println(buffer);

                strtok(buffer, " ");  // protocol version chunk (discard)
                char *statusCode = strtok(0, " ");

                if (statusCode) {
                    uint16_t responseStatusCode = atoi(statusCode);

                    if (responseStatusCode != RESPONSE_STATUS_CODE_VALID) {
                        Serial.println("Warning: invalid response status code: disabling callbacks");
                        callbacksEnabled = false;
                    } else {
                        Serial.println("Valid response status code");
                    }
                } else {
                    Serial.println("Error: cannot parse response status code");
                }

                httpState = STATE_HTTP_HEADER_LINES;
                break;
            }

        case STATE_HTTP_HEADER_LINES:
            if (strlen(buffer) == 0) {
                httpState = STATE_HTTP_BODY;
                if (callbacksEnabled && onStreamingStartingCallback) {
                    onStreamingStartingCallback();
                }
            } else {
                Serial.print("HDR=");
                Serial.println(buffer);

                char *key = strtok(buffer, ":");
                char *value = strtok(0, "");

                // Get rid of the trailing space if a valid key/value pair is found
                if (value && strlen(value) > 0) {
                    ++value;
                }

                if (strcasecmp(key, "content-type") == 0) {
                    if (strcasecmp(value, CONTENT_TYPE_VALID) != 0) {
                        Serial.print("Invalid content type ");
                        Serial.print(value);
                        Serial.println(": disabling callbacks");

                        callbacksEnabled = false;
                    } else {
                        Serial.println("Valid content type");
                    }
                }
            }
            break;

        case STATE_HTTP_BODY:
        case STATE_HTTP_INIT:
            Serial.println("processBuffer() called with unexpected http state");
            break;
    }
}

bool EPDStreamer::connect(const char *host, uint16_t port)
{
    if (connState != STATE_CONN_DISCONNECTED) {
        return false;
    }

    callbacksEnabled = true;

    IPAddress ip;

    connState = STATE_CONN_CONNECTING;
    Serial.print("Attempting connection to: ");
    Serial.print(host);
    Serial.print(":");
    Serial.println(port);

    bool connectionSuccess;

    if (ip.fromString(host)) {
        Serial.println("Skipping name resolution");
        connectionSuccess = client.connect(ip, port);
    } else {
        connectionSuccess = client.connect(host, port);
    }

    if (connectionSuccess) {
        Serial.println("Connected");

        connState = STATE_CONN_CONNECTED;
        return true;
    } else {
        Serial.println("Connection failed");
        connState = STATE_CONN_DISCONNECTED;
        return false;
    }
}

void EPDStreamer::get(const char *host, const char *path)
{
    if (connState != STATE_CONN_CONNECTED) {
        return;
    }

    Serial.print("GET ");
    Serial.println(path);

    client.print("GET ");
    client.print(path);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(host);
    client.println("Connection: close");
    client.println();

    httpState = STATE_HTTP_HEADER_STATUS;
}

void EPDStreamer::post(const char *host, const char *path, const char *data)
{
    if (connState != STATE_CONN_CONNECTED) {
        return;
    }

    Serial.print("POST ");
    Serial.print(path);
    Serial.print(" data=");
    Serial.println(data);

    client.print("POST ");
    client.print(path);
    client.println(" HTTP/1.1");
    client.print("Host: ");
    client.println(host);
    client.println("Connection: close");
    client.println("Content-Type: application/x-www-form-urlencoded");
    client.print("Content-Length: ");
    client.println(strlen(data));
    client.println();
    client.print(data);

    httpState = STATE_HTTP_HEADER_STATUS;
}

void EPDStreamer::update()
{
    if (connState == STATE_CONN_CONNECTED) {
        while (client.available()) {
            char c = client.read();

            if (httpState == STATE_HTTP_BODY) {
                if (callbacksEnabled && onBodyByteReadCallback) {
                    onBodyByteReadCallback(c);
                }
            } else {
                if (c == 13) {
                    continue;
                } else if (c == 10) {
                    buffer[bufferPtr] = 0;
                    processBuffer();
                    bufferPtr = 0;
                } else {
                    buffer[bufferPtr++] = c;
                }
            }
        }

        if (!client.connected()) {
            Serial.println();
            Serial.println("The client has disconnected");
            client.stop();
            connState = STATE_CONN_DISCONNECTED;

            if (callbacksEnabled && onStreamingCompletedCallback) {
                onStreamingCompletedCallback();
            }
        }
    }
}

void EPDStreamer::setCallbacks(void (*onStreamingStarting)(void), void (*onBodyByteRead)(char),
        void (*onStreamingCompleted)(void))
{
    onStreamingStartingCallback = onStreamingStarting;
    onBodyByteReadCallback = onBodyByteRead;
    onStreamingCompletedCallback = onStreamingCompleted;
}
