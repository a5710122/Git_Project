#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <SPI.h>
#include <SD.h>
#include <FlowMeter.h>
#include <Ultrasonic.h>
#include "index.h" //Our HTML webpage contents with javascripts
// ------------------------------ Call library ------------------------------- //
FlowMeter Meter = FlowMeter(2);
Ultrasonic ultrasonic(0, 2);
// -------------------------------------------------------------------------- //

#define control_pump D2        //Control Pump 



File myFile;

const unsigned long period = 1000; //Flowmeter Interrupt
unsigned long sd_millis; //Count time to save value to sdcard
unsigned long control_millis;
boolean manual = false;
String pump_manual = "ON";

//SSID and Password of your WiFi router
const char* ssid = "Maka";
const char* password = " ";
ESP8266WebServer server(80); //Server on port 80

//==============================================================
//                  SETUP
//==============================================================

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_AP); // Config WiFi in Mode AP
  WiFi.softAP("ESP SmartFarm"); // Config name WiFi
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  server.begin();                  //Start server

  pinMode(control_pump, OUTPUT);
  sd_millis = millis();
  control_millis = millis();

  // enable a call to the 'interrupt service handler' (ISR) on every rising edge at the interrupt pin
  // do this setup step for every ISR you have defined, depending on how many interrupts you use
  attachInterrupt(2, MeterISR, RISING);
  // sometimes initializing the gear generates some pulses that we should ignore
  Meter.reset();
  checkSD();

  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/setLED1", handleLED1);
  server.on("/setLED2", handleLED2);
  server.on("/readValue", handleADC);

}

//==============================================================
//                     LOOP
//==============================================================
void loop() {
  
  server.handleClient();          //Handle client requests
  delay(period);    // wait between output updates
  Meter.tick(period);   // process the (possibly) counted ticks
  Serial.println("Currently Flow: " + String(Meter.getCurrentFlowrate()) + " l/min, " + "Total Flow Volume: " + String(Meter.getTotalVolume()));
  Serial.println("Currently Ultrasonic: " + String(ultrasonic.calUltra()));


  unsigned long now_millis = millis();
  if (now_millis - control_millis > 10000) {   //Control 
    
    if ( manual == false) {
      if (ultrasonic.calUltra() > 35 or ultrasonic.calUltra() < 20 ) { //Auto
        digitalWrite(control_pump, LOW); //Auto
        Serial.println("Status Pump :LOW");
      } else {
        digitalWrite(control_pump, HIGH); //Auto
        Serial.println("Status Pump :HIGH");
      }
    } else {
      if (pump_manual == "ON") {
        digitalWrite(control_pump, HIGH);
        Serial.println("Status Pump :HIGH");
      } else {
        digitalWrite(control_pump, LOW);
        Serial.println("Status Pump :LOW");

      }
    }
    control_millis = now_millis;
  }
  
  if (now_millis - sd_millis > 60*15*1000) {
    writeData();
    sd_millis = now_millis;
  }

}


//==============================================================
//                     OTHET FUNCTION
//==============================================================

// -------------------------------- Function Flow meter --------------------------------- //
// define an 'interrupt service handler' (ISR) for every interrupt pin you use //
void MeterISR() {
  // let our flow meter count the pulses
  Meter.count();
}
// -------------------------------------------------------------------------------------- //

void checkSD() {
  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

// ----------------------------------- SD Card ------------------------------------ //
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

// ---------------------------------------- AP Control ----------------------------------------- //
void handleRoot() {
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

void handleADC() {
  //int a = analogRead(A0);
  //String adcValue = String(a);
  //Serial.print("R: ");
  String flow = String(Meter.getCurrentFlowrate());
  String distan = String(ultrasonic.calUltra());
  server.send(200, "text/plane", flow); //Send ADC value only to client ajax request
  server.send(200, "text/plane", distan); //Send ADC value only to client ajax request
}

void handleLED1() {
  String ledState1 = "OFF";
  String t_state = server.arg("LEDstate1"); //Refer  xhttp.open("GET", "setLED?LEDstate="+led, true);
  Serial.print("manual State: ");
  Serial.println(t_state);
  if (t_state == "1") {
    manual = false; //Feedback parameter
    ledState1 = "OFF"; //Feedback parameter
  } else {
    manual = true; //Feedback parameter
    ledState1 = "ON"; //Feedback parameter
  }
  server.send(200, "text/plane", ledState1); //Send web page
}

void handleLED2() {
  String ledState2 = "OFF";
  String t_state = server.arg("LEDstate2"); //Refer  xhttp.open("GET", "setLED?LEDstate="+led, true);
  Serial.print("pump_manual State: ");
  Serial.println(t_state);
  if (t_state == "1")
  {

    pump_manual = "ON"; //Feedback parameter
    ledState2 = "ON"; //Feedback parameter
  }
  else
  {
    pump_manual = "OFF"; //Feedback parameter
    ledState2 = "OFF"; //Feedback parameter
  }

  server.send(200, "text/plane", ledState2); //Send web page
}

