#include <SPI.h>
#include <SD.h>
#include <FlowMeter.h>
#include <Ultrasonic.h>
#include "index.h"   //Our HTML webpage contents with javascripts
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


#define control_pump D0        
const int buttonPin = 10;     // pin Switch control pump manual
const int buttonSW = 9;       // pin push button
//==============================================================
//                  CALL LIBRARY
//==============================================================

FlowMeter Meter = FlowMeter(3); //D2 -> D9 
Ultrasonic ultrasonic(0, 2); // D3, D4
File myFile;
LiquidCrystal_I2C lcd(0x3F, 16, 2);


//==============================================================
//                  VARIABLE
//==============================================================

const unsigned long period = 1000;     //Flowmeter Interrupt
unsigned long sd_millis;      //Count time to save value to sdcard

int currentButtonState = LOW; // state button now
int previousButtonState = LOW;        // state Button previous
bool isLedOn = false;                 // state led
int currentButtonM = LOW; // ค่าสถานะปัจจุบนของปุ่ม

//==============================================================
//                  SETUP
//==============================================================

void setup() {
  
  Serial.begin(115200);
  lcd.begin();
  lcd.print("Ultrasonic: ");
  pinMode(buttonPin, INPUT);
  pinMode(buttonSW, INPUT);
  pinMode(control_pump, OUTPUT);
  sd_millis = millis();
  
  attachInterrupt(4, MeterISR, RISING);     //Interrupt Flow meter

  attachInterrupt(9, control_manual, CHANGE);     //Interrupt switch control manuel
  attachInterrupt(10, control_manual_pump, CHANGE);     //Interrupt switch control pump manuel

  Meter.reset();     // sometimes initializing the gear generates some pulses that we should ignore
  checkSD();     //Call function "check SD card" 
  

}

//==============================================================
//                     LOOP
//==============================================================

void loop() {
  float ultra = ultrasonic.calUltra();
  lcd.setCursor(0, 1);
  lcd.print("Ultrasonic: ");
  lcd.setCursor(1, 0);
  lcd.print(ultra);
  lcd.clear();
  
  
  
  
  //delay(period);    // wait between output updates
  //Meter.tick(period);   // process the (possibly) counted ticks
  //Serial.println("Currently Flow: " + String(Meter.getCurrentFlowrate()) + " l/min, " + "Total Flow Volume: " + String(Meter.getTotalVolume()));
  Serial.print("Currently Ultrasonic: ");
  Serial.println(ultra);

  if (currentButtonM == LOW) {     // Auto Mode
    
    if (ultra > 35 or ultra < 20 ) { 
      digitalWrite(control_pump, LOW); //Auto
      Serial.println("Auto_pump: Status Pump :LOW");
    } else {
      digitalWrite(control_pump, HIGH); //Auto
      Serial.println("Auto_pump: Status Pump :HIGH");
    }
  }

  if (currentButtonM == HIGH) {     // manual Mode
    
    if (currentButtonState == HIGH) {
      digitalWrite(control_pump, HIGH);
      Serial.println("manual_pump: control pump ON");
      
    }else if (currentButtonState == LOW){
      digitalWrite(control_pump, LOW);
      Serial.println("manual_pump: control pump OFF");
    } 
  }

  unsigned long now_millis = millis();
  if (now_millis - sd_millis > 60 * 15 * 1000) {     //Check time for save value
    writeData();
    sd_millis = now_millis;
  }
}

//==============================================================
//                     OTHET FUNCTION
//==============================================================

// ---------- Control Manual & Control Pump Manual -------------

void control_manual() {
  currentButtonM = digitalRead(buttonSW);
  Serial.print("in to control manual: " );
  Serial.println(currentButtonM);

}

void control_manual_pump() {
  currentButtonState = digitalRead(buttonPin);
  Serial.print("in to control manual pump: " );
  Serial.println(currentButtonState);
}


// -------------------------------- Function Flow meter --------------------------------- //
// define an 'interrupt service handler' (ISR) for every interrupt pin you use //
void MeterISR() {
  // let our flow meter count the pulses
  Meter.count();
}

// ----------------------------------- SD Card ------------------------------------ //

void checkSD() {
  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

void writeData() {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.println("Writing to test.txt...");
    myFile.println("Currently Flow: " + String(Meter.getCurrentFlowrate()) + " l/min, " + "Total Flow Volume: " + String(Meter.getTotalVolume()));
    myFile.println("Currently Ultrasonic: " + String(ultrasonic.calUltra()));
    // close the file:
    myFile.close();
    Serial.println("Writing to test.txt done.");
    readData();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}

void readData() {

  // re-open the file for reading:
  myFile = SD.open("test.txt");
  if (myFile) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening test.txt");
  }
}


