#include <FlowMeter.h>
#include <Ultrasonic.h>
#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include <LiquidCrystal_I2C.h>

// Basic pin reading and pullup test for the MCP23017 I/O expander
// public domain!

// Connect pin #12 of the expander to Analog 5 (i2c clock)
// Connect pin #13 of the expander to Analog 4 (i2c data)
// Connect pins #15, 16 and 17 of the expander to ground (address selection)
// Connect pin #9 of the expander to 5V (power)
// Connect pin #10 of the expander to ground (common ground)
// Connect pin #18 through a ~10kohm resistor to 5V (reset pin, active low)

// Output #0 is on pin 21 so connect an LED or whatever from that to ground

Adafruit_MCP23017 mcp;
LiquidCrystal_I2C lcd(0x27, 16, 2); //LCD D1, D2
Ultrasonic ultrasonic(16, 0); //Ultrasonic D0, D3
FlowMeter Meter = FlowMeter(2); //D4 ??
#define MCP_control_pump 8 //Control Pump MCP23017 pin 1 - "8"

float Ultra ; // value ultrasonic

float Flow ; // value flowmeter
const unsigned long period = 1000; //Constant Flowmeter Interrupt

unsigned long sensor_millis; //Count time to take value sensor

long stage = 0; // state button now
String show_stage; //show stage to lcd

void setup() {
  sensor_millis = millis();
  mcp.begin(); // use default address 0
  Serial.begin (115200);
  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.print("SmartFarm");
  mcp.pinMode(0, OUTPUT);
  mcp.pinMode(MCP_control_pump, OUTPUT);

  attachInterrupt(14, MeterISR, RISING); //interrupt fan Flowmeter
  Meter.reset();

}


void loop() {
  delay(100);
  mcp.digitalWrite(0, HIGH);
  delay(100);
  mcp.digitalWrite(0, LOW);

  unsigned long now_millis = millis();
  if (now_millis - sensor_millis > 1000) {
    Read_value_form_sensor();
    sensor_millis = now_millis;
  }
  Show_Status_LCD();
  
  switch (stage) {

    case 0: //case 1

      if (Ultra > 35 or Ultra < 20) { //Auto
        mcp.digitalWrite(MCP_control_pump, LOW); //Auto
        Serial.println("Auto_pump: Status Pump :LOW");
      } else {
        mcp.digitalWrite(MCP_control_pump, HIGH); //Auto
        Serial.println("Auto_pump: Status Pump :HIGH");
      }
      show_stage = "auto  ";
      break;

    case 1: //case 2 Auto control

//      if (stage_manual_pump == HIGH) {
//        mcp.digitalWrite(MCP_control_pump, HIGH);
//        Serial.println("manual_pump: control pump ON");
//
//      } else if (stage_manual_pump == LOW) {
//        mcp.digitalWrite(MCP_control_pump, LOW);
//        Serial.println("manual_pump: control pump OFF");
//      }
      show_stage = "manual";
      break;

    default:
      if (stage >= 3) {
        stage = 0;
      }

  }

}

void Read_value_form_sensor() {
  Flow = Meter.getCurrentFlowrate();
  Meter.tick(period);   // process the (possibly) counted ticks
  Ultra = ultrasonic.calUltra();
  Serial.println("Ultra" + String(Ultra));
  Serial.println("Flow" + String(Flow));
}

void MeterISR() {
  //flowmeter count the pulses
  Meter.count();
}

int positionCounter = 0;

void Show_Status_LCD() {
  //Send value to LCD 1604 i2c 16x2
  lcd.setCursor(10, 0);
  lcd.print("Stage: " + show_stage);
  lcd.setCursor(0, 1);
  lcd.print("WaterLevel: " + String(Ultra) + " cm");
  lcd.print(" Flow: " + String(Flow) + " L/m");
  lcd.scrollDisplayLeft();  // scroll one position left:
  delay(300);
  positionCounter >= 20 ? positionCounter = 0 : positionCounter ++ ;

}



