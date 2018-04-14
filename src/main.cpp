/*
Repeating WiFi Web Client

This sketch connects to a a web server and makes a request
using an Arduino WiFi shield.

Circuit:
* WiFi shield attached to pins SPI pins and pin 7

created 23 April 2012
modified 31 May 2012
by Tom Igoe
modified 13 Jan 2014
by Federico Vanzati

http://arduino.cc/en/Tutorial/WiFiWebClientRepeating
This code is in the public domain.
*/

#ifndef _main_CPP_
#define _main_CPP_

#include <SPI.h>
#include <WiFi101.h>
#include <WiFi101OTA.h>
#include <WiFiMDNSResponder.h>
#include <SD.h>
#include <SDU.h>
#include <TextFinder.h>
#include <TimeLib.h>
#include <Adafruit_NeoPixel.h>
#include <FlashStorage.h>

#include "Pixel.h"
#include "Drop.h"
#include "arduino_secrets.h"
#include "getLocation.h"
#include "getTimeZone.h"
#include "getForecast.h"
#include "writePixels.h"
#include "firebase.h"
#include "utilityFunctions.h"
#include "webService.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
char ota_ssid[] = OTA_SSID;
char ota_pass[] = OTA_PASS;
int keyIndex = 0;            // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

const int ledPin = 13; // LED pin for connectivity status indicator

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  //SD Card Setup
  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
    Serial.println("SD initialization failed!");
  } else {
    Serial.println("SD initialization done.");
  }

  strip.begin();
  strip.setBrightness(255);
  blackOut();

  //Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8,7,4,2);

  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue:
    while (true);
  }

  // Start in provisioning mode:
  //  1) This will try to connect to a previously associated access point.
  //  2) If this fails, an access point named "wifi101-XXXX" will be created, where XXXX
  //     is the last 4 digits of the boards MAC address. Once you are connected to the access point,
  //     you can configure an SSID and password by visiting http://wifi101/
  WiFi.beginProvision();

  pinMode(ledPin, OUTPUT);

  // attempt to connect to WiFi network:
  while (WiFi.status() != WL_CONNECTED) {
    // Serial.print("Attempting to connect to SSID: ");
    // Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    //status = WiFi.begin(ssid, pass);

    Serial.write('.');
    delay(500);
  }

  // connected, make the LED stay on
  Serial.println("OK!");

  setUpWebService();

  // you're connected now, so print out the status:
  printWiFiStatus();

  getLocation(ip);
  getTimeZone(lat, lon);

  // start the WiFi OTA library with internal (flash) based storage
  // WiFiOTA.begin(ota_ssid, ota_pass, SDStorage);

  lastRefresh = 0;

  // if (!getEepName()) {
    //fbRegister();
  // }
  //fbGetHuey();

  //firebaseClient.flush();
}

void loop() {
  // check for WiFi OTA updates
  // WiFiOTA.poll();

  // Call the update() function on the MDNS responder every loop iteration to
  // make sure it can detect and respond to name requests.
  mdnsResponder.poll();
  checkForClients();

  long now = millis();
  // if (now % 1000 == 0) {
  //   Serial.print("now: ");
  //   Serial.println(now);
  //   Serial.print("lastRefresh: ");
  //   Serial.println(lastRefresh);
  // }
  if (now > lastRefresh) {
    Serial.println("---------");
    Serial.println("Refreshing firebase, forecast, and pixels...");

    // if (!fbGetHuey()){
    //   Serial.println("Could not refresh from firebase");
    // }

    // firebaseClient.flush();

    if (connectToDarkSky(lat, lon)){
        applySun();
        refreshPixels();
        // simpleRefresh();

    }

    // simpleRefresh();

    // client.stop();
    // client.flush();
    lastRefresh = now + refreshInterval;
  }

  // if (now > fbLastRefresh) {
  //   Serial.println("---------");
  //   Serial.println("Checking Firebase");
  //   fbGetHuey();
  //
  //   // if (_UPDATING_ == 1){
  //   //   fbGetHuey();
  //   //   fbStopUpdate();
  //   //   lastRefresh = 0;
  //   // }
  //
  //   // firebaseClient.stop();
  //   // firebaseClient.flush();
  //
  //   Serial.println("---------");
  //   fbLastRefresh = now + fbRefreshInterval;
  // }

  // while (firebaseClient.available()) {
  //   char c = firebaseClient.read();
  //   Serial.write(c);
  // }

  // if (_UPDATING_ && now > fbLastQuickRefresh) {
  //   Serial.println("---------");
  //   Serial.println("Quickly Checking Firebase");
  //   // _UPDATING_ = fbUpdate();
  //   fbGetHuey();
  //   if (quickUpdates > 2){
  //     _UPDATING_ = false;
  //     fbStopUpdate();
  //   }
  //   Serial.println("---------");
  //   quickUpdates++;
  //   fbLastQuickRefresh = now + fbQuickRefreshInterval;
  // }

  showTime();

  // long refreshInterval = 10L * 60L * 1000L; // delay between updates, in milliseconds
  int countDownSeconds = (lastRefresh - now) / 1000L ; //60 * 1000 for minutes left, 1000 for seconds
  int countDownMinutes = floor(countDownSeconds / (60L)); //60 * 1000 for minutes left, 1000 for seconds
  countDownSeconds = countDownSeconds - countDownMinutes * 60;
  if (now % 1000 == 0) {
    Serial.print("next refresh in ");
    Serial.print(countDownMinutes);
    Serial.print(":");
    Serial.println(countDownSeconds);
  }

  delay(3);
}

#endif
