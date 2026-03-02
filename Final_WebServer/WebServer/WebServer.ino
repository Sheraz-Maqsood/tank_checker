#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
//#include <NewPing.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//IPAddress ip(169, 254, 110, 100); // Static IP for Arduino
IPAddress ip(192, 168, 10, 222); // Static IP for Arduino
//IPAddress gateway(169, 254, 110, 1); // Gateway is the PC
//IPAddress subnet(255, 255, 0, 0); // Subnet mask

//byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
EthernetServer server(80);  // Web server running on port 80
File webFile;
//IPAddress ip(192, 168, 10, 222);  
//#define TRIGGER_PIN 7
//#define ECHO_PIN 6
//#define MAX_DISTANCE 400 // Maximum distance in cm
//NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);
String latestTankLevel = "";  // To store pin states from ESP32


void setup() {
  // Start the serial monitor
  Serial.begin(9600);

 // Initialize SD card
  if (!SD.begin(4)) {
    Serial.println("SD card failed to initialize.");
    while (true);  // Halt further execution
  } else {
    Serial.println("SD card initialized.");
  }

  //Initialize Ethernet using DHCP
  // if (Ethernet.begin(mac) == 0) {
  //   Serial.println("Ethernet shield failed to initialize.");
  //   while (true);  // Halt further execution
  // }else {
  //   Serial.println("Ethernet shield initialized.");
  // }
  //Ethernet.begin(mac, ip, gateway, subnet);
  // Use static IP address
  Ethernet.begin(mac, ip);
  // Start the server
  server.begin();
  Serial.print("Server is at: ");
  Serial.println(Ethernet.localIP());  // Show the IP address assigned via DHCP
}

void loop() {
  EthernetClient client = server.available();
  if (client) {
    Serial.println("Client connected.");
    String request = ""; // To store the HTTP request line
    
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


    delay(1); // Give the client time to receive the data
    client.stop(); 
    Serial.println("Client disconnected.");
    // Close the connection
    // client.println("HTTP/1.1 200 OK");
    // client.println("Content-Type: text/html");
    // client.println("Connection: close");
    // client.println();
    // client.println("<html><body><h1>Hello from Arduino Web Server!</h1></body></html>");
    // client.stop();
  }
}
void serveHtml(EthernetClient &client)
{
  webFile = SD.open("INDEX.HTM");  // Try the long filename first
    if (!webFile) {
      // If the long filename fails, try the short filename
      webFile = SD.open("INDEX~1.HTM");  
    }
    if (!webFile) {
      // If the long filename fails, try the short filename
      webFile = SD.open("index.html");  
    }
    if (webFile) {
      client.println("HTTP/1.1 200 OK");
      client.println("Content-Type: text/html");
      client.println("Connection: close");
      client.println();

      // Send the content of the file
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
  Serial.print("Serving Tank Level: ");
  Serial.println(latestTankLevel);
  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: application/json");
  client.println("Access-Control-Allow-Origin: *");
  client.println("Connection: close");
  client.println();
  
  // Send JSON response with correctly quoted string
  client.print("{\"tank_level\":\"");
  client.print(latestTankLevel);
  client.println("\"}");
}

void updateTankLevel(String request, EthernetClient &client) {
  // Parse the 'value' parameter from the request
  Serial.println("updateTankLevel Called");
  int valueIndex = request.indexOf("value=");
  if (valueIndex >= 0) {
    String valueString = request.substring(valueIndex + 6);
    
    // The HTTP request line ends with " HTTP/1.1", so we need to trim that suffix off
    int spaceIndex = valueString.indexOf(" ");
    if (spaceIndex >= 0) {
      valueString = valueString.substring(0, spaceIndex);
    }
    
    latestTankLevel = valueString;
    Serial.print("Updated Tank Level: ");
    Serial.println(latestTankLevel);
  }

  // Send HTTP response
  client.println("HTTP/1.1 200 OK");
  client.println("Connection: close");
  client.println();
  client.println("Distance updated successfully");
}
