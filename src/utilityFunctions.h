#ifndef _utilityFunctions_H_
#define _utilityFunctions_H_

#include "getForecast.h"
#include "getTimeZone.h"
#include <ArduinoJson.h>
#include <TextFinder.h>
#include <SD.h>

void printWiFiStatus();
IPAddress ip;
int reIndex(int Index);
time_t timeHunt(char* key, int timezone);
int intHunt(char* key);
float floatHunt(char* key);
uint8_t Red(uint32_t color);
uint8_t Green(uint32_t color);
uint8_t Blue(uint32_t color);
boolean writeJsonToSD(String filename, JsonObject& json);
String getJsonSD(String filename);
JsonObject& getJsonObject(String filename);
boolean colorFromSD(String currentlyFile, String hourlyFile, String dailyFile);

boolean writeJsonToSD(String filename, JsonObject& json){
  // open the file.
  const char* name = filename.c_str();
  File myFile = SD.open(name, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println(String("Writing to ") + filename);
    // myFile.println("testing 1, 2, 3.");

    // if the file opened okay, write to it:
    // Serialize JSON to file
    if (json.printTo(myFile) == 0) {
      Serial.println(String("Failed to write to ") + filename);
    }
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println(String("Error opening ") + filename);
  }
}

JsonObject& getJsonObject(String filename){
  // open the file for reading:
  String json = getJsonSD(filename);

  DynamicJsonBuffer jsonBuffer;

  JsonObject& root = jsonBuffer.parseObject(json);

  // Step 5
  if (!root.success()) {
    Serial.println("parseObject() failed");
    Serial.println(json);
  }

  return root;
}

String getJsonSD(String filename){
  // open the file.
  const char* name = filename.c_str();
  File myFile = SD.open(name);
  TextFinder finder(myFile, 20);

  if (myFile) {
    Serial.println(filename);

    String json;

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      json = json + (char)myFile.read();
    }
    Serial.println(json);
    // close the file:
    myFile.close();
    return json;
  } else {
  	// if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

int reIndex(int Index) {
  int theDay = Index / 24;
  int theHour = Index % 24;

  if (theDay % 2 == 1) {
    //reverse the order
    theHour = map(theHour, 0, 23, 23, 0);
    return (theDay*24 + theHour);
  } else {
    return Index;
  }
}

// Returns the Red component of a 32-bit color
uint8_t Red(uint32_t color){return (color >> 16) & 0xFF;}

// Returns the Green component of a 32-bit color
uint8_t Green(uint32_t color){return (color >> 8) & 0xFF;}

// Returns the Blue component of a 32-bit color
uint8_t Blue(uint32_t color){return color & 0xFF;}

#endif
