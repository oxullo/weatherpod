#include "epdstreamer.h"

EPDStreamer::EPDStreamer() :
    connState(STATE_CONN_IDLE),
    httpState(STATE_HTTP_INIT),
    bufferPtr(0),
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

void EPDStreamer::processLine(const char *line)
{
    switch (httpState) {
        case STATE_HTTP_INIT:
            Serial.println("processLine() called with unexpected http state");
            return;

        case STATE_HTTP_HEADER_STATUS:
            Serial.print("STATUS=");
            httpState = STATE_HTTP_HEADER_LINES;
            break;

        case STATE_HTTP_HEADER_LINES:
            Serial.print("HDR=");
            if (line[0] == 0) {
                httpState = STATE_HTTP_BODY;
                if (onStreamingStartingCallback) {
                    onStreamingStartingCallback();
                }
            }
            break;

        case STATE_HTTP_BODY:
            Serial.print("BODY=");
            break;
    }
    Serial.println(line);
}

void EPDStreamer::connect(const char *host, uint16_t port, const char *path)
{
    if (connState != STATE_CONN_IDLE) {
        return;
    }

    connState = STATE_CONN_CONNECTING;
    Serial.print("Attempting connection to: http://");
    Serial.print(host);
    Serial.print(":");
    Serial.print(port);
    Serial.println(path);

    if (client.connect(host, port)) {
        Serial.println("Connected");

        client.print("GET ");
        client.print(path);
        client.println(" HTTP/1.1");
        client.print("Host: ");
        client.println(host);
        client.println("Connection: close");
        client.println();
        httpState = STATE_HTTP_HEADER_STATUS;
        connState = STATE_CONN_ACTIVE;
    } else {
        Serial.println("Connection failed");
        connState = STATE_CONN_IDLE;
    }
}

void EPDStreamer::update()
{
    if (connState == STATE_CONN_ACTIVE) {
        while (client.available()) {
            char c = client.read();

            if (httpState == STATE_HTTP_BODY) {
                if (onBodyByteReadCallback) {
                    onBodyByteReadCallback(c);
                }
            } else {
                if (c == 13) {
                    continue;
                } else if (c == 10) {
                    buffer[bufferPtr] = 0;
                    processLine(buffer);
                    bufferPtr = 0;
                } else {
                    buffer[bufferPtr++] = c;
                }
            }
        }

        if (!client.connected()) {
            if (onStreamingCompletedCallback) {
                onStreamingCompletedCallback();
            }

            Serial.println();
            Serial.println("disconnecting from server.");
            client.stop();

            connState = STATE_CONN_IDLE;
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
