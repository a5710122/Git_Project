#include <Ultrasonic.h>

Ultrasonic ultrasonic(0, 2);

void setup()
{
  Serial.begin (115200);
}

void loop()
{
  ultrasonic.calUltra();
}



