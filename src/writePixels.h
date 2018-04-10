#ifndef _writePixels_H_
#define _writePixeks_H_

#include "Adafruit_NeoPixel.h"

#include "getForecast.h"
#include "utilityFunctions.h"

#define PIN    5
#define N_LEDS 168
Adafruit_NeoPixel strip = Adafruit_NeoPixel(N_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void showTime();
void breathe();
void refreshPixels();
void windGust(int fadeDelay);
void rain();
void rainDrop(int dropIndex);
void wipeDown();
void wipeFade(float brightness);
void fadeBack();

float breathSpeed = 2;//how fast should we breathe? increment per showDelay
int breathAmount = 120;//how much should we breathe?
int breathDirection = -1;
float breathProgress = 0;
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

void showTime(){
  long now = millis();

  if (currentPrecipIntensity > 0) {
    rain();
  }

  if (now >= checkShowTime){

    if (now >= checkSlowShowTime) {
      // if (gusting) {
      //   windGust(500);
      //   //add the wind to rain droplets
      //   float windForce = currentWindSpeed/maxWind * 5;
      //   // Serial.println("currentWindSpeed: " + String(currentWindSpeed));
      //   // Serial.println("windForce: " + String(windForce));
      //   for (int i = 0; i < maxDroplets; i++) {
      //     drops[i].applyForce(windForce, 0.5);
      //   }
      // }
      if (wipingDown) {wipeDown();}
      // if (wipeFading) {wipeFade(0.2);}

      checkSlowShowTime = now + slowShowDelay;
    }

    if (now >= checkSuperSlowShowTime){
      if (fadingBack) {fadeBack();}
      checkSuperSlowShowTime = now + superSlowShowDelay;
    }

    // if (now >= checkWindTime) {
    //   Serial.println("wind!");
    //
    //   gusting = true;
    //   checkWindTime = millis() + checkWindDelay;
    // }

    if (!wipingDown) {
      breathe();
    }

    strip.show();
    checkShowTime = now + showTimeInterval;
  }
}

void refreshPixels() {
  Serial.println("Refreshing Pixels");

  int day = floor(currentIndex / 24);
  int hour = reIndex(currentIndex) % 24;
  float mult = defaultBrightness;

  if (hour <= sunrises[day] || hour >= sunsets[day]) {
    Serial.println("dimming");
    mult = sunsetBrightness;
  }

  for (int i = 0; i < 168; i++){
    //showTime();
    int index = reIndex(i);
    pixels[index].multiply(mult);
    //showNextCrawl(i);

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
  }

  wipingDown = true;
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
    Serial.println(String("fading back ") + now + ", time is past " + timeToFade);
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
  int fallingDrops = maxDroplets * currentPrecipIntensity;
  fallingDrops = map(fallingDrops, 0, maxDroplets, 2, maxDroplets);

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

    //Pixel thePixel = pixels[pixelIndex];
    // strip.setPixelColor(
    //   pixelIndex,
    //   thePixel.appRed * theDrop.brightness,
    //   thePixel.appGreen * theDrop.brightness,
    //   thePixel.appBlue * theDrop.brightness
    // );
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

  if (numBack < 167 - 4) {
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

#endif
