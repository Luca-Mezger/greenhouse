#include "Arduino.h"
#include "Nano33BLEColour.h"
#include "GravityRtc.h"
#include "Wire.h"

#define HOUR_DURATION 10000  // 1 hour in milliseconds

#define MIN_BRIGHTNESS 6
#define MAX_BRIGHTNESS 4097
#define LIGHT_THRESHOLD_HOURS 10
#define LED_PIN D3
#define THRESHOLD_PERCENTAGE 50  // Brightness percentage threshold

Nano33BLEColourData colourData;
GravityRtc rtc;

float brightnessHistory[24];
int hoursWithLight = 0;

float hourlyBrightnessAccumulator = 0;
int brightnessReadingsCount = 0;

float clip(float n, float lower, float upper) {
    return max(lower, min(n, upper));
}

float calculateBrightness(int r, int g, int b) {
    float brightness = (r + g + b) / 3.0;
    brightness = clip(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
    return ((brightness - MIN_BRIGHTNESS) / (MAX_BRIGHTNESS - MIN_BRIGHTNESS)) * 100;
}

void setup() {
    Serial.begin(115200);
    while (!Serial);

    rtc.setup();
    rtc.adjustRtc(F(__DATE__), F(__TIME__));  // Set RTC time to your computer time

    Colour.begin();
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // Initially, the light is off
}

void loop() {
    rtc.read();

    if (Colour.pop(colourData)) {
        float brightness = calculateBrightness(colourData.r, colourData.g, colourData.b);
        hourlyBrightnessAccumulator += brightness;
        brightnessReadingsCount++;

        if (brightnessReadingsCount >= (HOUR_DURATION / 1000)) {  // Assuming one reading per second
            float averageBrightnessForHour = hourlyBrightnessAccumulator / brightnessReadingsCount;

            for (int i = 1; i < 24; i++) {
                brightnessHistory[i - 1] = brightnessHistory[i];
            }
            brightnessHistory[23] = averageBrightnessForHour;

            hourlyBrightnessAccumulator = 0;
            brightnessReadingsCount = 0;

            hoursWithLight = 0;
            for (int i = 0; i < 24; i++) {
                if (brightnessHistory[i] > THRESHOLD_PERCENTAGE) {
                    hoursWithLight++;
                }
            }

            int remainingHours = 24 - rtc.hour;
            Serial.println(remainingHours);

            if (hoursWithLight < LIGHT_THRESHOLD_HOURS) {
                int requiredLightHours = LIGHT_THRESHOLD_HOURS - hoursWithLight;
                if (remainingHours <= requiredLightHours) {
                    digitalWrite(LED_PIN, LOW);  // Turn on light
                    Serial.println("Light on!!!");

                } else {
                    digitalWrite(LED_PIN, HIGH);  // Turn off light
                }
            } else {
                digitalWrite(LED_PIN, HIGH);  // Turn off light if enough light
            }
        }

        delay(1000);  // Delay for 1 second for each sensor reading
    }

    Serial.print("current time - hour: ");
    Serial.print(rtc.hour);
    Serial.print(", minute: ");
    Serial.println(rtc.minute);
}
