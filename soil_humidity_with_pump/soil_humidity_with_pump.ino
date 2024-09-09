#define SOIL_HUMIDITY_PIN A0
#define RELATIVE_SOIL_HUMIDITY_UPPER_LIMIT 699
#define RELATIVE_SOIL_HUMIDITY_LOWER_LIMIT 363
#define RELATIVE_SOIL_HUMIDITY_A -0.29761 //a and b for linear regression to map Resistance of Soil Moisture reader to percentage
#define RELATIVE_SOIL_HUMIDITY_B 208.0357
#define STABILIZATION_THRESHOLD 0.5  // threshold for detecting stabilization
#define DELAY_TIME 10
#define PUMP D11
#define SOIL_MOISTURE_THRESHOLD 35

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


// Function to call when soil moisture is below the threshold
void onLowSoilMoisture() {
  Serial.println("Soil moisture is below threshold! pumping...");
  digitalWrite(PUMP, LOW);
  delay(2000);
  digitalWrite(PUMP, HIGH);
}

void loop() {
  float averageSoilHumidity = get_average_soil_humidity();
  
  Serial.print("Average Soil Humidity: ");
  Serial.println(averageSoilHumidity);

  // Check if moisture is below threshold
  if (averageSoilHumidity < SOIL_MOISTURE_THRESHOLD) {
    onLowSoilMoisture();
  }

  // Check if moisture is stabilizing
  if (is_stabilizing(averageSoilHumidity, previousAverageSoilHumidity)) {
    Serial.println("Moisture level stabilizing.");
    
    // If stabilizing, check moisture level again
    if (averageSoilHumidity < SOIL_MOISTURE_THRESHOLD) {
      onLowSoilMoisture(); 
    } else {
      Serial.println("Soil moisture is above threshold, good");
    }
  } else {
    Serial.println("Moisture level changing. still waiting whether to pump again");
  }

  // Update previous average
  previousAverageSoilHumidity = averageSoilHumidity;
}

