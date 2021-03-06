#ifndef _getForecast_H_
#define _getForecast_H_

#include <TimeLib.h>
#include <TextFinder.h>
#include <ArduinoJson.h>
#include <WiFi101.h>
#include <SD.h>

#include "arduino_secrets.h"
#include "utilityFunctions.h"
#include "Pixel.h"
#include "Drop.h"
#include "getLocation.h"
#include "getTimeZone.h"

boolean connectToDarkSky(String latitude, String longitude);
void getForecast();
void storeCurrentlyForecast();
void storeHourlyForecast();
void storeDailyForecast();
void applySun();
boolean writeJsonToSD(String, JsonObject&);
char* currentlyFileName = "current.txt";
const int currentBufferSize = 2*20;
char* hourlyFileName = "hour.txt";
const int hourBufferSize = 215;
char* dailyFileName = "day.txt";
const int dayBufferSize = 3 * 50;
char* forecastFileName = "cast.txt";

int maxTemp = 100;
int minTemp = 10;
float maxCloudCover = 0.7;//0.7
float sunIntensity = 0.2;//0.2
float maxWind = 40;//40

//current data
Pixel pixels[168];
int sunrises[] = {0,0,0,0,0,0,0};
int sunsets[] = {0,0,0,0,0,0,0};
float cloudCover[] = {0,0,0,0,0,0,0};
float currentPrecipIntensity = -1;
float currentWindSpeed = -1;
int currentIndex = 0;
int checkWindDelay = 15 * 1000; // 60 x 1000 (1 minute)
long checkWindTime = 30000;

boolean connectToDarkSky(char* latitude, char* longitude) {
  // Initialize the WiFi client library
  WiFiSSLClient client;
  TextFinder finder(client, 20);//finder and wait time in seconds

  Serial.println("---------");
  Serial.println("Connecting to darksky...");
  //"https://api.darksky.net/forecast/e412ad3481ba8e4fdc137985443d60ca/"+longlat+"/?extend=hourly"
  const char host[] = "api.darksky.net";
  String apiKey = DARK_SKY_KEY;
  const char extendHourly[] = "?extend=hourly";
  const char exclusions[] = "&exclude[latitude,longitude,offset,minutely,alerts,flags]/";
  String url = "/forecast/" + apiKey + "/" + latitude + "," + longitude + extendHourly + exclusions;
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

  //************************************************************//
  //************************************************************//
  //************** STORE CURRENTLY FORECAST ********************//
  //************************************************************//
  //************************************************************//

  StaticJsonBuffer<currentBufferSize> jsonBuffer;
  JsonObject& currentRoot = jsonBuffer.createObject();

  finder.findUntil((char *)"currently", (char *) "\n\r");
  finder.findUntil((char *)"precipIntensity", (char *) "\n\r");
  currentPrecipIntensity = finder.getFloat();
  finder.findUntil((char *)"windSpeed", (char *) "\n\r");
  currentWindSpeed = finder.getFloat();

  currentRoot["currentPrecipIntensity"] = currentPrecipIntensity;
  currentRoot["currentWindSpeed"] = currentWindSpeed;
  SD.remove(currentlyFileName);
  writeJsonToSD(currentlyFileName, currentRoot);

  checkWindDelay = map(currentWindSpeed, 0, maxWind, 10 * 1000, 1000);//convertWind Speed to refresh rate in ms

  Serial.println("currentWindSpeed: " + String(currentWindSpeed));
  Serial.println("checkWindDelay: " + String(checkWindDelay));

  checkWindTime = millis() + checkWindDelay;
  Serial.print("precipIntensity: ");
  Serial.println(currentPrecipIntensity);

  //*********************************************************//
  //*********************************************************//
  //************** STORE HOURLY FORECAST ********************//
  //*********************************************************//
  //*********************************************************//

  SD.remove(hourlyFileName);
  // open the file.
  const char* hname = hourlyFileName;
  File hourlyFile = SD.open(hname, FILE_WRITE);

  // if the file opened okay, write to it:
  if (hourlyFile) {
    Serial.println(String("Writing to ") + hourlyFileName);

    hourlyFile.print("{\"hourly\":[");

    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println(String("Error opening ") + hourlyFileName);
  }

  //search through the results and create pixels for each hour in a week
  for (int i = 0; i < 168; i++) {
    yield();

    finder.findUntil((char *)"time", (char *) "\n\r");
    time_t t = finder.getValue() + timeZone;
    int weekDay = weekday(t) - 1;
    int theHour = hour(t);

    finder.findUntil((char *)"precipProbability", (char *) "\n\r");
    float precipProb = finder.getFloat();
    finder.findUntil((char *)"temperature", (char *) "\n\r");
    int temp = finder.getValue();
    finder.findUntil((char *)"apparentTemperature", (char *) "\n\r");
    int appTemp = finder.getValue();
    finder.findUntil((char *)"humidity", (char *) "\n\r");
    float humidity = finder.getFloat();
    finder.findUntil((char *)"cloudCover", (char *) "\n\r");
    float cloudCover = finder.getFloat();

    delay(2);

    StaticJsonBuffer<hourBufferSize> jsonBuffer;
    JsonObject& hourRoot = jsonBuffer.createObject();
    JsonObject& data = hourRoot.createNestedObject(String(i));
    data["time"] = t;
    data["precipProbability"] = precipProb;
    data["temperature"] = temp;
    data["apparentTemperature"] = appTemp;
    data["cloudCover"] = cloudCover;
    data["humidity"] = humidity;

    // if the file opened okay, write to it:
    if (hourlyFile) {
      // if the file opened okay, write to it:
      // Serialize JSON to file
      if (hourRoot.printTo(hourlyFile) == 0) {
        Serial.println(String("Failed to write to ") + hourlyFileName);
      }
      if (i<167){
          hourlyFile.print(",");
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
  }
  // if the file opened okay, write to it:
  if (hourlyFile) {
    // if the file opened okay, write to it:
    hourlyFile.print("]}");
    hourlyFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println(String("Error opening ") + hourlyFileName);
  }

  //*******************************************************//
  //*******************************************************//
  //**************STORE DAILY FORECAST*********************//
  //*******************************************************//
  //*******************************************************//
  SD.remove(dailyFileName);
  // open the file.
  const char* dname = dailyFileName;
  File dailyFile = SD.open(dname, FILE_WRITE);

  // if the file opened okay, write to it:
  if (dailyFile) {
    Serial.println(String("Writing to ") + dailyFileName);

    dailyFile.print("{\"daily\":[");

    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println(String("Error opening ") + dailyFileName);
  }

  finder.findUntil((char *)"daily\":", (char *)"]");

  for (int i = 0; i < 7; i++) {
    finder.findUntil((char *)"sunriseTime", (char *) "\n\r");
    time_t sunrise = finder.getValue() + timeZone;
    finder.findUntil((char *)"sunsetTime", (char *) "\n\r");
    time_t sunset = finder.getValue() + timeZone;

    int theSunrise = hour(sunrise);
    int theSunset = hour(sunset);
    sunrises[i] = theSunrise;
    sunsets[i] = theSunset;

    finder.findUntil((char *)"cloudCover", (char *) "\n\r");
    cloudCover[i] = finder.getFloat();

    StaticJsonBuffer<dayBufferSize> jsonBuffer;
    JsonObject& dayRoot = jsonBuffer.createObject();
    JsonObject& data = dayRoot.createNestedObject(String(i));
    data["sunrise"] = theSunrise;
    data["sunset"] = theSunset;
    data["cloudCover"] = cloudCover;

    // if the file opened okay, write to it:
    if (dailyFile) {
      // if the file opened okay, write to it:
      // Serialize JSON to file
      if (dayRoot.printTo(dailyFile) == 0) {
        Serial.println(String("Failed to write to ") + dailyFileName);
      }
      if (i<6){
          dailyFile.print(",");
      }
      Serial.println("done.");
    } else {
      // if the file didn't open, print an error:
      Serial.println(String("Error opening ") + dailyFileName);
    }

    Serial.print("Sunrise: ");
    Serial.print(sunrises[i]);
    Serial.print(" Sunset: ");
    Serial.println(sunsets[i]);
  }

  // if the file opened okay, write to it:
  if (dailyFile) {
    // if the file opened okay, write to it:
    // Serialize JSON to file
    dailyFile.print("]}");
    dailyFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println(String("Error opening ") + hourlyFileName);
  }

  client.flush();
  client.stop();

  return true;
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

#endif
