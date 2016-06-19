// MKR1000 WiFi + TCM2
// Connects to WiFi and to a renderer server, downloading the generated bitmap
// and displaying it on the eink display

#include <SPI.h>
#include <WiFi101.h>

#define DEBUG

#define TRIGGER_IN_PIN 5

#define STATE_CONN_IDLE         0
#define STATE_CONN_CONNECTING   1
#define STATE_CONN_ACTIVE       2

#define STATE_HTTP_INIT             0
#define STATE_HTTP_HEADER_STATUS    1
#define STATE_HTTP_HEADER_LINES     2
#define STATE_HTTP_BODY             3

#define SPI_SPEED           1E06

#define MAX_CHUNK_SIZE      0xfa
#define SS_PIN              7
#define TC_BUSY_PIN         2
#define TC_ENABLE_PIN       3

#define LED_PIN             6

#define SS_ASSERT_DELAY_US      10
#define SS_DEASSERT_DELAY_US    10
#define BUSY_WAIT_DELAY_US      50
#define BUSY_RELEASE_DELAY_US   10

#define EP_SW_NORMAL_PROCESSING             0x9000
#define EP_SW_WRONG_LENGTH                  0x6700
#define EP_FRAMEBUFFER_SLOT_NOT_AVAILABLE   0x6981
#define EP_SW_INVALID_LE                    0x6c00
#define EP_SW_WRONG_PARAMETERS_P1P2         0x6a00
#define EP_FRAMEBUFFER_SLOT_OVERRUN         0x6a84
#define EP_SW_INSTRUCTION_NOT_SUPPORTED     0x6d00
#define EP_SW_GENERAL_ERROR                 0x6f00


const char *ssid = "ssid";
const char *pass = "psk";
//const char *server = "www.123.org";
IPAddress server(192,168,20,13);  // numeric IP for Google (no DNS)

WiFiClient client;
uint8_t connState = STATE_CONN_IDLE;
uint8_t httpState = STATE_HTTP_INIT;


SPISettings spiSettings(SPI_SPEED, MSBFIRST, SPI_MODE3);

void startTransmission()
{
    digitalWrite(SS_PIN, LOW);
    delayMicroseconds(SS_ASSERT_DELAY_US);
    SPI.beginTransaction(spiSettings);
}

void endTransmission()
{
    SPI.endTransaction();
    delayMicroseconds(SS_DEASSERT_DELAY_US);
    digitalWrite(SS_PIN, HIGH);
}

void busyWait()
{
    delayMicroseconds(BUSY_WAIT_DELAY_US);
    while(digitalRead(TC_BUSY_PIN) == LOW) {
        #ifdef DEBUG
        Serial.print(".");
        delay(10);
        #endif
    };
    delayMicroseconds(BUSY_RELEASE_DELAY_US);
}

uint16_t sendCommand(uint8_t ins, uint8_t p1, uint8_t p2, uint8_t lc, uint8_t *data)
{
    #ifdef DEBUG
    Serial.print("INS=");
    Serial.print(ins, HEX);
    Serial.print(" P1=");
    Serial.print(p1, HEX);
    Serial.print(" P2=");
    Serial.print(p2, HEX);
    Serial.print(" Lc=");
    Serial.print(lc, HEX);
    Serial.print(": ");
    #endif

    startTransmission();
    SPI.transfer(ins);
    SPI.transfer(p1);
    SPI.transfer(p2);

    if (lc) {
        SPI.transfer(lc);
        SPI.transfer(data, lc);
    }
    endTransmission();
    busyWait();

    startTransmission();
    uint16_t rc = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
    endTransmission();
    busyWait();

    #ifdef DEBUG
    if (rc != EP_SW_NORMAL_PROCESSING) {
        Serial.print(" ERR=");
        Serial.println(rc, HEX);
    } else {
        Serial.println("OK");
    }
    #endif

    return rc;
}

uint16_t sendAndReadString(uint8_t ins, uint8_t p1, uint8_t p2, uint8_t le, char *buffer)
{
    startTransmission();
    SPI.transfer(ins);
    SPI.transfer(p1);
    SPI.transfer(p2);
    SPI.transfer(le);
    endTransmission();
    busyWait();

    startTransmission();

    char ch;
    uint8_t i=0;
    do {
        ch = SPI.transfer(0x00);
        buffer[i++] = ch;

        #ifdef DEBUG
        Serial.print("CH=");
        Serial.println(ch, HEX);
        #endif
    } while (ch);

    uint16_t rc = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
    endTransmission();
    busyWait();

    return rc;
}

uint16_t sendCommand(uint8_t ins, uint8_t p1, uint8_t p2)
{
    return sendCommand(ins, p1, p2, 0, NULL);
}

void dumpLinesStates()
{
    Serial.print("SS=");
    Serial.print(digitalRead(SS_PIN));
    Serial.print(" TC_ENA=");
    Serial.print(digitalRead(TC_ENABLE_PIN));
    Serial.print(" TC_BUSY=");
    Serial.print(digitalRead(TC_BUSY_PIN));
    Serial.print(" CLK=");
    Serial.print(digitalRead(13));
    Serial.print(" MISO=");
    Serial.print(digitalRead(11));
    Serial.print(" MOSI=");
    Serial.println(digitalRead(12));
}

void tcm2init()
{
    SPI.begin();
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    digitalWrite(SS_PIN, HIGH);
    pinMode(SS_PIN, OUTPUT);

    // Necessary to prepare the clock for a falling edge
    SPI.beginTransaction(spiSettings);
    SPI.endTransaction();

    Serial.println("Waking up TCM");
    pinMode(TC_ENABLE_PIN, OUTPUT);
    digitalWrite(TC_ENABLE_PIN, LOW);
    delay(100);
    busyWait();

    dumpLinesStates();
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
    
    tcm2init();
    Serial.print("TCM Dev info: ");

    char buffer[24];
    uint16_t rc = sendAndReadString(0x30, 0x01, 0x01, 0x00, (char *)buffer);
    Serial.println((char *)buffer);

    Serial.println(rc, HEX);

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
                // Resetting image buffer pointer
                sendCommand(0x20, 0x0d, 0x00);
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
    
    sendCommand(0x20, 0x01, 0x00, length, (uint8_t*)buffer);
    // ErrataSheet_rA, solution 1
    delayMicroseconds(1200);
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
            
            // Refresh screen
            sendCommand(0x24, 0x01, 0x00);

            Serial.println();
            Serial.println("disconnecting from server.");
            client.stop();

            connState = STATE_CONN_IDLE;
        }
    }
}

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
