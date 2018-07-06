/*
  Ultrasonic.h - Library for Ultrasonic code.
  Created by  
  
*/

#ifndef Ultrasonic_h
#define Ultrasonic_h

#include "Arduino.h"

class Ultrasonic
{
  public:
    Ultrasonic(int TRIGGER_PIN, int ECHO_PIN);
    float calUltra();
    
  private:
    int TRIGGER_pin;
	int ECHO_pin;
};

#endif