#include <Arduino.h>

#include "batteryprobe.h"

#define NOMINAL_VCC_MV          3300.0
#define VOLTAGEDIVIDER_FACTOR   2.0

BatteryProbe::BatteryProbe(uint8_t input_pin_, uint8_t enable_pin_) :
    input_pin(input_pin_),
    enable_pin(enable_pin_)
{
}

uint16_t BatteryProbe::getVoltage()
{
    pinMode(enable_pin, OUTPUT);
    digitalWrite(enable_pin, LOW);
    delay(10);

    uint16_t adcval = analogRead(input_pin);

    pinMode(input_pin, INPUT);
    pinMode(enable_pin, INPUT);

    return (uint16_t)((float)adcval / 1024.0 * NOMINAL_VCC_MV * VOLTAGEDIVIDER_FACTOR);
}
