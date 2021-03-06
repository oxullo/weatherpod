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

#ifndef LOCALSENSOR_H
#define LOCALSENSOR_H

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>

class LocalSensor {
public:
    LocalSensor();
    void begin();
    void end();
    void printData();
    void getDataAsPostPayload(char *buffer);

private:
    Adafruit_BME280 bme;
};

#endif
