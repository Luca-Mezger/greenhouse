# Automatic Greenhouse

The goal for this project week was to make a fully functioning automated greenhouse, where all funtions that would need manual work, would be able to be done by the greenhouse.

## Arduino

We used Arduino a hardware and software platform. Arduino has these mini computers. Where not a lot of space is needed and it is possible to code what they do.


## Todo
- [ ] Detection Water Supply (empty or not)
- [ ] Temperature Sensor/Fan
- [ ] Multithreading
- [ ] Build Greenhouse
- [ ] Display
- [ ] Smartphone Connection
- [ ] Poster
- [ ] Report



## Work journal
### 09 September 2024

#### Decided on materials needed:
  - Arduino Nano 33 Sense BLE (contains various sensors)
  - Soil Moisture Sensor - to detect water level in soil
  - LED light strips - to supply plant with needed light
  - Water pump - to supply plant with water
  - Voltage Regulator - water pump needs more voltage than what the Nano can supply
  - Relais - works like a light switch

#### Calibrate humidity sensor
1. Figure out the output value in completely dry environment (here: $363$)
2. Figure out the output value while submerged in water (here: $699$)

These values correspond to 0% humidity and 100% humidity:
   Create a linear function that maps these values:
   $f(x) = mx + b = -0.29761x + 208.0357$

#### Relay
A relay is like a remote control switch that lets a small electrical signal turn a bigger device on or off. It allows you to control things like motors or lights using just a small amount of power, keeping the control system safe and separate from the high power used by the device. We use it to power our water pump and the LED strips.

#### Voltage Regulator
With a voltage regulator, we can convert an input voltage of 3.3 V into 12 V. We use it to power the LED strips, as they need 12 V to function.

#### Humidity Logic

- Check moisture level
- If moisture level below threshold, pump water
- Check again once level is stable


 

```c++
  float averageSoilHumidity = get_average_soil_humidity();

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
```

#### LED Logic
- Initializes color sensor and RTC.
- Calculates hourly average of  brightness, updates 24-hour history.
- Tracks hours with brightness above threshold.
- If light hours < required and remaining daylight insufficient, turns on LED.
  
```c++
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

            if (hoursWithLight < LIGHT_THRESHOLD_HOURS) {
                int requiredLightHours = LIGHT_THRESHOLD_HOURS - hoursWithLight;
                if (remainingHours <= requiredLightHours) {
                    digitalWrite(LED_PIN, LOW);  // Turn on light
                } else {
                    digitalWrite(LED_PIN, HIGH);  // Turn off light
                }
            } else {
                digitalWrite(LED_PIN, HIGH);  // Turn off light if enough light
            }
        }

        delay(1000);  // Delay for 1 second for each sensor reading
    }
```

#### Temperature Logic

#### Display Logic
