#include <WiFi.h>
#include <HTTPClient.h>

// Pin Configuration
//const int testPins[] = {0, 2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};  
const int testPins[] = {2, 4, 5, 12, 13, 14, 15, 16, 17, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33};  
const int total_pins = sizeof(testPins) / sizeof(testPins[0]);

// Wi-Fi Credentials
//const char* ssid = "MMA:PTCL:F1";
const char* ssid = "MMA:T:2F";
const char* password = "a5d5m0i5n5";

// Arduino Web Server IP
const char* serverIP = "192.168.10.222";  // IP of Arduino Uno

void setup() {
  Serial.begin(115200);

  // Set all pins as INPUT_PULLUP
  for (int i = 0; i < total_pins; i++) {
    pinMode(testPins[i], INPUT_PULLUP);
  }

  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi...");
  int timeout = 20;  // 20 attempts (10 seconds)
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(500);
    Serial.print(".");
    timeout--;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWi-Fi Connected!");
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to Wi-Fi.");
  }
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Read all pin states
    String pinStates = "";
    for (int i = 0; i < total_pins; i++) {
      pinStates += digitalRead(testPins[i]);
      if (i < total_pins - 1) pinStates += ",";
    }

    Serial.print("Pin States: ");
    Serial.println(pinStates);

    // Send pin states to Arduino via HTTP
    HTTPClient http;
    String url = String("http://") + serverIP + "/update-distance?value=" + pinStates;
    http.begin(url);  // Configure the request

    int httpResponseCode = http.GET();  // Send the request

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response Code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Error Sending Data: ");
      Serial.println(httpResponseCode);
    }
    http.end();  // Close connection
  } else {
    Serial.println("Wi-Fi Disconnected!");
  }

  delay(1000);  // Wait before sending the next request
}