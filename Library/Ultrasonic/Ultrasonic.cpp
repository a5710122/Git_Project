/*
  Ultrasonic.cpp - Library for Ultrasonic code.
  Created by 
*/

#include "Arduino.h"
#include "Ultrasonic.h"

Ultrasonic::Ultrasonic(int TRIGGER_PIN, int ECHO_PIN)
{
  pinMode(TRIGGER_PIN, OUTPUT); //3
  pinMode(ECHO_PIN, INPUT); //4
  
  TRIGGER_pin = TRIGGER_PIN;
  ECHO_pin = ECHO_PIN;
}

float Ultrasonic::calUltra()
{
  long duration, distance;
  digitalWrite(TRIGGER_pin, LOW);  // Added this line
  //delayMicroseconds(2); // Added this line
  digitalWrite(TRIGGER_pin, HIGH);
  //delayMicroseconds(10); // Added this line
  digitalWrite(TRIGGER_pin, LOW);
  duration = pulseIn(ECHO_pin, HIGH);
  distance = (duration/2) / 29.1;
  //return(distance);
  //Serial.print("distance: " + distance);
  //Serial.println(" cm");
  
}

