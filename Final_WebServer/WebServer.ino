#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetServer server(80);  // Web server running on port 80
File webFile;
IPAddress ip(192, 168, 10, 222);  
String latestPinStates = "";  // To store pin states from ESP32
unsigned long lastUpdateMillis = 0; // Tracks the last time data was received
bool hasReceivedData = false;

void setup() {
  Serial.begin(9600);

  // Initialize SD card
  if (!SD.begin(4)) {
    Serial.println("SD card failed to initialize.");
    while (true);
  } else {
    Serial.println("SD card initialized.");
  }

  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("Server is at: ");
  Serial.println(Ethernet.localIP());
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Client connected.");
    String request = "";
    
    // Read the first line of the request
    char c;
    while (client.available() && (c = client.read()) != '\n') {
      request += c;
    }

    // Handle the request based on the URL
    if (request.indexOf("GET /tank-level") >= 0) {
      serveTankLevel(client);
    } else if (request.indexOf("GET /update-distance") >= 0) {
      updateTankLevel(request, client);
    } else {
      serveHtml(client);
    }

    delay(1);
    client.stop(); 
    Serial.println("Client disconnected.");
  }
}

void serveHtml(EthernetClient &client) {
  webFile = SD.open("INDEX.HTM");
  if (!webFile) {
    webFile = SD.open("INDEX~1.HTM");  
  }
  if (!webFile) {
    webFile = SD.open("index.html");  
  }
  
  if (webFile) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();

    while (webFile.available()) {
      client.write(webFile.read());
    }

    webFile.close();
  } else {
    Serial.println("Failed to open INDEX.HTM");
    client.println("HTTP/1.1 404 Not Found");
    client.println("Connection: close");
    client.println();
  }
}

void serveTankLevel(EthernetClient &client) {
  Serial.print("Serving Pin States: ");
  Serial.println(latestPinStates);
  
  // If data was received recently (within 5 seconds), mark as live
  bool isLive = hasReceivedData && (millis() - lastUpdateMillis < 5000);
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Connection: close");
  client.println();
  
  // Send JSON response
  client.print("{\"tank_level\":\"");
  client.print(latestPinStates);
  client.print("\",\"is_live\":");
  client.print(isLive ? "true" : "false");
  client.println("}");
}

void updateTankLevel(String request, EthernetClient &client) {
  // Parse the 'value' parameter from the request
  int valueIndex = request.indexOf("value=");
  if (valueIndex >= 0) {
    String valueString = request.substring(valueIndex + 6);
    // Extract up to the first space or question mark
    valueString = valueString.substring(0, valueString.indexOf(' '));
    latestPinStates = valueString;
    lastUpdateMillis = millis();
    hasReceivedData = true;
    Serial.print("Updated Pin States: ");
    Serial.println(latestPinStates);
  }

  // Send HTTP response
  client.println("HTTP/1.1 200 OK");
  client.println("Connection: close");
  client.println();
  client.println("Pin states updated successfully");
}
