#ifndef BATTERYPROBE_H
#define BATTERYPROBE_H

#include <stdint.h>

class BatteryProbe {
public:
    BatteryProbe(uint8_t input_pin_, uint8_t enable_pin_);
    uint16_t getVoltage();

private:
    uint8_t input_pin;
    uint8_t enable_pin;
};

#endif
