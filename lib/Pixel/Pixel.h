/*
  Pixel.h
  Created by Griffin Mullins
*/

#ifndef Pixel_h
#define Pixel_h

#include "Arduino.h"

class Pixel
{
  public:
    int red;
    int green;
    int blue;
    int appRed;
    int appBlue;
    int appGreen;
    Pixel();
    Pixel(int Red, int Green, int Blue);
    Pixel(int Red, int Green, int Blue, int AppRed, int AppBlue, int appGreen);
    void brighten(float amount);
    void multiply(float percent);
    void constraint();
    float percentRed();
    float percentGreen();
    float percentBlue();
  private:
};

#endif
