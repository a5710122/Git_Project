#include <FlowMeter.h>
#include <Ultrasonic.h>
#include <Wire.h>
#include "Adafruit_MCP23017.h"
#include <LiquidCrystal_I2C.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Basic pin reading and pullup test for the MCP23017 I/O expander
// Connect pin #12 of the expander to Analog 5 (i2c clock)
// Connect pin #13 of the expander to Analog 4 (i2c data)
// Connect pins #15, 16 and 17 of the expander to ground (address selection)
// Connect pin #9 of the expander to 5V (power)
// Connect pin #10 of the expander to ground (common ground)
// Connect pin #18 through a ~10kohm resistor to 5V (reset pin, active low)
// Output #0 is on pin 21 so connect an LED or whatever from that to ground

//===================================== PIN ================================//

Adafruit_MCP23017 mcp; //MCP23017
LiquidCrystal_I2C lcd(0x27, 16, 2); //LCD D1, D2
Ultrasonic ultrasonic(16, 0); //Ultrasonic D0, D3
FlowMeter Meter = FlowMeter(2); //Flowmeter D4
#define MCP_control_pump 8 //Control Pump MCP23017 pin 1 - "8"
const int switch_stage = 10;   //switch control stage GPIO 10 buttonPin
const int switch_manual_pump = 9;   //Switch control manual GPIO 9 buttonSW
const int led = 16;

//===================================== VARIABLE ================================//

float Ultra ; // value ultrasonic
float Flow ; // value flowmeter
const unsigned long period = 1000; //Constant Flowmeter Interrupt
unsigned long sensor_millis; //Count time to take value sensor
unsigned long check_millis; //Count time to take reconnect server


long stage = 0; // state button now
String show_stage; //show stage to lcd

boolean lastState; //Last stage pump (manual)
boolean manual_pump = LOW; //Stage pump (manual)

//===================================== MQTT ================================//

const char* ssid = "Maka";
const char* password =  "";

const char* mqttServer = "172.19.201.179";
const int mqttPort = 1883;
const char* mqttUser = "s5710122";
const char* mqttPassword = "ap18659993";
WiFiClient espClient;
PubSubClient client(espClient);
long connect_server;

//================================== CODE =================================//

void setup() {
  sensor_millis = millis(); //Set time to count for take value
  check_millis = millis();  //Set time to count for reconnect server
  mcp.begin(); // MCP use default address 0
  Serial.begin (115200);
  lcd.begin(); //LCD
  lcd.setCursor(0, 0);
  lcd.print("SmartFarm");
  lcd.clear();

  pinMode(switch_stage, INPUT); //Call pin in nodemcu input
  pinMode(switch_manual_pump, INPUT);

  mcp.pinMode(0, OUTPUT); //Call pin mcp23017
  mcp.pinMode(MCP_control_pump, OUTPUT);

  attachInterrupt(switch_stage, change_stage, RISING); //interrupt stage
  attachInterrupt(switch_manual_pump, change_stage_manual, RISING); //interrupt pump
  attachInterrupt(2, MeterISR, RISING); //interrupt fan Flowmeter
  Meter.reset();

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    if (client.connect("ESP8266Client" )) { //Connect to MQTT และกำหนดชื่อในการติดต่อ
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }

  //ส่งข้อมูล publish ไปที่ MQTT Broker โดยตั้ง topic เป็น "esp/test"
  client.publish("esp/pump", "Hello from ESP8266");

  //subscribe topic "esp/test"
  client.subscribe("esp/pump");


}


void loop() {

  unsigned long now_millis = millis();
  if (now_millis - sensor_millis > 2000) {
    Read_value_form_sensor();
    char buf_ultra[5]; /// array char
    String valor_ultra = String(Ultra); //// input to string
    String (valor_ultra).toCharArray(buf_ultra, 5); /// String to array char
    client.publish("esp/ultra", buf_ultra);

    char buf_flow[5]; /// array char
    String valor_flow = String(Flow); //// input to string
    String (valor_flow).toCharArray(buf_flow, 5); /// String to array char
    client.publish("esp/flow", buf_flow);
    sensor_millis = now_millis;
  }

  Show_Status_LCD();
  //Serial.println(stage);

  unsigned long now_millis_2 = millis(); //for check
  if (now_millis - check_millis > 60 * 1000) {
    if (!client.connected()) {
      reconnect();
    }
    check_millis = now_millis_2;
  }

  switch (stage) {

    case 0 : //case 0 auto mode
      if (Ultra > 35 or Ultra < 20) { //Auto
        mcp.digitalWrite(MCP_control_pump, LOW); //Auto
        Serial.println("Auto_pump: Status Pump :LOW");
      } else {
        mcp.digitalWrite(MCP_control_pump, HIGH); //Auto
        Serial.println("Auto_pump: Status Pump :HIGH");
      }
      show_stage = "auto  ";
      break;

    case 1: //case 1 manual control
      if (manual_pump == HIGH) {
        mcp.digitalWrite(MCP_control_pump, HIGH);
        Serial.println("manual_pump: control pump ON");

      } else if (manual_pump == LOW) {
        mcp.digitalWrite(MCP_control_pump, LOW);
        Serial.println("manual_pump: control pump OFF");
      }
      show_stage = "manual";
      break;



    default:
      if (stage >= 3) {
        stage = 0;
      }
  }
  client.loop();
}

//===================================== Function ==========================================//

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

void change_stage() {
  stage++;
}

void change_stage_manual() {
  int reading = digitalRead(switch_manual_pump);

  if ( reading == HIGH && lastState == LOW ) {
    lastState = HIGH;
    manual_pump = !manual_pump;
    //    if (digitalRead(switch_manual_pump) == HIGH) manual_pump = !manual_pump;
  } else if (reading == HIGH && lastState == HIGH ) {
    lastState = LOW;
    manual_pump = !manual_pump;
  }



}


void callback(char* topic, byte * payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);

    //หาก message ที่ส่งกลับมามีค่าเป็น 0 ที่ array index ที่ 10 จะสั่งให้ไฟดับ เช่น {'hello':'0'}
    //หาก message ที่ส่งกลับมาเป็น 1 จะสั่งให้ไฟติด เช่น {'hello':'1'}
    //ตรงนี้แล้วแต่เราจะกำหนดครับ แต่ผมอยากทำให้ง่าย ๆ ก่อนเลยใช้วิธีการ fix ค่าไว้ครับ

    if (i == 10) {
      if ((char)payload[i] == '0') { //turn on/off pump
        //stage = 1;
        manual_pump = LOW;
        //show_stage = "manual";
      } else {
        //stage = 1;
        manual_pump = HIGH;
        //show_stage = "manual";
      }
    }

//    if (i == 9) {
//      if ((char)payload[i] == '0') { //turn on/off manual
//        stage = 1;
//        show_stage = "manual";
//      } else {
//        stage = 0;
//        show_stage = "manual";
//      }
//    }

  }

  Serial.println();
  Serial.println("-----------------------");

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    /*
      YOU MIGHT NEED TO CHANGE THIS LINE, IF YOU'RE HAVING PROBLEMS WITH MQTT MULTIPLE CONNECTIONS
      To change the ESP device ID, you will have to give a new name to the ESP8266.
      Here's how it looks:
       if (client.connect("ESP8266Client")) {
      You can do it like this:
       if (client.connect("ESP1_Office")) {
      Then, for the other ESP:
       if (client.connect("ESP2_Garage")) {
      That should solve your MQTT multiple connections problem
    */
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.subscribe("room/lamp");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
    break;
  }
}



