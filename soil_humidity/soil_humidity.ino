#define SOIL_HUMIDITY_PIN A0
#define RELATIVE_SOIL_HUMIDITY_UPPER_LIMIT 699
#define RELATIVE_SOIL_HUMIDITY_LOWER_LIMIT 363
#define RELATIVE_SOIL_HUMIDITY_A -0.29761
#define RELATIVE_SOIL_HUMIDITY_B 208.0357
#define STABILIZATION_THRESHOLD 0.05  // trheshold for detecting stabilization
#define DELAY_TIME 10

float previousAverageSoilHumidity = 0; 

float clip(float n, float lower, float upper) {
  return max(lower, min(n, upper));
}

float calculate_relative_soil_humidity(float measurement, float a = RELATIVE_SOIL_HUMIDITY_A, float b = RELATIVE_SOIL_HUMIDITY_B) {
  float clipped_measurement = clip(measurement, RELATIVE_SOIL_HUMIDITY_LOWER_LIMIT, RELATIVE_SOIL_HUMIDITY_UPPER_LIMIT);
  return a * clipped_measurement + b;
}

//for stabiltiy
float get_average_soil_humidity(int numReadings = 10000) {
  float totalSoilHumidity = 0;
  for (int i = 0; i < numReadings; i++) {
    int sensorValue = analogRead(SOIL_HUMIDITY_PIN);
    totalSoilHumidity += calculate_relative_soil_humidity(sensorValue);
    delay(DELAY_TIME);
  }
  return totalSoilHumidity / numReadings;
}

// calculates rate of change
bool is_stabilizing(float currentHumidity, float previousHumidity, float threshold = STABILIZATION_THRESHOLD) {
  float changeRate = abs(currentHumidity - previousHumidity);
  Serial.print("Rate of Change: ");
  Serial.println(changeRate);
  return changeRate < threshold;
}

void setup() {
  Serial.begin(9600);
}

void loop() {
  float averageSoilHumidity = get_average_soil_humidity();
  
  Serial.print("Average Soil Humidity: ");
  Serial.println(averageSoilHumidity);

  // check if moisture is stabilizing
  if (is_stabilizing(averageSoilHumidity, previousAverageSoilHumidity)) {
    Serial.println("Moisture level stabilizing.");
  } else {
    Serial.println("Moisture level changing.");
  }

  //update previous average
  previousAverageSoilHumidity = averageSoilHumidity;
}
