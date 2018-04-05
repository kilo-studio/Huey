#ifndef _getForecast_H_
#define _getForecast_H_

#include <TimeLib.h>
#include <TextFinder.h>
#include <ArduinoJson.h>
#include <WiFi101.h>
#include <SD.h>

#include "utilityFunctions.h"
#include "Pixel.h"
#include "Drop.h"
#include "getLocation.h"
#include "getTimeZone.h"

boolean connectToDarkSky(char* latitude, char* longitude);
void getForecast();
void storeCurrentlyForecast();
void storeHourlyForecast();
void storeDailyForecast();
void applySun();
boolean writeJsonToSD(String, JsonObject&);
char* currentlyFileName = "current.txt";
char* hourlyFileName = "hour.txt";

int maxTemp = 100;
int minTemp = 10;
float maxCloudCover = 0.7;
float sunIntensity = 0.2;
float maxWind = 40;

//current data
Pixel pixels[168];
int sunrises[] = {0,0,0,0,0,0,0};
int sunsets[] = {0,0,0,0,0,0,0};
float windSpeeds[] = {0,0,0,0,0,0,0};
float currentPrecipIntensity = -1;
float currentWindSpeed = -1;
int currentIndex = 0;
int checkWindDelay = 15 * 1000; // 60 x 1000 (1 minute)
long checkWindTime = 30000;

// Initialize the WiFi client library
WiFiSSLClient client;
TextFinder finder(client, 20);//finder and wait time in seconds

boolean connectToDarkSky(char* latitude, char* longitude) {
  Serial.println("---------");
  Serial.println("Connecting to darksky...");
  //"https://api.darksky.net/forecast/e412ad3481ba8e4fdc137985443d60ca/"+longlat+"/?extend=hourly"
  const char host[] = "api.darksky.net";
  const char apiKey[] = "/forecast/e412ad3481ba8e4fdc137985443d60ca/";
  const char extendHourly[] = "?extend=hourly";
  const char exclusions[] = "&exclude[latitude,longitude,offset,minutely,alerts,flags]/";
  String url = String(apiKey) + latitude + "," + longitude + extendHourly + exclusions;
  //const static char url[] = "/forecast/e412ad3481ba8e4fdc137985443d60ca/37.593626,-77.521149/?extend=hourly&exclude=[latitude,longitude,minutely,alerts,flags]";

  Serial.print(F("connecting to "));
  Serial.println(host);

  if (!client.connect(host, 443)) {
    Serial.println(F("connection failed"));
    Serial.println(client.connect(host, 165));
    client.stop();
    return false;
  }

  Serial.print(F("requesting URL: "));
  Serial.println(url);

  // send the HTTPS GET request:
  client.println(String("GET ") + url + " HTTP/1.1");
  client.println(String("Host: ") + host);
  client.println("Connection: close");
  client.println();

  Serial.println(F("request sent"));

  return true;
}

void getForecast() {
  if (client.connected()) {
    Serial.println("---------");
    Serial.println(F("Getting the Forecast..."));

    storeCurrentlyForecast();
    storeHourlyForecast();
    storeDailyForecast();
    applySun();
  }
}

void storeCurrentlyForecast() {
  const int BUFFER_SIZE = 2*20;
  StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();

  finder.findUntil((char *)"currently\":", (char *)"]");
  currentPrecipIntensity = floatHunt((char *)"precipIntensity");
  currentWindSpeed = floatHunt((char *)"windSpeed");

  root["currentPrecipIntensity"] = currentPrecipIntensity;
  root["currentWindSpeed"] = currentWindSpeed;
  SD.remove(currentlyFileName);
  writeJsonToSD(currentlyFileName, root);

  checkWindDelay = map(currentWindSpeed, 0, maxWind, 10 * 1000, 1000);//convertWind Speed to refresh rate in ms

  Serial.println("currentWindSpeed: " + String(currentWindSpeed));
  Serial.println("checkWindDelay: " + String(checkWindDelay));

  checkWindTime = millis() + checkWindDelay;
  Serial.print("precipIntensity: ");
  Serial.print(currentPrecipIntensity);
  Serial.print(" windspeed: ");
  Serial.println(currentWindSpeed);
}

void storeHourlyForecast() {
  SD.remove(hourlyFileName);
  // open the file.
  const char* name = hourlyFileName;
  File myFile = SD.open(name, FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println(String("Writing to ") + hourlyFileName);

    myFile.print("{\"hourly\":[");

    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println(String("Error opening ") + hourlyFileName);
  }

  //search through the results and create pixels for each hour in a week
  for (int i = 0; i < 168; i++) {
    delay(5);
    //showTime();
    //finder.findUntil("hourly\":", "]");
    time_t t = timeHunt((char *)"time", timeZone);
    int weekDay = weekday(t) - 1;
    int theHour = hour(t);
    float precipProb = floatHunt((char *)"precipProbability");;
    int temp = intHunt((char *)"temperature");
    int appTemp = intHunt((char *)"apparentTemperature");
    float humidty = floatHunt((char *)"humidity");

    int red = map(temp, minTemp, maxTemp, 0, 255);
    int blue = 255 - red;
    int appRed = map(appTemp, minTemp, maxTemp, 0, 255);
    int appBlue = 255 - appRed;

    int green = float(255) * precipProb;
    if (green > 0) {
      green = map(green, 0, 255, 50, 255);
    }

    int appGreen = green + 40 * humidty;
    appGreen = constrain(appGreen, 0, 255);

    int theIndex = reIndex(weekDay * 24 + theHour);
    pixels[theIndex] = Pixel(red, green, blue, appRed, appBlue, appGreen);

    float cloudCover = floatHunt((char *)"cloudCover");
    cloudCover = cloudCover * maxCloudCover;
    pixels[theIndex].multiply(1 - cloudCover);//more cloud coverage should be less bright

    if (i == 0) {currentIndex = theIndex;}//remember where now is in pixel[]

    const int BUFFER_SIZE = 100;
    StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    JsonObject& data = root.createNestedObject(String(i));
    data["time"] = t;
    data["precipProbability"] = precipProb;
    data["temperature"] = temp;
    data["apparentTemperature"] = appTemp;
    data["humidity"] = humidty;

    // if the file opened okay, write to it:
    if (myFile) {
      // if the file opened okay, write to it:
      // Serialize JSON to file
      if (root.printTo(myFile) == 0) {
        Serial.println(String("Failed to write to ") + hourlyFileName);
      }
      if (i<167){
          myFile.print(",");
      }
      Serial.println("done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println(String("Error opening ") + hourlyFileName);
    }

    Serial.print("weekDay: ");
    Serial.print(weekDay);
    Serial.print(", theHour: ");
    Serial.println(theHour);

    // Serial.print("red: ");
    // Serial.print(pixels[theIndex].red);
    // Serial.print(", green: ");
    // Serial.print(pixels[theIndex].green);
    // Serial.print(", blue: ");
    // Serial.print(pixels[theIndex].blue);
    // Serial.print(", appRed: ");
    // Serial.print(pixels[theIndex].appRed);
    // Serial.print(", appBlue: ");
    // Serial.println(pixels[theIndex].appBlue);
  }
  // if the file opened okay, write to it:
  if (myFile) {
    // if the file opened okay, write to it:
    // Serialize JSON to file
    myFile.print("]}");
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println(String("Error opening ") + hourlyFileName);
  }
  // SD.remove(hourlyFileName);
  // writeJsonToSD(hourlyFileName, root);
}

void storeDailyForecast() {
  finder.findUntil((char *)"daily\":", (char *)"]");

  for (int i = 0; i < 7; i++) {
    //time_t t = timeHunt((char *)"time", timeZone);
    // int weekDay = weekday(t) - 1;
    // int theHour = hour(t);

    time_t sunrise = timeHunt((char *)"sunriseTime", timeZone);
    time_t sunset = timeHunt((char *)"sunsetTime", timeZone);
    int theSunrise = hour(sunrise);
    int theSunset = hour(sunset);
    sunrises[i] = theSunrise;
    sunsets[i] = theSunset;
    windSpeeds[i] = floatHunt((char *)"cloudCover");;

    Serial.print("Sunrise: ");
    Serial.print(sunrises[i]);
    Serial.print(" Sunset: ");
    Serial.println(sunsets[i]);
  }
}

void applySun() {
  for (int i = 0; i < 168; i++) {
    int day = floor(i / 24);
    int hour = i % 24;

    if (hour < sunrises[day] || hour > sunsets[day]) {
      pixels[reIndex(i)].brighten(sunIntensity * -0.5);
    } else if (hour > sunrises[day] && hour < sunsets[day]) {
      pixels[reIndex(i)].brighten(sunIntensity * 0.5);
    }
  }
}

//finder functions
time_t timeHunt(char* key, int timezone) {
  finder.findUntil(key, (char *) "\n\r");
  time_t t = finder.getValue();
  return t + timeZone;//get the time and adjust it for the timeZone
}

int intHunt(char* key){
  finder.findUntil(key, (char *) "\n\r");
  return finder.getValue();
}

float floatHunt(char* key){
  finder.findUntil(key, (char *) "\n\r");
  return finder.getFloat();
}

#endif
