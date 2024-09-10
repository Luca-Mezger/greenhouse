#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
void setup() {
  // put your setup code here, to run once:
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {  // Address 0x3D

#ifdef PC_CONNECTED
    Serial.println(F("SSD1306 allocation failed"));
#endif
    for (;;)
      ;
  }

  
}

void loop() {
  // put your main code here, to run repeatedly:
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.print("Humidity ");
  display.print(averageSoilHumidity);
  display.println("%");
  $

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("Pump status:");
  if (isPumping) {
    display.print(" is pumping...");
  } else {
    display.print(HOUR - endTime + "h left");
  }
  display.display();

// something to delay time
  //Temperature
    display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(temperature + "°C");

  //Light

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.print("Light status:");
  if (isGettingLight) {
    display.print(" lit");
  } else {
    display.print(" dark times");
  }

  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 20);
  display.print("Hours of light:");
  display.print(hoursWithLight);
  
}
