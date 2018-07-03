#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <BlynkSimpleEsp8266.h>
#include <DNSServer.h>
#include <SPI.h>

// --------------------- Templast HTML -------------------------------------- //
#include "index.h" //Our HTML webpage contents with javascripts

// set pin numbers:
#define BLYNK_PRINT Serial // BLYNK
#define TRIGGER_PIN  D3  //Ultrasonic Yell
#define ECHO_PIN     D4  //Ultrasonic Green
#define HALLSERSON  D5  //Flowmeter
#define control_pump D2        //Control Pump 
#define ESP_AP_NAME "ESP8266 Config AP"

// -------------------------- SSID and Password of your WiFi router ----------------------- //
const char* ssid = "Maka";
const char* password = "";
ESP8266WebServer server(80); //Server on port 80

// -------------------------- You should get Auth Token in the Blynk App. ----------------- //
char auth[] = "2113a7384617491f88edd8f79d21a3ee";


// -------------------------- This routine is executed when you open its IP in browser ---- //
void handleRoot() {
  String s = MAIN_page; //Read HTML contents
  server.send(200, "text/html", s); //Send web page
}

void handleADC() {
  int a = analogRead(A0);
  String adcValue = String(a);
  Serial.print("R: ");
  Serial.println(a);
  server.send(200, "text/plane", adcValue); //Send ADC value only to client ajax request
}

void handleLED1() {
  String ledState1 = "OFF";
  String t_state = server.arg("LEDstate1"); //Refer  xhttp.open("GET", "setLED?LEDstate="+led, true);
  Serial.print("control_pump State: ");
  Serial.println(t_state);
  if (t_state == "1")
  {
    digitalWrite(control_pump, HIGH); //LED ON
    ledState1 = "ON"; //Feedback parameter
  }
  else
  {
    digitalWrite(control_pump, LOW); //LED OFF
    ledState1 = "OFF"; //Feedback parameter
  }

  server.send(200, "text/plane", ledState1); //Send web page
}


// -------------------------- variable --------------------------------------------------//
volatile int NbTopsFan; //measuring the rising edges of the signal
volatile int duration, distance;
volatile int Calc; // Flow meter
int ph_sensor; // pH sensor
int pinValue;
void rpm () {    //This is the function that the interupt calls

  NbTopsFan++; //This function measures the rising and falling edge of the hall effect sensors signal
}

BlynkTimer timer;

void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.

  Blynk.virtualWrite(V3, distance);
  Blynk.virtualWrite(V5, Calc);
}

void notifyUptime() {
  if (pinValue == 0) {
    if (distance > 35 or distance < 20 ) { //Auto
      Blynk.notify("Water Level abnormal and pump shutdown");
    }
  }
}

BLYNK_WRITE(V2) {
  pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  Serial.print("V2  value is: ");
  Serial.println(pinValue);
}

// ----------------------------------- restore config and reboot system -----------------------


BLYNK_CONNECTED() {
  Blynk.syncAll();
}

/// --------------------------------------------------------------------------------------------//
void setup() {

  // -------------------- Serial -------------------------------------- //
  Serial.begin(115200); //This is the setup function where the serial port is initialised,
  pinMode(control_pump, OUTPUT);

  // --------------------- WiFi --------------------------------------- //
  WiFi.mode(WIFI_AP); // ใช้งาน WiFi ในโหมด AP
  WiFi.softAP("ESP_SmartFarm"); // ตั้งให้ชื่อ WiFi เป็น ESP
  Serial.println("");
  server.begin();                  //Start server

  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/setLED1", handleLED1);
  server.on("/readADC", handleADC);

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // --------------------------- Blynk --------------------------------------
  Blynk.config(auth);

  // --------------------------- Flowmeter ---------------------------------------
  pinMode(HALLSERSON, INPUT); //initializes digital pin 2 as an input
  attachInterrupt(14, rpm, RISING); //and the interrupt is attached

  // --------------------------- Ultrasonic ---------------------------------------
  pinMode(TRIGGER_PIN, OUTPUT); // initiallizes Ultrasonic as trigger as pin D3
  pinMode(ECHO_PIN, INPUT);     // initiallizes Ultrasonic as trigger as pin D4

  timer.setInterval(1000L, myTimerEvent);

  timer.setInterval(2000L, notifyUptime);

}

void loop() {

  timer.run();

  // ---------------------------- Server AP ------------------------------------ //
  server.handleClient();          //Handle client requests
  // ---------------------------- Blynk ----------------------------------------

  Blynk.run();

  // ---------------------------- Flow meter -------------------------------------
  NbTopsFan = 0;     //Set NbTops to 0 ready for calculations
  sei();           //Enables interrupts
  delay (1000);     //Wait 1 second
  cli();           //Disable interrupts
  Calc = (NbTopsFan  / 7.5); //(Pulse frequency x 60) / 7.5Q, = flow rate in L/hour
  Blynk.virtualWrite(V5, Calc); //Send data from flow meter to Blynk
  Serial.print (Calc, DEC); //Prints the number calculated above
  Serial.print (" L/hour\r\n"); //Prints "L/hour" and returns a new line
  // TD_Set_Firebase("Flow meter", String(Calc));

  // ---------------------------- Ultrasonic -------------------------------------
  digitalWrite(TRIGGER_PIN, LOW);  // Added this line
  delayMicroseconds(2); // Added this line
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10); // Added this line
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  //distance = 30;
  distance = (duration / 2) / 29.1;
  Serial.print(distance);
  Serial.println(" cm");
  Blynk.virtualWrite(V3, distance); //Send data from flow meter to Blynk
  //TD_Set_Firebase("Ultrasonic", String(distance));


  // ---------------------------------- Test connect Blynk ------------------------
  if (Blynk.connected() == true) {
    Serial.println("Connected to Blynk server");
    //Blynk.virtualWrite(V2,"Connected to Blynk server" );
  }
  // ------------------------------------------------------------------------------- //
  //if (distance > 35 or distance < 20 ) { //Auto
    //digitalWrite(control_pump, LOW); //Auto
    //Blynk.virtualWrite(V0, "Pump : Off");
    //Serial.println("Status Pump :LOW");
  //} else {
    //digitalWrite(control_pump, HIGH); //Auto
    //Blynk.virtualWrite(V0, "Pump : On");
    //Serial.println("Status Pump :HIGH");
  //}

}


