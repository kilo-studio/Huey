/*
  Pixel.cpp
  Created by Griffin Mullins
*/

#include "Arduino.h"
#include "Pixel.h"



Pixel::Pixel()
{
  red = 0;
  green = 0;
  blue = 0;
  appRed = 0;
  appGreen = 0;
  appBlue = 0;
}

Pixel::Pixel(int Red, int Green, int Blue)
{
  red = Red;
  green = Green;
  blue = Blue;
  appRed = 0;
  appGreen = 0;
  appBlue = 0;
}

Pixel::Pixel(int Red, int Green, int Blue, int AppRed, int AppBlue, int AppGreen)
{
  red = Red;
  green = Green;
  blue = Blue;
  appRed = AppRed;
  appGreen = AppGreen;
  appBlue = AppBlue;
}

void Pixel::brighten(float amount){
  red += (amount) * red;
  green += (amount) * green;
  blue += (amount) * blue;
  appRed += (amount) * appRed;
  appGreen += (amount) * appGreen;
  appBlue += (amount) * appBlue;
  constraint();
}

void Pixel::multiply(float value){
  red *= value;
  blue *= value;
  green *= value;
  appRed *= value;
  appGreen *= value;
  appBlue *= value;
  // constraint();
}

void Pixel::divide(float value){
  red = red/value;
  blue = blue/value;
  green = green/value;
  appRed = appRed/value;
  appGreen = appGreen/value;
  appBlue = appBlue/value;
  // constraint();
}

void Pixel::constraint(){
  red = constrain(red, 0, 255);
  green = constrain(green, 0, 255);
  blue = constrain(blue, 0, 255);
  appRed = constrain(appRed, 0, 255);
  appGreen = constrain(appGreen, 0, 255);
  appBlue = constrain(appBlue, 0, 255);
}

float Pixel::percentRed(){
  int sum = red+green+blue;
  return float(red) / sum;
}

float Pixel::percentGreen(){
  int sum = red+green+blue;
  return float(green) / sum;
}
float Pixel::percentBlue(){
  int sum = red+green+blue;
  return float(blue) / sum;
}
