/*
 * ESP8266 NodeMCU AJAX Demo
 * Updates and Gets data from webpage without page refresh
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "index.h" //Our HTML webpage contents with javascripts

#define LED1 D1  //On board LED
#define LED2 D2  //On board LED

//SSID and Password of your WiFi router
const char* ssid = "silver";
const char* password = "025405730";

ESP8266WebServer server(80); //Server on port 80

//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
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
 Serial.print("LED 1 State: ");
 Serial.println(t_state);
 if(t_state == "1")
 {
  digitalWrite(LED1,HIGH); //LED ON
  ledState1 = "ON"; //Feedback parameter
 }
 else
 {
  digitalWrite(LED1,LOW); //LED OFF
  ledState1 = "OFF"; //Feedback parameter  
 }
 
 server.send(200, "text/plane", ledState1); //Send web page
}

void handleLED2() {
 String ledState2 = "OFF";
 String t_state = server.arg("LEDstate2"); //Refer  xhttp.open("GET", "setLED?LEDstate="+led, true);
 Serial.print("LED 2 State: ");
 Serial.println(t_state);
 if(t_state == "1")
 {
  digitalWrite(LED2,HIGH); //LED ON
  ledState2 = "ON"; //Feedback parameter
 }
 else
 {
  digitalWrite(LED2,LOW); //LED OFF
  ledState2 = "OFF"; //Feedback parameter  
 }
 
 server.send(200, "text/plane", ledState2); //Send web page
}

//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);
  
   WiFi.mode(WIFI_AP); // ใช้งาน WiFi ในโหมด AP
   WiFi.softAP("ESP"); // ตั้งให้ชื่อ WiFi เป็น ESP_IOXhop
   Serial.println("");
   server.begin();                  //Start server
   
  //Onboard LED port Direction output
  pinMode(LED1,OUTPUT); 
  pinMode(LED2,OUTPUT); 
  
  
 
  server.on("/", handleRoot);      //Which routine to handle at root location. This is display page
  server.on("/setLED1", handleLED1);
  server.on("/setLED2", handleLED2);
  server.on("/readADC", handleADC);

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
  
}
//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  
  server.handleClient();          //Handle client requests
  
}
