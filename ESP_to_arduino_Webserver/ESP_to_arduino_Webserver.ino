#include <WiFi.h>
#include <HTTPClient.h>
#include <NewPing.h>

// Sonar Pins
#define TRIGGER_PIN 32
#define ECHO_PIN 33
#define MAX_DISTANCE 200

// Wi-Fi Credentials
const char* ssid = "PTCL-BB";
const char* password = "03008537437";

// Arduino Web Server IP
const char* serverIP = "192.168.10.222";  // IP of Arduino Uno

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

void setup() {
  Serial.begin(9600);

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
    unsigned int distance = sonar.ping_cm();  // Measure distance
    Serial.print("Measured Distance: ");
    Serial.print(distance);
    Serial.println(" cm");

    Serial.println("HTTP Request is going to send");
    // Send distance to Arduino via HTTP
    HTTPClient http;
    String url = String("http://") + serverIP + "/update-distance?value=" + distance;
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

  delay(3000);  // Wait before sending the next request
}
