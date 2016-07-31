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

#ifndef EPDSTREAMER_H
#define EPDSTREAMER_H

#include <WiFi101.h>

#define MAX_BUFFER_LENGTH       64

typedef enum ConnectionState {
    STATE_CONN_DISCONNECTED,
    STATE_CONN_CONNECTING,
    STATE_CONN_CONNECTED
} ConnectionState;

typedef enum HTTPState {
    STATE_HTTP_INIT,
    STATE_HTTP_HEADER_STATUS,
    STATE_HTTP_HEADER_LINES,
    STATE_HTTP_BODY
} HTTPState;

class EPDStreamer {
public:
    EPDStreamer();
    void begin(const char *wifiSsid, const char *wifiPsk);
    void printWifiStatus();
    bool connect(const char *host, uint16_t port);
    void get(const char *host, const char *path);
    void post(const char *host, const char *path, const char *data);
    void update();
    void setCallbacks(void (*onStreamingStartingCallback)(void), void (*onBodyByteRead)(char),
            void (*onStreamingCompleted)(void));

private:
    WiFiClient client;
    ConnectionState connState;
    HTTPState httpState;
    char buffer[MAX_BUFFER_LENGTH];
    uint8_t bufferPtr;
    bool callbacksEnabled;

    void processBuffer();

    void (*onStreamingStartingCallback)(void);
    void (*onBodyByteReadCallback)(char);
    void (*onStreamingCompletedCallback)(void);
};

#endif
