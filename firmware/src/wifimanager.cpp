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

#include "wifimanager.h"

WiFiManager::WiFiManager()
{

}

void WiFiManager::setup(const char *wifiSsid, const char *wifiPsk)
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
    printStatus();
}

void WiFiManager::printStatus()
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
