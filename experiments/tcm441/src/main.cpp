#include <Arduino.h>
#include <SPI.h>

#include "test_pattern.h"

#define MAX_CHUNK_SIZE      0xfa
#define SS_PIN              10
#define TC_BUSY_PIN         2
#define TC_ENABLE_PIN       3


SPISettings spiSettings(1000000, MSBFIRST, SPI_MODE3);


uint8_t sendCommand(uint8_t ins, uint8_t p1, uint8_t p2, uint8_t len, uint8_t *data)
{
    digitalWrite(SS_PIN, LOW);
    delay(1);
    SPI.beginTransaction(spiSettings);
    SPI.transfer(ins);
    SPI.transfer(p1);
    SPI.transfer(p2);

    if (len) {
        SPI.transfer(len);
        SPI.transfer(data, len);
    }
    SPI.endTransaction();
    delay(1);
    digitalWrite(SS_PIN, HIGH);

    delay(1);
    while(digitalRead(TC_BUSY_PIN) == LOW);
    delay(1);

    digitalWrite(SS_PIN, LOW);
    delay(1);
    SPI.beginTransaction(spiSettings);
    uint8_t rc = SPI.transfer(0x00);
    SPI.transfer(0x00);
    SPI.endTransaction();
    delay(1);
    digitalWrite(SS_PIN, HIGH);

    delay(1);
    while(digitalRead(TC_BUSY_PIN) == LOW);
    delay(1);

    return rc;
}

uint8_t sendCommand(uint8_t ins, uint8_t p1, uint8_t p2)
{
    return sendCommand(ins, p1, p2, 0, NULL);
}

void setup()
{
    SPI.begin();
    Serial.begin(115200);
    pinMode(TC_ENABLE_PIN, OUTPUT);
    digitalWrite(TC_ENABLE_PIN, LOW);
    delay(10);
}

void loop()
{
  const unsigned char *ptr = test_1bit;
  uint8_t buffer[MAX_CHUNK_SIZE];
  uint8_t chunkSize;
  uint16_t cazzo = 0;

  Serial.println(sendCommand(0x20, 0x0d, 0x00), HEX);

  while (cazzo < TOTAL_SIZE) {
    chunkSize = min(TOTAL_SIZE - cazzo, MAX_CHUNK_SIZE);

    memcpy_P(buffer, ptr, chunkSize);

    uint8_t rc = sendCommand(0x20, 0x01, 0x00, chunkSize, buffer);

    ptr += chunkSize;
    cazzo += chunkSize;

    Serial.print(cazzo);
    Serial.print(" ");
    Serial.print(chunkSize);
    Serial.print(" ");
    Serial.println(rc, HEX);
 }
  Serial.println("--");

  Serial.println(sendCommand(0x24, 0x01, 0x00), HEX);

  digitalWrite(TC_ENABLE_PIN, HIGH);
  for(;;);
}
