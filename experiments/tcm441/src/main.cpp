// Image uploading test
// TCM-P441-230-v1.0 + Pervasive display 4,41"
// Hardware SPI connected to the TCM interface
// SS, /TC_BUSY, /TC_ENABLE connected as shown below

#include <Arduino.h>
#include <SPI.h>

#include "test_pattern.h"

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
#define EP_SW_INVALID_LE                    0x6c00
#define EP_SW_WRONG_PARAMETERS_P1P2         0x6a00
#define EP_SW_INSTRUCTION_NOT_SUPPORTED     0x6d00


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

uint16_t sendCommand(uint8_t ins, uint8_t p1, uint8_t p2, uint8_t len, uint8_t *data)
{
    startTransmission();
    SPI.transfer(ins);
    SPI.transfer(p1);
    SPI.transfer(p2);

    if (len) {
        SPI.transfer(len);
        SPI.transfer(data, len);
    }
    endTransmission();
    busyWait();

    startTransmission();
    uint16_t rc = (SPI.transfer(0x00) << 8) | SPI.transfer(0x00);
    endTransmission();
    busyWait();

    return rc;
}

uint16_t sendCommand(uint8_t ins, uint8_t p1, uint8_t p2)
{
    return sendCommand(ins, p1, p2, 0, NULL);
}

void setup()
{
    SPI.begin();
    Serial.begin(115200);
    pinMode(TC_ENABLE_PIN, OUTPUT);

    Serial.println("Waking up TCM");
    digitalWrite(TC_ENABLE_PIN, LOW);
    delay(10);
    busyWait();
}

void loop()
{
    uint8_t buffer[MAX_CHUNK_SIZE];
    uint8_t chunkSize;
    uint16_t offset = 0;
    uint16_t rc;

    Serial.println("Resetting image data pointer");

    rc = sendCommand(0x20, 0x0d, 0x00);

    if (rc != EP_SW_NORMAL_PROCESSING) {
        Serial.print("Error while resetting image pointer err=");
        Serial.println(rc, HEX);
    } else {
        Serial.println("Done.");
    }

    while (offset < TOTAL_SIZE) {
        chunkSize = min(TOTAL_SIZE - offset, MAX_CHUNK_SIZE);

        memcpy_P(buffer, &test_1bit[offset], chunkSize);

        rc = sendCommand(0x20, 0x01, 0x00, chunkSize, buffer);

        if (rc != EP_SW_NORMAL_PROCESSING) {
            Serial.print("Error while sending packet offset=");
            Serial.print(offset, HEX);
            Serial.print(" err=");
            Serial.println(rc, HEX);
        }
        offset += chunkSize;
    }
    Serial.println("Refreshing screen");

    rc = sendCommand(0x24, 0x01, 0x00);

    if (rc != EP_SW_NORMAL_PROCESSING) {
        Serial.print("Error while refreshing screen err=");
        Serial.println(rc, HEX);
    } else {
        Serial.println("Done.");
    }

    Serial.println("Putting TCM to sleep");
    digitalWrite(TC_ENABLE_PIN, HIGH);
    for(;;);
}
