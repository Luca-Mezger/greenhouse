#include <mbed.h>
#include "Nano33BLEColour.h"
#include "GravityRtc.h"
#include "Wire.h"
using namespace mbed;
using namespace rtos;
using namespace std::literals::chrono_literals;

#include <Arduino_HTS221.h>

#define FAN_PIN D4
#define TEMPERATURE_THRESHOLD 27.00
#define BUFFER 2.00


#define SOIL_HUMIDITY_PIN A0
#define RELATIVE_SOIL_HUMIDITY_UPPER_LIMIT 699
#define RELATIVE_SOIL_HUMIDITY_LOWER_LIMIT 363
#define RELATIVE_SOIL_HUMIDITY_A -0.29761  // a and b for linear regression to map Resistance of Soil Moisture reader to percentage
#define RELATIVE_SOIL_HUMIDITY_B 208.0357
#define STABILIZATION_THRESHOLD 0.5  // threshold for detecting stabilization
#define DELAY_TIME_AVERAGES 10
#define DELAY_TIME_PUMP 2000
#define DELAY_TIME_STABILIZING_ARRAY 10000
#define HOUR 3600000
#define PUMP D11
#define LED_PIN D10
#define SOIL_MOISTURE_THRESHOLD 35
#define HUMIDITY_MOISTURE_AVERAGE_ELEMENTS 9

#define HOUR_DURATION 3600000  // 1 hour in milliseconds
#define MIN_BRIGHTNESS 6
#define MAX_BRIGHTNESS 4097
#define LIGHT_THRESHOLD_HOURS 8
#define THRESHOLD_PERCENTAGE 50  // Brightness percentage threshold

Thread pump_thread;
Thread light_sensor_thread;  // New thread for light sensor functionality
Thread temperature_thread;

// Variables for water pump/humidity
float previousAverageSoilHumidity = 0;
float differenceAverageSoilHumidity[HUMIDITY_MOISTURE_AVERAGE_ELEMENTS];
int endTime = 0;
int head = 0;   // Keeps track of the next position to insert the new element
int count = 0;  // Keeps track of the number of elements added

// Variables for light sensor
Nano33BLEColourData colourData;
GravityRtc rtc;
float brightnessHistory[24];
int hoursWithLight = 0;
float hourlyBrightnessAccumulator = 0;
int brightnessReadingsCount = 0;

// ------------------------------------------------
// WATER PUMP/HUMIDITY
// ------------------------------------------------

void addElement(float arr[], int newElement) {
  arr[head] = newElement;
  head = (head + 1) % HUMIDITY_MOISTURE_AVERAGE_ELEMENTS;
  if (count < HUMIDITY_MOISTURE_AVERAGE_ELEMENTS) {
    count++;
  }
}

float clip(float n, float lower, float upper) {
  return max(lower, min(n, upper));
}

float calculate_relative_soil_humidity(float measurement, float a = RELATIVE_SOIL_HUMIDITY_A, float b = RELATIVE_SOIL_HUMIDITY_B) {
  float clipped_measurement = clip(measurement, RELATIVE_SOIL_HUMIDITY_LOWER_LIMIT, RELATIVE_SOIL_HUMIDITY_UPPER_LIMIT);
  return a * clipped_measurement + b;
}

float get_average_soil_humidity(int numReadings = 10000) {
  float totalSoilHumidity = 0;
  for (int i = 0; i < numReadings; i++) {
    int sensorValue = analogRead(SOIL_HUMIDITY_PIN);
    totalSoilHumidity += calculate_relative_soil_humidity(sensorValue);
    ThisThread::sleep_for(DELAY_TIME_AVERAGES);
  }
  return totalSoilHumidity / numReadings;
}

float findMaxDifference(float arr[], int size = 9) {
  if (size < 2) {
    return -1;
  }
  float minValue = arr[0];
  float maxDiff = -32768;
  for (int i = 1; i < size; ++i) {
    float diff = arr[i] - minValue;
    if (diff > maxDiff) {
      maxDiff = diff;
    }
    if (arr[i] < minValue) {
      minValue = arr[i];
    }
  }
  return maxDiff;
}

bool is_stabilizing(float humidityArray[HUMIDITY_MOISTURE_AVERAGE_ELEMENTS], float threshold = STABILIZATION_THRESHOLD) {
  float maxDiff = findMaxDifference(humidityArray, HUMIDITY_MOISTURE_AVERAGE_ELEMENTS);
  return (maxDiff < threshold);
}

void onLowSoilMoisture() {
  digitalWrite(PUMP, LOW);
  ThisThread::sleep_for(DELAY_TIME_PUMP);
  digitalWrite(PUMP, HIGH);
}

void onNoWaterEffect() {
    // Serial.println("watertank empty");
}

void pump_loop() {
  while (1) {
    float averageSoilHumidity = get_average_soil_humidity();

    if (averageSoilHumidity < SOIL_MOISTURE_THRESHOLD) {
      float humidityBeforeWatering = averageSoilHumidity;  // Save humidity before watering
      int currentTime = millis();

      while (averageSoilHumidity < SOIL_MOISTURE_THRESHOLD) {
        onLowSoilMoisture();

        do {
          for (int i = 0; i < HUMIDITY_MOISTURE_AVERAGE_ELEMENTS; i++) {
            averageSoilHumidity = get_average_soil_humidity();
            addElement(differenceAverageSoilHumidity, averageSoilHumidity);
            ThisThread::sleep_for(DELAY_TIME_STABILIZING_ARRAY);
          }
        } while (!is_stabilizing(differenceAverageSoilHumidity));
      }

      // Wait for 10 minutes after watering to check humidity again
      ThisThread::sleep_for(10min);
      float humidityAfterWatering = get_average_soil_humidity();


      if (humidityAfterWatering - humidityBeforeWatering <= 1.5) {
        onNoWaterEffect();  
      }

      endTime = millis() - currentTime;
    }

    if (HOUR > endTime) {
      ThisThread::sleep_for((HOUR - endTime));
    }
    endTime = 0;
  }
}

// ------------------------------------------------
// LIGHT SENSOR
// ------------------------------------------------

float calculateBrightness(int r, int g, int b) {
  float brightness = (r + g + b) / 3.0;
  brightness = clip(brightness, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  return ((brightness - MIN_BRIGHTNESS) / (MAX_BRIGHTNESS - MIN_BRIGHTNESS)) * 100;
}

void light_sensor_loop() {
    rtc.setup();
    rtc.adjustRtc(F(__DATE__), F(__TIME__));
    Colour.begin();
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);  // Initially light is off

    while (1) {
        rtc.read();

        if (Colour.pop(colourData)) {
            float brightness = calculateBrightness(colourData.r, colourData.g, colourData.b);
            hourlyBrightnessAccumulator += brightness;
            brightnessReadingsCount++;

            // Perform hourly check
            if (brightnessReadingsCount >= (HOUR_DURATION / 10000)) {
                float averageBrightnessForHour = hourlyBrightnessAccumulator / brightnessReadingsCount;

                // shift brightness history and store the current hour's average brightness
                for (int i = 1; i < 24; i++) {
                    brightnessHistory[i - 1] = brightnessHistory[i];
                }
                brightnessHistory[23] = averageBrightnessForHour;

                // Reset
                hourlyBrightnessAccumulator = 0;
                brightnessReadingsCount = 0;

                // count: hours with sufficient light
                hoursWithLight = 0;
                for (int i = 0; i < 24; i++) {
                    if (brightnessHistory[i] > THRESHOLD_PERCENTAGE) {
                        hoursWithLight++;
                    }
                }

                int remainingHours = 24 - rtc.hour;

                // If there havent been enough light hours and remaining hours are insufficient
                if (hoursWithLight < LIGHT_THRESHOLD_HOURS) {
                    int requiredLightHours = LIGHT_THRESHOLD_HOURS - hoursWithLight;

                    float currentBrightness = calculateBrightness(colourData.r, colourData.g, colourData.b);
                    
                    // Only turn on the LED if external light is below 50%
                    if (remainingHours <= requiredLightHours && currentBrightness < THRESHOLD_PERCENTAGE) {
                        digitalWrite(LED_PIN, LOW);  // Turn on the LED 
                        // Serial.println("Light on!!!");
                    } else {
                        digitalWrite(LED_PIN, HIGH);  //turn off if remaining time is enough
                    }
                } else {
                    digitalWrite(LED_PIN, HIGH);  // Turn off the LED if enough litght
                }
            }

            ThisThread::sleep_for(10000);  // Sleep for 10 seconds
        }
    }
}


// ------------------------------------------------
// Temperature
// ------------------------------------------------
void temperature_loop() {
  while (1) {
    float temperature = HTS.readTemperature();
    // Serial.println(temperature);

    if (temperature > TEMPERATURE_THRESHOLD) {
      digitalWrite(FAN_PIN, LOW);  // Turn fan on
    } else if (temperature < TEMPERATURE_THRESHOLD - BUFFER) {
      digitalWrite(FAN_PIN, HIGH);  // Turn fan off
    }

    ThisThread::sleep_for(1800000); //30min
  }
}

void setup() {
  // Serial.begin(9600);
  
  // Initialize fan pin
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, HIGH);  // Start with fan off
  
  // Initialize temperature sensor
  if (!HTS.begin()) {
    // Serial.println("Failed to initialize humidity temperature sensor!");
    while (1);
  }

  for (int i = 0; i < HUMIDITY_MOISTURE_AVERAGE_ELEMENTS; i++) {
    differenceAverageSoilHumidity[i] = 0;
  }

  pump_thread.start(pump_loop);
  light_sensor_thread.start(light_sensor_loop);
  temperature_thread.start(temperature_loop); 
}


void loop() {
  //empty
}
