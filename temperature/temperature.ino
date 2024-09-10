#include <Arduino_HTS221.h>

#define FAN_PIN D4
#define TEMPERATURE_THRESHOLD 27.00
#define BUFFER 2.00

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  if (!HTS.begin()) {
    Serial.println("Failed to initialize humidity temperature sensor!");
    while (1)
      ;
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  float temperature = HTS.readTemperature()

  Serial.println(temperature);
  if (temperature > TEMPERATURE_THRESHOLD) {
    digitalWrite(FAN_PIN, LOW);
  } else if (temperature < TEMPERATURE_THRESHOLD - BUFFER) {
    digitalWrite(FAN_PIN, HIGH);
  }
  delay(1800000);
}
