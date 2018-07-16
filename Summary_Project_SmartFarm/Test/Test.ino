#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Ultrasonic.h>
#include <FlowMeter.h>

FlowMeter Meter = FlowMeter(14); //D ??
Ultrasonic ultrasonic(0, 2); //Ultrasonic D3, D4
LiquidCrystal_I2C lcd(0x27, 16, 2); //LCD D1, D2

#define control_pump D0 //Control Pump D0
//const int control_button = 12;     // pin push change button  ???
//const int manual_button = 13;     // pin push change button  ???
const int buttonPin = 10;     // pin push button
const int buttonSW = 9;

int currentButtonState = LOW; // state button now
int previousButtonState = LOW;        // state Button previous

int currentButtonM = LOW; // ค่าสถานะปัจจุบนของปุ่ม
int previousButtonM = LOW;        // ค่าสถานะของปุ่มกดครั้งที่แล้ว

long stage = 0; // state button now

float Flow ;
float Ultra ;
const unsigned long period = 1000; //Constant Flowmeter Interrupt
String show_stage;
unsigned long sensor_millis;
bool stage_manual_pump = LOW;

void setup() {
  Serial.begin(115200);
  sensor_millis = millis();

  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.print("SmartFarm");
  pinMode(buttonPin, INPUT);
  pinMode(buttonSW, INPUT);
  pinMode(control_pump, OUTPUT);
  attachInterrupt(9, control_manuel, CHANGE); //interrupt if switch control manual
  attachInterrupt(10, control_manuel_pump, CHANGE); //interrupt if switch control pump manual

  attachInterrupt(14, MeterISR, RISING); //interrupt fan Flowmeter
  Meter.reset();

}

void loop() {

  unsigned long now_millis = millis();
  if (now_millis - sensor_millis > 1000) {
    Read_value_form_sensor();
    sensor_millis = now_millis;
  }

  Show_Status_LCD();

  Serial.println(stage);

  switch (stage) {

    case 0: //case 1

      if (Ultra > 35 or Ultra < 20) { //Auto
        digitalWrite(control_pump, LOW); //Auto
        Serial.println("Auto_pump: Status Pump :LOW");
      } else {
        digitalWrite(control_pump, HIGH); //Auto
        Serial.println("Auto_pump: Status Pump :HIGH");
      }
      show_stage = "auto  ";
      break;

    case 1: //case 2 Auto control

      if (stage_manual_pump == HIGH) {
        digitalWrite(control_pump, HIGH);
        Serial.println("manual_pump: control pump ON");

      } else if (stage_manual_pump == LOW) {
        digitalWrite(control_pump, LOW);
        Serial.println("manual_pump: control pump OFF");
      }
      show_stage = "manual";
      break;

    default:
      if (stage >= 3) {
        stage = 0;
      }

  }
}

//==============================================================
//                     FUNCTION
//==============================================================

void MeterISR() {
  //flowmeter count the pulses
  Meter.count();
}

void Read_value_form_sensor() {
  Flow = Meter.getCurrentFlowrate();
  Meter.tick(period);   // process the (possibly) counted ticks
  Ultra = ultrasonic.calUltra();
  Serial.println("Ultra" + String(Ultra));
  Serial.println("Flow" + String(Flow));
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
  delay(500);
  positionCounter >= 20 ? positionCounter = 0 : positionCounter ++ ;

}

void control_manuel() {
  currentButtonM = digitalRead(buttonSW);
  Serial.print("in to control manule: " );
  Serial.println(currentButtonM);

}

void control_manuel_pump() {
  currentButtonState = digitalRead(buttonPin);
  Serial.print("in to control manule pump: " );
  Serial.println(currentButtonState);
}

