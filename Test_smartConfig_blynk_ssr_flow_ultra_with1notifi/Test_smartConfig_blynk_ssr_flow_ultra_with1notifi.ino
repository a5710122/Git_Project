#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>     //https://github.com/tzapu/WiFiManager
#include <SPI.h>

// set pin numbers:
#define BLYNK_PRINT Serial
#define D1 5             // Reset WiFi
#define TRIGGER_PIN  D3  //Ultrasonic Yell
#define ECHO_PIN     D4  //Ultrasonic Green
#define HALLSERSON  D5  //Flowmeter
#define control_pump D2        //Control Pump 
#define ConfigWiFi_Pin D1
#define FIREBASE_HOST "https://smartfarmkmutnb.firebaseio.com/"
#define FIREBASE_AUTH "nwAPS7TY5xG1BiKOuDwitUbnKKDiq6nBr27jjSgt"

#define ESP_AP_NAME "ESP8266 Config AP"

// You should get Auth Token in the Blynk App.
char auth[] = "2113a7384617491f88edd8f79d21a3ee";

volatile int NbTopsFan; //measuring the rising edges of the signal
volatile int duration, distance;
volatile int Calc; // Flow meter
int ph_sensor; // pH sensor


int pinValue;

void rpm () {    //This is the function that the interupt calls

  NbTopsFan++; //This function measures the rising and falling edge of the hall effect sensors signal
}

String  TD_Get_Firebase(String path );                                // สำหรับรับ
int     TD_Set_Firebase(String path, String value, bool push=false ); // สำหรับส่ง
int     TD_Push_Firebase(String path, String value ); // สำหรับส่งแบบ Pushing data


BlynkTimer timer;


void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.

  Blynk.virtualWrite(V3, distance); 
  Blynk.virtualWrite(V5, Calc);
}

// ----------------------------------- restore config and reboot system -----------------------


BLYNK_CONNECTED() {
  Blynk.syncAll();
}


void setup() {

  Serial.begin(115200); //This is the setup function where the serial port is initialised,
  pinMode(control_pump, OUTPUT); <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< ห้ามลืม

  // --------------------- WiFi ---------------------------------------
  // initialize pin D0 as an output.
  //pinMode(ledPin, OUTPUT);
  pinMode(ConfigWiFi_Pin, INPUT_PULLUP);

  //digitalWrite(ledPin, LOW); //Turn on the LED
  //WiFiManager
  //Local intialization. Once its business is done, there is no need to keep it around
  WiFiManager wifiManager;
  if (digitalRead(ConfigWiFi_Pin) == LOW) // Press button
  {
    //reset saved settings
    wifiManager.resetSettings(); // go to ip 192.168.4.1 to config
  }
  //fetches ssid and password from EEPROM and tries to connect
  //if it does not connect, it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  wifiManager.autoConnect(ESP_AP_NAME);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(250);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //digitalWrite(ledPin, HIGH);

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

BLYNK_WRITE(V2)
{
  pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  Serial.print("V2  value is: ");
  Serial.println(pinValue);
}


void notifyUptime(){
   if(pinValue == 0){
      if(distance > 35 or distance < 20 ){  //Auto
           Blynk.notify("Water Level abnormal and pump shutdown");
      }
    }
   }
   
void loop() {

  timer.run();

  // ---------------------------- Blynk ----------------------------------------
  //digitalWrite(D0, HIGH);  // turn off the LED
  //delay(1000);             // wait for two seconds
  //digitalWrite(D0, LOW);   // turn on the LED
  //delay(1000);             // wait for two seconds
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
 
  if(distance > 35 or distance < 20 ){  //Auto
    digitalWrite(control_pump, LOW); //Auto
     Blynk.virtualWrite(V0, "Pump : Off");
    Serial.println("Status Pump :LOW");
    }else {
       digitalWrite(control_pump, HIGH); //Auto
       Blynk.virtualWrite(V0, "Pump : On");
       Serial.println("Status Pump :HIGH");
    }
  
}

/**********************************************************
 * ฟังกชั่น TD_Set_Firebase 
 * ใช้สำหรับ ESP8266 กำหนด ค่า value ให้ path ของ Firebase
 * โดย path อยู่ใน รูปแบบ เช่น "Room/Sensor/DHT/Humid" เป็นต้น
 * 
 * ทั้ง path และ  value ต้องเป็น ข้อมูลประเภท String
 * และ คืนค่าฟังกชั่น กลับมาด้วย http code
 * 
 * เช่น หากเชื่อมต่อไม่ได้ จะคินค่าด้วย 404 
 * หากกำหนดลงที่ Firebase สำเร็จ จะคืนค่า 200 เป็นต้น
 * 
 **********************************************************/
//#ifndef FIREBASE_FINGERPRINT  // Fingerprint ของ https://www.firebaseio.com
//#define FIREBASE_FINGERPRINT  "B8 4F 40 70 0C 63 90 E0 07 E8 7D BD B4 11 D0 4A EA 9C 90 F6"
//#endif
//
//int TD_Set_Firebase(String path, String value , bool push) {
//  WiFiClientSecure ssl_client;
//  String host = String(FIREBASE_HOST); host.replace("https://", "");
//  if(host[host.length()-1] == '/' ) host = host.substring(0,host.length()-1);
//  String resp = "";
//  int httpCode = 404; // Not Found
//
//  String firebase_method = (push)? "POST " : "PUT ";
//  if( ssl_client.connect( host, 443)){
//    if( ssl_client.verify( FIREBASE_FINGERPRINT, host.c_str())){
//      String uri = ((path[0]!='/')? String("/"):String("")) + path + String(".json?auth=") + String(FIREBASE_AUTH);      
//      String request = "";
//            request +=  firebase_method + uri +" HTTP/1.1\r\n";
//            request += "Host: " + host + "\r\n";
//            request += "User-Agent: ESP8266\r\n";
//            request += "Connection: close\r\n";
//            request += "Accept-Encoding: identity;q=1,chunked;q=0.1,*;q=0\r\n";
//            request += "Content-Length: "+String( value.length())+"\r\n\r\n";
//            request += value;
//
//      ssl_client.print(request);
//      while( ssl_client.connected() && !ssl_client.available()) delay(10);
//      if( ssl_client.connected() && ssl_client.available() ) {
//        resp      = ssl_client.readStringUntil('\n');
//        httpCode  = resp.substring(resp.indexOf(" ")+1, resp.indexOf(" ", resp.indexOf(" ")+1)).toInt();
//      }
//    }else{
//      Serial.println("[Firebase] can't verify SSL fingerprint");
//    }
//    ssl_client.stop();    
//  } else {
//    Serial.println("[Firebase] can't connect to Firebase Host");
//  }
//  return httpCode;
//}

