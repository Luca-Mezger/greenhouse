#include <mbed.h>
#include "GravityRtc.h"
#include "Wire.h"
using namespace mbed;
using namespace rtos;
using namespace std::literals::chrono_literals;
#include "Adafruit_HTU21DF.h"
#include <BH1750.h>



BH1750 lightMeter;

//temp & air humidity
Adafruit_HTU21DF htu = Adafruit_HTU21DF();

#define FAN_PIN 4
#define TEMPERATURE_THRESHOLD 27.00
#define BUFFER 2.00


#define SOIL_HUMIDITY_PIN A1
#define RELATIVE_SOIL_HUMIDITY_UPPER_LIMIT 699
#define RELATIVE_SOIL_HUMIDITY_LOWER_LIMIT 363
#define RELATIVE_SOIL_HUMIDITY_A -0.29761  // a and b for linear regression to map Resistance of Soil Moisture reader to percentage
#define RELATIVE_SOIL_HUMIDITY_B 208.0357
#define STABILIZATION_THRESHOLD 0.5  // threshold for detecting stabilization
#define DELAY_TIME_AVERAGES 10
#define DELAY_TIME_PUMP 2000
#define DELAY_TIME_STABILIZING_ARRAY 10000
#define HOUR 3600000
#define PUMP 2
#define LED_PIN 3
#define SOIL_MOISTURE_THRESHOLD 35
#define HUMIDITY_MOISTURE_AVERAGE_ELEMENTS 9

#define HOUR_DURATION 3600000  // 1 hour in milliseconds

#define MIN_BRIGHTNESS 0
#define MAX_BRIGHTNESS 40000
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
    //Serial.println("watertank empty");
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


float calculateBrightnessFromLux(float lux) {
  float brightness = clip(lux, MIN_BRIGHTNESS, MAX_BRIGHTNESS);
  return ((brightness - MIN_BRIGHTNESS) / (MAX_BRIGHTNESS - MIN_BRIGHTNESS)) * 100;
}


void light_sensor_loop() {
    while (1) {
        // Read light level from BH1750 sensor
        float lux = lightMeter.readLightLevel();
        float brightness = calculateBrightnessFromLux(lux);

        Serial.println(lux);
        // Accumulate brightness values for the hour
        hourlyBrightnessAccumulator += brightness;
        brightnessReadingsCount++;

        // Perform hourly check
        if (brightnessReadingsCount >= (HOUR_DURATION / 10000)) {
            float averageBrightnessForHour = hourlyBrightnessAccumulator / brightnessReadingsCount;

            // Shift brightness history and store the current hour's average brightness
            for (int i = 1; i < 24; i++) {
                brightnessHistory[i - 1] = brightnessHistory[i];
            }
            brightnessHistory[23] = averageBrightnessForHour;

            // Reset hourly accumulator
            hourlyBrightnessAccumulator = 0;
            brightnessReadingsCount = 0;

            // Count the hours with sufficient light
            hoursWithLight = 0;
            for (int i = 0; i < 24; i++) {
                if (brightnessHistory[i] > THRESHOLD_PERCENTAGE) {
                    hoursWithLight++;
                }
            }

            // Determine if the LED needs to be turned on for additional light
            int remainingHours = 24 - rtc.hour;
            if (hoursWithLight < LIGHT_THRESHOLD_HOURS) {
                int requiredLightHours = LIGHT_THRESHOLD_HOURS - hoursWithLight;
                if (remainingHours <= requiredLightHours && brightness < THRESHOLD_PERCENTAGE) {
                    digitalWrite(LED_PIN, HIGH);  // Turn on the LED
                } else {
                    digitalWrite(LED_PIN, LOW);  // Turn off the LED
                }
            } else {
                digitalWrite(LED_PIN, LOW);  // Turn off the LED if sufficient light
            }
        }

        ThisThread::sleep_for(10000);  // Sleep for 10 seconds
    }
}



// ------------------------------------------------
// Temperature
// ------------------------------------------------
void temperature_loop() {
  while (1) {
    float temperature = htu.readTemperature(); //in Degree Celcius
    //Serial.println(temperature);

    if (temperature > TEMPERATURE_THRESHOLD) {
      digitalWrite(FAN_PIN, LOW);  // Turn fan on
    } else if (temperature < TEMPERATURE_THRESHOLD - BUFFER) {
      digitalWrite(FAN_PIN, HIGH);  // Turn fan off
    }

    ThisThread::sleep_for(1800000); //30min
  }
}

void setup() {
  Serial.begin(9600);

  // for light sensor
  Wire.begin();
  lightMeter.begin();
  
  // Initialize fan pin
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, HIGH);  // Start with fan off
  


  for (int i = 0; i < HUMIDITY_MOISTURE_AVERAGE_ELEMENTS; i++) {
    differenceAverageSoilHumidity[i] = 0;
  }

    //Temperature
    if (!htu.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }

  pump_thread.start(pump_loop);
  light_sensor_thread.start(light_sensor_loop);
  temperature_thread.start(temperature_loop); 
}


void loop() {
  //empty
}
