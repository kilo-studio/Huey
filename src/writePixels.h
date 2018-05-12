#ifndef _writePixels_H_
#define _writePixeks_H_

#include "Adafruit_NeoPixel.h"

#include "getForecast.h"
#include "utilityFunctions.h"

void showTime();
void breathe();
void refreshPixels();
void applyBrightness();
void windGust(int fadeDelay);
void rain();
void rainDrop(int dropIndex);
void wipeDown();
void wipeFade(float brightness);
void fadeBack();
void colorFromHour(String hourlyFile);
void colorFromDay(String dailyFile);

#define PIN    5
#define N_LEDS 168
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);

float breathSpeed = 2;//how fast should we breathe? increment per showDelay
int breathAmount = 120;//how much should we breathe?
int breathDirection = -1;
float breathProgress = 0;
boolean rainOverride = false;
float rainIntensityOverride = 0.5;
int const minDroplets = 2;
int const maxDroplets = 20;
Drop drops[maxDroplets];
float sunsetBrightness = 0.2;//brightness after the sun goes down
float defaultBrightness = 0.5;//brightness before the sun goes down
float refreshBrightness = 0.9;

long refreshInterval = 10L * 60L * 1000L; // delay between updates, in milliseconds
long lastRefresh = 0;//initial delay
long fbRefreshInterval = 6L * 60L * 1000L;
long fbLastRefresh = 30L * 1000L;//initial delay
long fbQuickRefreshInterval = 30 * 1000;
long fbLastQuickRefresh = 0;
int showTimeInterval = 20; //ms max frame rate
long checkShowTime;
int slowShowDelay = 50; // 60 x 1000 (1 minute)
long checkSlowShowTime;
int superSlowShowDelay = 150;
long checkSuperSlowShowTime;

int _UPDATING_ = 0;
int quickUpdates = 0;
boolean fadingBack = false;
boolean gusting = false;
int gustColumn = 0;
int timeToFade = 0;
boolean wipingDown = false;
int wipeDownColumn = 0;
boolean wipeFading = false;
int wipeFadingColumn = 0;
boolean settingBrightness = false;

void showTime(){
  long now = millis();

  if (currentPrecipIntensity > 0 || rainOverride) {
    rain();
  }

  if (settingBrightness && !wipingDown) {
    colorFromSD(currentlyFileName, hourlyFileName, dailyFileName);
    applySun();
    applyBrightness();
  }

  if (now >= checkShowTime){

    if (now >= checkSlowShowTime) {
      if (gusting) {
        windGust(500);
        //add the wind to rain droplets
        float windForce = currentWindSpeed/maxWind * 5;
        // Serial.println("currentWindSpeed: " + String(currentWindSpeed));
        // Serial.println("windForce: " + String(windForce));
        for (int i = 0; i < maxDroplets; i++) {
          drops[i].applyForce(windForce, 0.5);
        }
      }
      if (wipingDown) {wipeDown();}
      // if (wipeFading) {wipeFade(0.2);}
      checkSlowShowTime = now + slowShowDelay;
    }

    if (now >= checkSuperSlowShowTime){
      if (fadingBack) {fadeBack();}
      checkSuperSlowShowTime = now + superSlowShowDelay;
    }

    if (now >= checkWindTime) {
      Serial.println("wind!");

      gusting = true;
      checkWindTime = millis() + checkWindDelay;
    }

    if (!wipingDown) {
      breathe();
    }

    strip.show();
    checkShowTime = now + showTimeInterval;
  }
}

void refreshPixels() {
  Serial.println("Refreshing Pixels");
  wipingDown = true;
  settingBrightness = true;

  colorFromSD(currentlyFileName, hourlyFileName, dailyFileName);
  applySun();
  applyBrightness();

  Serial.println("Done Refreshing Pixels");
}

void applyBrightness(){
  int day = floor(currentIndex / 24);
  int hour = reIndex(currentIndex) % 24;
  float mult = defaultBrightness;

  if (hour <= sunrises[day] || hour >= sunsets[day]) {
    Serial.println("dimming");
    mult = sunsetBrightness;
  }

  for (int i = 0; i < 168; i++){
    int index = reIndex(i);
    // pixels[index].divide(divide);
    pixels[index].multiply(mult);

    if(!wipingDown){
      strip.setPixelColor(index, pixels[index].red, pixels[index].green, pixels[index].blue);
    }
  }

  settingBrightness = false;
}

void simpleRefresh(){
  for (int i = 0; i < 168; i++){
    //showTime();
    // strip.setPixelColor(
    //   i,
    //   pixels[i].red,
    //   pixels[i].green,
    //   pixels[i].blue
    // );

    strip.setPixelColor(i , 0, 150, 150);

    Serial.print("red: ");
    Serial.print(pixels[i].red);
    Serial.print(", green: ");
    Serial.print(pixels[i].green);
    Serial.print(", blue: ");
    Serial.print(pixels[i].blue);
    Serial.print(", appRed: ");
    Serial.print(pixels[i].appRed);
    Serial.print(", appBlue: ");
    Serial.println(pixels[i].appBlue);

    strip.show();
    delay(5);
  }
}

void wipeDown(){
  Serial.println("Wiping Down");

  for (int c = 0; c < 7; c++) {
    int index = c * 24 + wipeDownColumn;
    int prev = reIndex(index-1);
    index = reIndex(index);

    if (wipeDownColumn < 23){
      strip.setPixelColor(
        index,
        pixels[index].red,
        pixels[index].green,
        pixels[index].blue
      );
    }
    if (wipeDownColumn > 0) {
      strip.setPixelColor(
        prev,
        pixels[prev].red*refreshBrightness,
        pixels[prev].green*refreshBrightness,
        pixels[prev].blue*refreshBrightness
      );
    }
  }

  if (wipeDownColumn == 24) {
    wipeDownColumn = 0;
    wipingDown = false;
    return;
  } else {
    wipeDownColumn++;
  }
}

void wipeFade(float brightness){
  Serial.println("Wiping Down");

  for (int c = 0; c < 7; c++) {
    int index = c * 24 + wipeDownColumn;
    int prev = reIndex(index-1);
    index = reIndex(index);

    if (wipeFadingColumn < 23){
      strip.setPixelColor(
        index,
        pixels[index].red * 0.6,
        pixels[index].green * 0.6,
        pixels[index].blue * 0.6
      );
    }
    if (wipeFadingColumn > 0) {
      strip.setPixelColor(
        prev,
        pixels[prev].red*brightness,
        pixels[prev].green*brightness,
        pixels[prev].blue*brightness
      );
    }
  }

  if (wipeFadingColumn == 24) {
    wipeFadingColumn = 0;
    wipeFading = false;
    fadingBack = true;
    return;
  } else {
    wipeFadingColumn++;
  }
}

void windGust(int fadeDelay){
  //for (int c = 0; c < 8; c++) {
  for (int r = 0; r < 24; r++) {
    int index = gustColumn * 24 + r;
    int previousIndex = reIndex(index-24);
    index = reIndex(index);

    if (currentIndex != index) {
      if (gustColumn < 8) {
        strip.setPixelColor(
          index,
          pixels[index].appRed * refreshBrightness,
          pixels[index].appGreen * refreshBrightness,
          pixels[index].appBlue * refreshBrightness
        );
      }
    }
    if (currentIndex != previousIndex) {
      if (gustColumn > 0) {
        strip.setPixelColor(
          previousIndex,
          pixels[previousIndex].appRed,
          pixels[previousIndex].appGreen,
          pixels[previousIndex].appBlue
        );
      }
    }
  }

  int now = millis();

  if (gustColumn == 8 && timeToFade == 0) {
    timeToFade = now + fadeDelay;
    //Serial.println(String("timeToFade: ") + timeToFade);
  } else if (timeToFade != 0 && now > timeToFade) {
    // Serial.println(String("fading back ") + now + ", time is past " + timeToFade);
    gusting = false;
    gustColumn = 0;
    timeToFade = 0;
    fadingBack = true;
    return;
  } else if (gustColumn < 8){
    gustColumn++;
  }
}

void rain(){
  float intensity = currentPrecipIntensity;

  if (rainOverride) {
    intensity = rainIntensityOverride;
  }

  int fallingDrops = maxDroplets * intensity;
  fallingDrops = map(fallingDrops, 0, maxDroplets, minDroplets, maxDroplets);

  for (int i = 0; i < fallingDrops; i++) {
    rainDrop(i);
  }
}

void rainDrop(int dropIndex){
  Drop theDrop = drops[dropIndex];
  int pixelIndex = theDrop.x * 24 + theDrop.y;
  pixelIndex = reIndex(pixelIndex);
  Pixel thePixel = pixels[pixelIndex];

  if (theDrop.y < 24 && theDrop.x < 7 && pixelIndex != currentIndex) {
    strip.setPixelColor(
      pixelIndex,
      thePixel.appRed * 1.2,
      thePixel.appGreen * 1.2,
      thePixel.appBlue * 1.2
    );
  }

  //int trailIndex = reIndex(pixelIndex-1);
  // int trailIndex = theDrop.pX * 24 + theDrop.pY;
  // trailIndex = reIndex(trailIndex);
  // Pixel trailPixel = pixels[trailIndex];

  // if (theDrop.pY >= 0 && theDrop.pX >= 0 && theDrop.pX < 7) {
  //   strip.setPixelColor(
  //     trailIndex,
  //     trailPixel.red,
  //     trailPixel.green,
  //     trailPixel.blue
  //   );
  // }

  // Serial.print("x: ");
  // Serial.print(theDrop.x);
  // Serial.print(" y: ");
  // Serial.println(theDrop.y);

  fadingBack = true;
  drops[dropIndex].fall();
}

void fadeBack(){
  int numBack = 0;
  int lastNumBack = 0;
  int repeats = 0;

  for (int i = 0; i < 168; i++) {
    int index = reIndex(i);

    if (currentIndex != index) {
      int rin = 0;
      int bin = 0;
      int gin = 0;
      uint8_t r = Red(strip.getPixelColor(index));
      uint8_t g = Green(strip.getPixelColor(index));
      uint8_t b = Blue(strip.getPixelColor(index));
      int red = pixels[index].red;
      int green = pixels[index].green;
      int blue = pixels[index].blue;

      if (r > red) {rin = -1;}
      else if (r < red) {rin = 1;}

      if (g > green) {gin = -1;}
      else if (g < green) {gin = 1;}

      if (b > blue) {bin = -1;}
      else if (b < blue) {bin = 1;}

      if (rin == 0 && bin == 0 & gin == 0) {
        numBack++;
      } else {
        strip.setPixelColor(index, r + rin, g + gin, b + bin);
      }
    }
  }

  int fallingDrops = maxDroplets * currentPrecipIntensity;
  if (fallingDrops > 0) {
    fallingDrops = map(fallingDrops, 0, maxDroplets, 3, maxDroplets);
  }

  if (numBack < 167 - 3) {
    if (numBack == lastNumBack) {
      if (repeats > 10) {
        // Serial.println("close as it's going to get");
        fadingBack = false;
        return;
      }
      repeats++;
    } else {
      repeats = 0;
    }

    // Serial.print(numBack);
    // Serial.println(" back to normal");
    lastNumBack = numBack;
    numBack = 0;
  } else {
    // Serial.println("close as it's going to get");
    fadingBack = false;
    return;
  }

}

void breathe(){
  Pixel currentPixel = pixels[currentIndex];
  int sum = currentPixel.red + currentPixel.green + currentPixel.blue + breathProgress;
  int newRed = constrain((sum * currentPixel.percentRed()), 0, 255);
  int newGreen = constrain((sum * currentPixel.percentGreen()), 0, 255);
  int newBlue = constrain((sum * currentPixel.percentBlue()), 0, 255);

  strip.setPixelColor(currentIndex, newRed, newGreen, newBlue);
  breathProgress += breathSpeed * breathDirection;

  // Serial.print(sum);
  // Serial.print(" breathProgress: ");
  // Serial.println(breathProgress);

  if (breathProgress > breathAmount || breathProgress < 0) {
    breathDirection *= -1;
    // Serial.print("switching breathe direction, currentPixel: ");
    // Serial.println(currentIndex);
  }
}

void blackOut(){
  for(int i = 0; i < 168; i++){
    int index = reIndex(i);
    strip.setPixelColor(index,0,0,0);
    strip.show();
    delay(10);
  }
}

boolean colorFromSD(String currentlyFile, String hourlyFile, String dailyFile){
  JsonObject& currentJson = getJsonObject(currentlyFile);
  currentPrecipIntensity = currentJson["currentPrecipIntensity"];
  currentWindSpeed = currentJson["currentWindSpeed"];

  Serial.println(String("currentPrecipIntensity: ") + currentPrecipIntensity);
  Serial.println(String("currentWindSpeed: ") + currentWindSpeed);

  colorFromHour(hourlyFile);
  colorFromDay(dailyFile);

  return true;
}

void colorFromHour(String hourlyFile){
  // open the hourly file.
  const char* name = hourlyFile.c_str();
  File myFile = SD.open(name);
  // TextFinder finder(myFile, 20);


  if (myFile) {
    Serial.println(hourlyFile);

    // read from the file until there's nothing else in it:
    while (myFile.available())
    {
      // json = json + (char)myFile.read();
      //search through the results and create pixels for each hour in a week
      for (int i = 0; i < 168; i++)
      {
        yield();
        delay(2);
        //showTime();

        myFile.findUntil((char *)"time\":", (char *) "\n\r");
        time_t t = atoi(myFile.readStringUntil(',').c_str());
        int weekDay = weekday(t) - 1;
        int theHour = hour(t);

        myFile.findUntil((char *)"precipProbability\":", (char *) "\n\r");
        float precipProb = strtod(myFile.readStringUntil(',').c_str(), NULL);
        myFile.findUntil((char *)"temperature\":", (char *) "\n\r");
        int temp = atoi(myFile.readStringUntil(',').c_str());
        myFile.findUntil((char *)"apparentTemperature\":", (char *) "\n\r");
        int appTemp = atoi(myFile.readStringUntil(',').c_str());
        myFile.findUntil((char *)"cloudCover\":", (char *) "\n\r");
        float cloudCover = strtod(myFile.readStringUntil(',').c_str(), NULL);
        cloudCover = cloudCover * maxCloudCover;
        myFile.findUntil((char *)"humidity\":", (char *) "\n\r");
        float humidity = strtod(myFile.readStringUntil('}').c_str(), NULL);

        int red = map(temp, minTemp, maxTemp, 0, 255);
        int blue = 255 - red;
        int appRed = map(appTemp, minTemp, maxTemp, 0, 255);
        int appBlue = 255 - appRed;

        int green = float(255) * precipProb;
        if (green > 0) {
          green = map(green, 0, 255, 50, 255);
        }

        int appGreen = green + 40 * humidity;
        appGreen = constrain(appGreen, 0, 255);

        int theIndex = reIndex(weekDay * 24 + theHour);
        pixels[theIndex] = Pixel(red, green, blue, appRed, appBlue, appGreen);
        pixels[theIndex].multiply(1 - cloudCover);//more cloud coverage should be less bright

        if (i == 0) {currentIndex = theIndex;}//remember where now is in pixel[]

        // Serial.print("i: ");
        // Serial.print(i);
        // Serial.print(" weekDay: ");
        // Serial.print(weekDay);
        // Serial.print(", theHour: ");
        // Serial.println(theHour);
        // Serial.print("clouds: ");
        // Serial.println(cloudCover);

        if (i == 167) {
          // close the file:
          myFile.close();
          return;
        }
      }
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening hour.txt");
    return;
  }

}

void colorFromDay(String dailyFile){
  // open the hourly file.
  const char* name = dailyFile.c_str();
  File myFile = SD.open(name);
  // TextFinder finder(myFile, 20);


  if (myFile) {
    Serial.println(dailyFile);

    // read from the file until there's nothing else in it:
    while (myFile.available())
    {
      for (int i = 0; i < 7; i++) {
        myFile.findUntil((char *)"sunrise\":", (char *) "\n\r");
        sunrises[i] = atoi(myFile.readStringUntil(',').c_str());
        myFile.findUntil((char *)"sunset\":", (char *) "\n\r");
        sunsets[i] = atoi(myFile.readStringUntil(',').c_str());

        // myFile.findUntil((char *)"cloudCover\":", (char *) "\n\r");
        // cloudCover[i] = strtod(myFile.readStringUntil('}').c_str(), NULL);

        // Serial.print("Sunrise: ");
        // Serial.print(sunrises[i]);
        // Serial.print(" Sunset: ");
        // Serial.println(sunsets[i]);

        if (i == 6) {
          // close the file:
          myFile.close();
          return;
        }
      }
    }
  }
}

#endif
