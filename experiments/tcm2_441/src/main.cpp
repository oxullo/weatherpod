// Image uploading test
// TCM-P441-230-v1.0 + Pervasive display 4,41"
// Hardware SPI connected to the TCM interface
// SS, /TC_BUSY, /TC_ENABLE connected as shown below

#include <Arduino.h>
#include <SPI.h>

#include "test_pattern.h"

#undef DEBUG

#define SPI_SPEED           1E06

#define MAX_CHUNK_SIZE      0xfa
#define SS_PIN              10
#define TC_BUSY_PIN         2
#define TC_ENABLE_PIN       3

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
    while(digitalRead(TC_BUSY_PIN) == LOW);
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

uint16_t sendMerda(uint8_t ins, uint8_t p1, uint8_t p2)
{
    startTransmission();
    SPI.transfer(ins);
    SPI.transfer(p1);
    SPI.transfer(p2);
    SPI.transfer(0x00);
    endTransmission();
    busyWait();

    startTransmission();
    uint16_t rc = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
    endTransmission();
    busyWait();

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

uint16_t fillTestPattern()
{
    uint8_t pattern = 0x55;
    return sendCommand(0x20, 0x0b, 0x00, 0x01, &pattern);
}

void setup()
{
    SPI.begin();
    Serial.begin(115200);
    digitalWrite(SS_PIN, HIGH);
    pinMode(SS_PIN, OUTPUT);

    Serial.println("Waking up TCM");
    pinMode(TC_ENABLE_PIN, OUTPUT);
    digitalWrite(TC_ENABLE_PIN, LOW);
    delay(100);
    busyWait();

    dumpLinesStates();

    // Necessary to prepare the clock for a falling edge
    SPI.beginTransaction(spiSettings);
    SPI.endTransaction();

    dumpLinesStates();
}

void loop()
{
    uint8_t buffer[MAX_CHUNK_SIZE];
    uint8_t chunkSize;
    uint16_t offset = 0;
    uint16_t rc;

    Serial.print("TCM Dev info: ");

    rc = sendAndReadString(0x30, 0x01, 0x01, 0x00, (char *)buffer);
    Serial.println((char *)buffer);

    Serial.println(rc, HEX);

    // fillTestPattern();
    //
    // Serial.println("Displaying fb #0");
    // rc = sendMerda(0x24, 0x01, 0x00);

    rc = sendCommand(0x20, 0x0d, 0x00);

    while (offset < TOTAL_SIZE) {
        chunkSize = min(TOTAL_SIZE - offset, MAX_CHUNK_SIZE);

        memcpy_P(buffer, &test_1bit[offset], chunkSize);

        rc = sendCommand(0x20, 0x01, 0x00, chunkSize, buffer);
        // ErrataSheet_rA, solution 1
        delayMicroseconds(1200);

        offset += chunkSize;
    }
    rc = sendCommand(0x24, 0x01, 0x00);

    Serial.println("Putting TCM to sleep");
    digitalWrite(TC_ENABLE_PIN, HIGH);
    for(;;);
}
