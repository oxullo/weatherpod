/*
  Copyright (c) 2016, OXullo Intersecans <x@brainrapers.org>
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:
      * Redistributions of source code must retain the above copyright
        notice, this list of conditions and the following disclaimer.
      * Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.
      * Neither the name of the <organization> nor the
        names of its contributors may be used to endorse or promote products
        derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
  ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "FastLED.h"

FASTLED_USING_NAMESPACE;

#define LEDS_NUM            144
#define LEDS_BRIGHTNESS     32

#define ONBOARD_LED_PIN             D7
#define ONBOARD_LED_BLINK_TIME_MS   50

#define DATA_PIN        A5
#define CLOCK_PIN       A3

#define MIN_TEMP        5
#define MAX_TEMP        26

#define MIN_HUE_DEG     180
#define MAX_HUE_DEG     360

#define REQUEST_PERIOD_MS       60000 * 5
#define GLITTER_PERIOD_MS       20000

#define GLITTER_STEP_PERIOD_MS      5
#define TEASER_STEP_PERIOD_MS       5

#define WEBHOOK_NAME       "get_local_temperature"


#define MIN_HUE_8BIT    255.0 * MIN_HUE_DEG / 360.0
#define MAX_HUE_8BIT    255.0 * MAX_HUE_DEG / 360.0

CRGB leds[LEDS_NUM];

uint32_t tsLastRequest = 0;
uint32_t tsLastGlitter = 0;


float fMap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Set the light bar to a level between [0:LEDS_NUM]
// the hue changes linearly with the height
void setLevel(uint8_t level)
{
    for (uint8_t i = 0 ; i < LEDS_NUM ; ++i) {
        if (i < level) {
            leds[i] = CHSV((uint8_t) fMap(i, 0, LEDS_NUM, MIN_HUE_8BIT, MAX_HUE_8BIT), 255, 255);
        } else {
            leds[i] = CRGB::Black;
        }
    }
    FastLED.show();
}

// Stripe tester which runs at startup
void playTeaser()
{
    for (uint8_t i = 0 ; i < LEDS_NUM ; ++i) {
        setLevel(i);
        delay(TEASER_STEP_PERIOD_MS);
    }

    for (uint8_t i = LEDS_NUM - 1 ; i > 0 ; --i) {
        setLevel(i);
        delay(TEASER_STEP_PERIOD_MS);
    }
}

// Activity monitor
void pulseOnboardLed()
{
    digitalWrite(ONBOARD_LED_PIN, LOW);
    delay(ONBOARD_LED_BLINK_TIME_MS);
    digitalWrite(ONBOARD_LED_PIN, HIGH);
}

// Called when the webhook returns a payload
// such payload is the result of a moustache parsing of the original json content
// returned by the weather service
void onTemperatureUpdate(const char *name, const char *data)
{
    String str = String(data);

    if (str.length() > 0) {
        float temp = str.toFloat();
        uint8_t target = (uint8_t) fMap(temp, MIN_TEMP, MAX_TEMP, 0, LEDS_NUM);

        Serial.println("The temp is: " + String(temp));
        Serial.println("Target: " + String(target));

        setLevel(target);
    }
}

// Periodically called to fire up a webhook event
// once the event calls back, data is passed to onTemperatureUpdate()
void requestNewTemp()
{
    pulseOnboardLed();

    Serial.println("Requesting temperature update");
    Particle.publish(WEBHOOK_NAME);
}

// Pure aesthetic effect which runs periodically
void glitter()
{
    CRGB old;

    for (uint8_t i = 0 ; i < LEDS_NUM ; ++i) {
        if (i != 0) {
            leds[i - 1] = old;
        }
        old = leds[i];

        if (!leds[i]) {
            FastLED.show();
            return;
        }
        leds[i] = CRGB::White;
        FastLED.show();

        delay(GLITTER_STEP_PERIOD_MS);
    }
}

//
// setup + loop
//

void setup()
{
    Serial.begin(115200);

    Particle.subscribe("hook-response/" WEBHOOK_NAME, onTemperatureUpdate, MY_DEVICES);

    // Initialize FastLED library for a 144-leds APA102 stripe (dotstar)
    // reduce the clock rate to 4MHz
    FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR, DATA_RATE_MHZ(4)>(leds, LEDS_NUM);
    FastLED.setBrightness(LEDS_BRIGHTNESS);

    pinMode(ONBOARD_LED_PIN, OUTPUT);
    pulseOnboardLed();

    playTeaser();
    requestNewTemp();
}

void loop()
{
    if (millis() - tsLastRequest > REQUEST_PERIOD_MS) {
        requestNewTemp();
        tsLastRequest = millis();
    }

    if (millis() - tsLastGlitter > GLITTER_PERIOD_MS) {
        glitter();
        tsLastGlitter = millis();
    }
}
