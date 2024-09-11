#include <WiFi.h>
char ssid[] = "luca";        // your network SSID (name)
char pass[] = "87654321";
//char server[] = "http://192.168.90.62:5000/api/sensor_data";
char server[] = "192.168.90.62";

WiFiClient client;
String apiKey = "bruh_bruh_greenhouse"; // Define your API key here


void setup() {
    Serial.begin(9600);
    while (!Serial) {
    ; // Wait for the serial port to connect
  }

  // Attempt to connect to WiFi
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);
    delay(3000);
  }
  Serial.println("Connected to WiFi");
}

void send_to_server(String lightStatus, String hoursOfLight, String pumpStatus, String humidity, String temperature, String apiKey) {
  // If connected to WiFi
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Sending data to server...");

    // Start connection to server
    if (client.connect(server, 5000)) {
      Serial.println("Connected to server");

      // Construct the POST data
      String postData = "light_status=" + lightStatus + "&hours_of_light=" + hoursOfLight + 
                        "&pump_status=" + pumpStatus + "&humidity=" + humidity + "&temperature=" + temperature;

      // Send HTTP request
      client.println("POST /api/sensor_data HTTP/1.1");
      client.println("Host: " + String(server));
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.println("API-Key: " + apiKey); // Add the API key as a custom header
      client.println("Connection: close");
      client.print("Content-Length: ");
      client.println(postData.length());
      client.println();
      client.println(postData);

      // Wait for response from the server
      while (client.connected()) {
        while (client.available()) {
          char c = client.read();
          Serial.write(c);  // Display server response
        }
      }

      // Close the connection
      client.stop();
      Serial.println("Data sent and connection closed");
    } else {
      Serial.println("Failed to connect to server");
    }
  } else {
    Serial.println("WiFi not connected");
  }
}


void loop() {
  // put your main code here, to run repeatedly:
  send_to_server("11", "22", "pumpStatus", "3", "4", apiKey);  // Sending data to server
  delay(2000);
}
