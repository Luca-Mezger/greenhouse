#define SOIL_HUMIDITY_PIN A0
#define RELATIVE_SOIL_HUMIDITY_UPPER_LIMIT 699
#define RELATIVE_SOIL_HUMIDITY_LOWER_LIMIT 363
#define RELATIVE_SOIL_HUMIDITY_A -0.29761
#define RELATIVE_SOIL_HUMIDITY_B 208.0357

float clip(float n, float lower, float upper) {
  return max(lower, min(n, upper));
}

float calculate_relative_soil_humidity(float measurement, float a = RELATIVE_SOIL_HUMIDITY_A, float b = RELATIVE_SOIL_HUMIDITY_B) {
  float clipped_measurement = clip(measurement, RELATIVE_SOIL_HUMIDITY_LOWER_LIMIT, RELATIVE_SOIL_HUMIDITY_UPPER_LIMIT);
  return a * clipped_measurement + b;
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  float totalSoilHumidity = 0;

  for (int i = 0; i < 100; i++) {
    int sensorValue = analogRead(SOIL_HUMIDITY_PIN);
    totalSoilHumidity += calculate_relative_soil_humidity(sensorValue);
    delay(100); 
  }

  float averageSoilHumidity = totalSoilHumidity / 100.0;
  Serial.println(averageSoilHumidity);
}
