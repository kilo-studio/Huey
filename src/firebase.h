#ifndef _firebase_H_
#define _firebase_H_

#include <TextFinder.h>
#include <WiFi101.h>
#include <FlashStorage.h>
#include <FlashAsEEPROM.h>
#include <SD.h>

#include "arduino_secrets.h"

void fbPrintResponse();
boolean fbGetHuey();
bool fbParseName();
bool fbParseHuey();
float fbFloatHunt(char* key);
bool fbBoolHunt(char* key);
int fbIntHunt(char* key);
time_t fbTimeHunt(char* key, int timezone);
long fbLongHunt(char* key);
bool fbGive(String command, String content);
bool fbGet(String request);

String fbSerial;

WiFiSSLClient firebaseClient;
TextFinder fbFinder(firebaseClient, 20);//finder and wait time in seconds
const char fbDatabaseURL[] = "kilo-olli.firebaseio.com";

bool fbRegister(){
  Serial.println("---------");
  Serial.println("Registering new huey with firebase...");

  IPAddress ip = WiFi.localIP();
  String ipStr = String(ip[0])+"."+String(ip[1])+"."+String(ip[2])+"."+String(ip[3]);

  String content = "{";
  content += String("\"IP\" : ") + "\"" + ipStr + "\"";
  // content += String(",\"_UPDATING_\" : ") + "0";
  content += String(",\"latitude\" : ") + "\"" + String(lat) + "\"";
  content += String(",\"longitude\" : ") + "\"" + String(lon) + "\"";
  content += String(",\"defaultBrightness\" : ") + "\"" + String(defaultBrightness) + "\"";
  content += String(",\"maxCloudCover\" : ") + "\"" + String(maxCloudCover) + "\"";
  content += String(",\"maxTemp\" : ") + "\"" + String(maxTemp) + "\"";
  content += String(",\"minTemp\" : ") + "\"" + String(minTemp) + "\"";
  content += String(",\"refreshBrightness\" : ") + "\"" + String(refreshBrightness) + "\"";
  content += String(",\"refreshInterval\" : ") + "\"" + String(refreshInterval) + "\"";
  content += String(",\"sunIntensity\" : ") + "\"" + String(sunIntensity) + "\"";
  content += String(",\"sunsetBrightness\" : ") + "\"" + String(sunsetBrightness) + "\"";
  content += "}";

  fbGive("POST", content);
  fbParseName();

  return true;
}

bool fbParseName(){
  Serial.println("---------");
  Serial.println("Parsing and saving the name to SD card...");
  String tempName = "";

  if (!fbFinder.findUntil((char *)"name\":\"", (char *)"}")) {
    Serial.println("couldn't find the name");

    return false;
  }

  Serial.println("Found the name!");

  while (firebaseClient.available()) {
    char c = firebaseClient.read();
    if (c == '\"') {
      break;
    }
    tempName += c;
    // Serial.print(c);
  }

  Serial.print("Tempname: ");
  Serial.println(tempName);
  fbSerial = tempName;

  //write the fbSerial to the SD Card
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File myFile = SD.open("fbSerial.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to fbSerial.txt...");
    myFile.println(fbSerial);
  // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening fbSerial.txt");
  }

  Serial.print("Saved name: ");
  Serial.println(fbSerial);
  return true;
}

// bool fbUpdate(){
//   Serial.println("---------");
//
//   String allOllis = String("/ollis/") + fbSerial + "/_UPDATING_" + ".json";
//   allOllis += String("?auth=") + FB_SECRET;
//
//   if (!fbGet(allOllis)){
//     return false;
//   }
//
//   fbFinder.findUntil((char *) "\n\r", (char *) "\n\r");
//   _UPDATING_ = fbFinder.getValue();
//   firebaseClient.flush();
//
//   Serial.println("_UPDATING_: " + String(_UPDATING_));
//   return true;
// }

boolean fbGetHuey() {
  Serial.println("---------");
  Serial.println("Getting huey...");

  String me = String("/ollis/") + fbSerial + ".json";
  me += String("?auth=") + FB_SECRET;

  if(!fbGet(me)){
    return false;
  }
  fbParseHuey();
  return true;
}

bool fbParseHuey(){
  String tempName = "";

  if (!fbFinder.findUntil((char *)"{", (char *)"}")) {
    Serial.println("Could not retrieve settings from firebase, using defaults.");
    return false;
  }

  defaultBrightness = fbFloatHunt((char *)"defaultBrightness");
  maxCloudCover = fbFloatHunt((char *)"maxCloudCover");
  maxTemp = fbIntHunt((char *)"maxTemp");
  minTemp = fbIntHunt((char *)"minTemp");
  refreshBrightness = fbFloatHunt((char *)"refreshBrightness");
  refreshInterval = fbLongHunt((char*)"refreshInterval");
  sunIntensity = fbFloatHunt((char *)"sunIntensity");
  sunsetBrightness = fbFloatHunt((char *)"sunsetBrightness");

  Serial.println("Successfully retrieved settings from firebase.");
  Serial.println(String("defaultBrightness: ") + defaultBrightness);
  Serial.println(String("maxCloudCover: ") + maxCloudCover);
  Serial.println(String("maxTemp: ") + maxTemp);
  Serial.println(String("minTemp: ") + minTemp);
  Serial.println(String("refreshBrightness: ") + refreshBrightness);
  Serial.println(String("refreshInterval: ") + refreshInterval);
  Serial.println(String("sunIntensity: ") + sunIntensity);
  Serial.println(String("sunsetBrightness: ") + sunsetBrightness);

  return true;
}

bool fbGet(String request){
  //https://[PROJECT_ID].firebaseio.com/users/jack/name.json
  Serial.println("---------");
  Serial.println("Looking for my name on Firebase...");

  if (fbSerial == NULL) {
    Serial.println("I don't know what my name is!");
    return false;
  }

  Serial.print("My Name: ");
  Serial.println(fbSerial);

  Serial.print(F("connecting to "));
  Serial.println(fbDatabaseURL);

  if (!firebaseClient.connect(fbDatabaseURL, 443)) {
    Serial.println(F("connection failed"));
    Serial.println(firebaseClient.connect(fbDatabaseURL, 165));
    firebaseClient.stop();
    return false;
  }

  Serial.print(F("requesting URL: "));
  Serial.println(request);

  // send the HTTPS GET request:
  firebaseClient.println(String("GET ") + request + " HTTP/1.1");
  firebaseClient.println(String("Host: ") + fbDatabaseURL);
  firebaseClient.println("Connection: close");
  firebaseClient.println();

  Serial.println(F("request sent"));

  return true;
}

bool fbGive(String command, String content){
  Serial.println("---------");
  Serial.println(command);
  Serial.println(content);

  if (!firebaseClient.connect(fbDatabaseURL, 443)) {
    Serial.println(F("connection failed"));
    Serial.println(firebaseClient.connect(fbDatabaseURL, 165));
    firebaseClient.stop();
    firebaseClient.flush();
    return false;
  }

  String test = String("/ollis/") + fbSerial + ".json" + "?auth=" + FB_SECRET;
  Serial.print(F("connecting to "));
  Serial.println(fbDatabaseURL);

  firebaseClient.println(command + " " + test + " HTTP/1.1");
  firebaseClient.println(String("Host: ") + fbDatabaseURL);
  firebaseClient.println(String("Content-Type: application/json; charset=UTF-8"));
  firebaseClient.println(String("Content-Length: ") + content.length());
  firebaseClient.println("Connection: close");
  firebaseClient.println();
  firebaseClient.println(content);

  Serial.println(F("request sent"));
  Serial.println(String(fbDatabaseURL) + String(test));

  return true;
}

bool getEepName(){
  // open the file for reading:
  Serial.println("---------");
  Serial.println("Looking for my name...");

  File myFile = SD.open("fbSerial.txt");
  if (myFile) {
    Serial.println("trimmed fbSerial.txt:");
    String readSerial = "";
    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      readSerial = readSerial + (char)myFile.read();
    }
    // close the file:
    myFile.close();
    readSerial.trim();
    Serial.println(readSerial);
    fbSerial = readSerial;
    return true;
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening fbSerial.txt");
    return false;
  }
}

//finder functions
time_t fbTimeHunt(char* key, int timezone) {
  fbFinder.findUntil(key, (char *) "\n\r");
  time_t t = fbFinder.getValue();
  return t + timeZone;//get the time and adjust it for the timeZone
}

bool fbBoolHunt(char* key){
  fbFinder.findUntil(key, (char *) "\n\r");
  return fbFinder.getValue();
}

int fbIntHunt(char* key){
  fbFinder.findUntil(key, (char *) "\n\r");
  return fbFinder.getValue();
}

float fbFloatHunt(char* key){
  fbFinder.findUntil(key, (char *) "\n\r");
  return fbFinder.getFloat();
}

long fbLongHunt(char* key){
  fbFinder.findUntil(key, (char *) "\n\r");
  return fbFinder.getValue();
}

#endif
