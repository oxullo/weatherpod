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

#include <avr/dtostrf.h>

#include "localsensor.h"

LocalSensor::LocalSensor()
{
}

void LocalSensor::begin()
{
    bme.begin();
    bme.setOperatingMode(BME280_OPERATINGMODE_FORCED);
    bme.setOversampling(BME280_OVERSAMPLING_1X, BME280_OVERSAMPLING_1X, BME280_OVERSAMPLING_1X);
}

void LocalSensor::end()
{
    bme.setOperatingMode(BME280_OPERATINGMODE_SLEEP);
}

void LocalSensor::printData()
{
    Serial.print("T=");
    Serial.print(bme.readTemperature(), 2);
    Serial.print("C P=");
    Serial.print(bme.readPressure() / 100.0, 2);
    Serial.print("Pa H=");
    Serial.print(bme.readHumidity(), 2);
    Serial.println("%");
}

void LocalSensor::getDataAsPostPayload(char *buffer)
{
    char t[6];
    char p[6];
    char h[6];

    dtostrf(bme.readTemperature(), 4, 2, t);
    dtostrf(bme.readPressure() / 100.0, 4, 2, p);
    dtostrf(bme.readHumidity(), 4, 2, h);

    sprintf(buffer, "t=%s&p=%s&h=%s", t, p, h);
}
