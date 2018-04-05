/*
  Drop.h
  Created by Griffin Mullins
*/

#ifndef Drop_h
#define Drop_h

#include "Arduino.h"

class Drop
{
  public:
    Drop();
    void fall();
    void applyForce(float xForce,float yForce);
    int x;
    int y;
    int pX;
    int pY;
    float drag;
    float xVel;
    float yVel;
    float xAccel;
    float yAccel;
    float brightness;
  private:
    void reset();
    float xFloat;
    float yFloat;
    float gravity;
};

#endif
