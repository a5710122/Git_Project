#include <SPI.h>
#include <SD.h>
#include <FlowMeter.h>
#include <Ultrasonic.h>
#include "index.h" //Our HTML webpage contents with javascripts
// ------------------------------ Call library ------------------------------- //
FlowMeter Meter = FlowMeter(4);
Ultrasonic ultrasonic(0, 2);
// -------------------------------------------------------------------------- //

#define control_pump D0        //Control Pump 

File myFile;

const unsigned long period = 1000; //Flowmeter Interrupt
unsigned long sd_millis; //Count time to save value to sdcard
unsigned long control_millis;

const int buttonPin = 10;     // pin push button
const int buttonSW = 9;

int currentButtonState = LOW; // state button now
int previousButtonState = LOW;        // state Button previous
bool isLedOn = false;                 // state led
int currentButtonM = LOW; // ค่าสถานะปัจจุบนของปุ่ม
int previousButtonM = LOW;        // ค่าสถานะของปุ่มกดครั้งที่แล้ว

//==============================================================
//                  SETUP
//==============================================================

void setup() {
  Serial.begin(115200);


  pinMode(buttonPin, INPUT);
  pinMode(buttonSW, INPUT);
  pinMode(control_pump, OUTPUT);
  sd_millis = millis();
  control_millis = millis();

  // enable a call to the 'interrupt service handler' (ISR) on every rising edge at the interrupt pin
  // do this setup step for every ISR you have defined, depending on how many interrupts you use
  attachInterrupt(4, MeterISR, RISING);

  attachInterrupt(9, control_manuel, CHANGE   ); // pump
  attachInterrupt(10, control_manuel_pump, CHANGE   ); // in to manuel

  // sometimes initializing the gear generates some pulses that we should ignore
  Meter.reset();
  checkSD();

}

//==============================================================
//                     LOOP
//==============================================================
void loop() {

  delay(period);    // wait between output updates
  Meter.tick(period);   // process the (possibly) counted ticks
  //Serial.println("Currently Flow: " + String(Meter.getCurrentFlowrate()) + " l/min, " + "Total Flow Volume: " + String(Meter.getTotalVolume()));
  //Serial.println("Currently Ultrasonic: " + String(ultrasonic.calUltra()));

  if (currentButtonM == LOW) {
    Serial.println("in to control_Auto_pump");
    if (ultrasonic.calUltra() > 35 or ultrasonic.calUltra() < 20 ) { //Auto
      digitalWrite(control_pump, LOW); //Auto
      Serial.println("Status Pump :LOW");
    } else {
      digitalWrite(control_pump, HIGH); //Auto
      Serial.println("Status Pump :HIGH");
    }
  }

  if (currentButtonM == HIGH) {
    Serial.println("in to control_manuel_pump");
    if (currentButtonState == HIGH) {
      digitalWrite(control_pump, HIGH);
      Serial.println("control pump ON");
      
    }else if (currentButtonState == LOW){
      digitalWrite(control_pump, LOW);
      Serial.println("control pump OFF");
    }
    
  }

  unsigned long now_millis = millis();
  if (now_millis - sd_millis > 60 * 15 * 1000) {
    writeData();
    sd_millis = now_millis;
  }


}


//==============================================================
//                     OTHET FUNCTION
//==============================================================

// --------------------------------------------------------------
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


