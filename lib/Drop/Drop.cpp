/*
  Drop.cpp
  Created by Griffin Mullins
*/

#include "Arduino.h"
#include "Drop.h"

Drop::Drop()
{
  xVel = 0;
  xAccel = 0;
  reset();
}

void Drop::fall()
{
  applyForce( xAccel, gravity);

  float xFriction = xVel;
  xFriction = xFriction * drag;
  applyForce(xFriction, 0);

  pX = x;
  xVel = xVel + xAccel;
  xFloat = xFloat + xVel/100;
  x = floor(xFloat);
  if (x > 7) {
    x = 0;
    xFloat = 0;
  }

  pY = y;
  yVel = yVel + yAccel;
  yFloat = yFloat + yVel/100;
  y = floor(yFloat);
  if (y > 24) {
    reset();
  }

  xAccel = 0;
  yAccel = 0;
}

void Drop::reset()
{
  x = floor(random(0, 7));
  pX = x;
  xFloat = x;

  y = 0;
  pY = -1;
  yFloat = y;
  yVel = random(2, 15);
  yAccel = 0;//make acceleration 5 for freezing rain!

  drag = -0.008;
  gravity = 0.02;
  brightness = random(60, 90)/100.0;
}

void Drop::applyForce(float xForce,float yForce){
  xAccel = xAccel + xForce;
  yAccel = yAccel + yForce;
}
