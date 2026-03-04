#include <SPI.h>
#include <SD.h>

const int chipSelect = 4; // CS pin for Ethernet Shield SD card

void setup() {
  // Open serial communications
  Serial.begin(9600);
  
  Serial.println(F("Initializing SD card analyzer..."));

  // Check if the SD card is present and can be initialized
  if (!SD.begin(chipSelect)) {
    Serial.println(F("SD Card initialization failed! It is either missing or broken."));
    while (true); // Halt
  }
  Serial.println(F("SD Card initialized successfully."));

  Serial.println(F("========================================"));
  Serial.println(F("LISTING STRICT EXACT FILENAMES ON SD CARD:"));
  Serial.println(F("========================================"));
  
  File root = SD.open("/");
  printDirectory(root, 0);

  Serial.println(F("========================================"));
  Serial.println(F("Please copy the EXACT filename shown above and tell me what it is!"));
}

void loop() {
  // Do nothing
}

// Helper function to print directory contents
void printDirectory(File dir, int numTabs) {
  while (true) {
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files in this folder
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial.print('\t');
    }
    
    // Print the EXACT name that the Arduino recognizes
    Serial.print(entry.name());
    
    if (entry.isDirectory()) {
      Serial.println(F("/"));
      printDirectory(entry, numTabs + 1);
    } else {
      // Print file sizes
      Serial.print(F("\t\t Size: "));
      Serial.println(entry.size(), DEC);
    }
    entry.close();
  }
}
