#include <Arduino.h>
// #include <RTCZero.h>
#include "mkrpm.h"

#define LED_PIN     6
#define LED2_PIN    1

// RTCZero rtc;
MkrPM pm;
uint16_t iter = 0;


void blink()
{
    digitalWrite(LED_PIN, HIGH);
    delay(200);
    digitalWrite(LED_PIN, LOW);
    delay(1000);
}

void blink2(uint16_t times)
{
    for (uint16_t i = 0 ; i < times ; ++i) {
        digitalWrite(LED2_PIN, HIGH);
        delay(20);
        digitalWrite(LED2_PIN, LOW);
        delay(200);
    }
}

void setup()
{
    delay(2000);
    pinMode(LED_PIN, OUTPUT);
    pinMode(LED2_PIN, OUTPUT);

    blink();

    // rtc.begin();
    // rtc.setAlarmSeconds(0);
    // rtc.enableAlarm(rtc.MATCH_SS);
    pm.begin();
}

void loop()
{
    blink();
    blink2(++iter);
    // rtc.standbyMode();
    USBDevice.detach();
    pm.sleepForMinutes(1);

    USBDevice.attach();
    delay(2000);
    // while(!SerialUSB);
    // SerialUSB.println("awaken");
}
