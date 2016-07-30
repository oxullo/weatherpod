#ifndef EPDSTREAMER_H
#define EPDSTREAMER_H

#include <WiFi101.h>

#define MAX_BUFFER_LENGTH       64

typedef enum ConnectionState {
    STATE_CONN_IDLE,
    STATE_CONN_CONNECTING,
    STATE_CONN_ACTIVE
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
    void connect(const char *host, uint16_t port, const char *path);
    void update();
    void setCallbacks(void (*onStreamingStartingCallback)(void), void (*onBodyByteRead)(char),
            void (*onStreamingCompleted)(void));

private:
    WiFiClient client;
    ConnectionState connState;
    HTTPState httpState;
    char buffer[MAX_BUFFER_LENGTH];
    uint8_t bufferPtr;

    void processLine(const char *line);

    void (*onStreamingStartingCallback)(void);
    void (*onBodyByteReadCallback)(char);
    void (*onStreamingCompletedCallback)(void);
};

#endif
