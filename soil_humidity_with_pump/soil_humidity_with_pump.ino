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
#define SOIL_MOISTURE_THRESHOLD 35
#define HUMIDITY_MOISTURE_AVERAGE_ELEMENTS 9

float previousAverageSoilHumidity = 0;
float differenceAverageSoilHumidity[HUMIDITY_MOISTURE_AVERAGE_ELEMENTS];
int endTime = 0;


void setup() {
  Serial.begin(9600);
  for (int i = 0; i < HUMIDITY_MOISTURE_AVERAGE_ELEMENTS; i++) {
    differenceAverageSoilHumidity[i] = 0;
  }
}

int head = 0;   // Keeps track of the next position to insert the new element
int count = 0;  // Keeps track of the number of elements added

// Function to add a new element and overwrite the oldest if necessary
void addElement(float arr[], int newElement) {
  arr[head] = newElement;                                  // Add new element at the 'head' position
  head = (head + 1) % HUMIDITY_MOISTURE_AVERAGE_ELEMENTS;  // Move 'head' to the next position (circularly)

  // Keep count of how many elements are filled (useful for printing)
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

//for stabiltiy
float get_average_soil_humidity(int numReadings = 10000) {
  float totalSoilHumidity = 0;
  for (int i = 0; i < numReadings; i++) {
    int sensorValue = analogRead(SOIL_HUMIDITY_PIN);
    totalSoilHumidity += calculate_relative_soil_humidity(sensorValue);
    delay(DELAY_TIME_AVERAGES);
  }
  return totalSoilHumidity / numReadings;
}

float findMaxDifference(float arr[], int size = 9) {
  if (size < 2) {
    return -1;  // Return -1 if there are not enough elements to find a difference
  }

  float minValue = arr[0];  // Initialize the minimum value as the first element
  float maxDiff = -32768;   // Initialize the maximum difference (using a small value)

  // Traverse the array starting from the second element
  for (int i = 1; i < size; ++i) {
    float diff = arr[i] - minValue;  // Calculate the difference

    // Update maxDiff if the current difference is larger
    if (diff > maxDiff) {
      maxDiff = diff;
    }

    // Update minValue if the current element is smaller
    if (arr[i] < minValue) {
      minValue = arr[i];
    }
  }

  return maxDiff;
}

// calculates rate of change
bool is_stabilizing(float humidityArray[HUMIDITY_MOISTURE_AVERAGE_ELEMENTS], float threshold = STABILIZATION_THRESHOLD) {
  float maxDiff = findMaxDifference(humidityArray, HUMIDITY_MOISTURE_AVERAGE_ELEMENTS);
  return (maxDiff < threshold);
}


// Function to call when soil moisture is below the threshold
void onLowSoilMoisture() {
  Serial.println("Soil moisture is below threshold! pumping...");
  digitalWrite(PUMP, LOW);
  delay(DELAY_TIME_PUMP);
  digitalWrite(PUMP, HIGH);
}

void loop() {
  float averageSoilHumidity = get_average_soil_humidity();

  Serial.print("Average Soil Humidity: ");
  Serial.println(averageSoilHumidity);

  // Check if moisture is below threshold
  if (averageSoilHumidity < SOIL_MOISTURE_THRESHOLD) {
    int currentTime = millis();

    while (averageSoilHumidity < SOIL_MOISTURE_THRESHOLD) {

      onLowSoilMoisture();

      do {
        for (int i = 0; i < HUMIDITY_MOISTURE_AVERAGE_ELEMENTS; i++) {
          averageSoilHumidity = get_average_soil_humidity();
          addElement(differenceAverageSoilHumidity, averageSoilHumidity);
          delay(DELAY_TIME_STABILIZING_ARRAY);
        }
      } while (!is_stabilizing(differenceAverageSoilHumidity));
    }

    endTime = millis() - currentTime;
  }

  if (HOUR > endTime) {
    delay(HOUR - endTime);
  } else {
    delay(0);
  }
  endTime = 0;

  // Update previous average
  previousAverageSoilHumidity = averageSoilHumidity;
}
