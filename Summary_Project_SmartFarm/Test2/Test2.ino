//=================================== LIBRARY ================================//

#include <FlowMeter.h>
#include <SPI.h>
#include <SD.h>
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
File myFile;

//===================================== VARIABLE ================================//

float Ultra ; // value ultrasonic
float Flow ; // value flowmeter
const unsigned long period = 1000; //Constant Flowmeter Interrupt
unsigned long sensor_millis; //Count time to take value sensor
unsigned long check_millis; //Count time to take reconnect server
unsigned long sd_millis; //Count time to save value to sd card


long stage = 0; // state button now
String show_stage; //show stage to lcd

boolean lastState; //Last stage pump (manual)
boolean manual_pump = LOW; //Stage pump (manual)

//===================================== MQTT ================================//

const char* ssid = "true_homewifi_99C";
const char* password =  "00000RY2";

const char* mqttServer = "192.168.1.35";
const int mqttPort = 1883;
const char* mqttUser = "s5710122";
const char* mqttPassword = "ap18659993";
WiFiClient espClient;
PubSubClient client(espClient);


//================================== CODE =================================//

void setup() {
  sensor_millis = millis(); //Set time to count for take value
  check_millis = millis();  //Set time to count for reconnect server
  sd_millis = millis(); //Set time to count for take value
  mcp.begin(); // MCP use default address 0
  Serial.begin (115200);
  setup_LCD();
  setup_wifi();
  setup_MQTT();
  check_sd();
  
  pinMode(switch_stage, INPUT); //Call pin in nodemcu input
  pinMode(switch_manual_pump, INPUT);

  mcp.pinMode(0, OUTPUT); //Call pin mcp23017
  mcp.pinMode(MCP_control_pump, OUTPUT);

  attachInterrupt(switch_stage, change_stage, RISING); //interrupt stage
  attachInterrupt(switch_manual_pump, change_stage_manual, RISING); //interrupt pump
  attachInterrupt(2, MeterISR, RISING); //interrupt fan Flowmeter

  Meter.reset();

  client.publish("esp/pump", "Hello from ESP8266"); //Start massgea if esp connect server
  client.subscribe("esp/pump"); //subscribe topic "esp/pump"
  client.subscribe("esp/manual");

}


void loop() {

  // Counttime take value and send (publish) to server
  unsigned long now_millis = millis();
  if (now_millis - sensor_millis > 2000) {
    Read_value_form_sensor();  //Read value from sensor
    char buf_ultra[5]; /// array char
    String valor_ultra = String(Ultra); // input to string ultrasonic
    String (valor_ultra).toCharArray(buf_ultra, 5); // String to array char
    client.publish("esp/ultra", buf_ultra); //Publish ultrasonic
    char buf_flow[5]; /// array char
    String valor_flow = String(Flow); //// input to string Flowmeter
    String (valor_flow).toCharArray(buf_flow, 5); /// String to array char
    client.publish("esp/flow", buf_flow);  //Publish Flowmeter
    sensor_millis = now_millis; //set time default
  }

  unsigned long now_millis_3 = millis(); //Count time
  if (now_millis_3 - sd_millis > 60 * 1000) { //Countdown time 1 minute
    check_sd();
    write_sd();
    read_sd();
    sd_millis = now_millis_3;
  }

  Show_Status_LCD(); //Show value to LCD

  //Counttime to check status server
  unsigned long now_millis_2 = millis(); //Count time
  if (now_millis - check_millis > 60 * 1000) { //Countdown time 1 minute
    if (!client.connected()) { //Check connect
      reconnect();
    }
    check_millis = now_millis_2;
  }

  switch (stage) {
    Switch case

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

    case 2: //case 1 manual control
      mcp.digitalWrite(MCP_control_pump, LOW);
      Serial.println("Stage Off SmartFarm");
      show_stage = "Stage Off SmartFarm";
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

void setup_LCD() {
  lcd.begin(); //LCD
  lcd.setCursor(0, 0);
  lcd.print("SmartFarm");
  lcd.clear();
}

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting to WiFi..");
  }
  Serial.println("Connected to the WiFi network");

}

void setup_MQTT() {
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
}
void callback(String topic, byte* message, unsigned int length) {

  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if (topic == "esp/manual") {
    Serial.print("Changing stage to ");
    if (messageTemp == "0") {
      stage = 0;
      Serial.print("0 : Auto mode");
    }
    else if (messageTemp == "1") {
      stage = 1;
      Serial.print("1 : manual mode");
    }
  }

  if (topic == "esp/pump") {
    Serial.print("Changing pump to ");
    if (messageTemp == "on") {
      manual_pump = HIGH;
      Serial.print("On");
    }
    else if (messageTemp == "off") {
      manual_pump = LOW;
      Serial.print("Off");
    }
  }
  Serial.println();
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
      client.publish("esp/pump", "Hello from ESP8266");
      client.subscribe("esp/pump");
      client.subscribe("esp/manual");
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

void check_sd() {
  Serial.print("Initializing SD card...");

  if (!SD.begin(SS)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
}

void write_sd() {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("database.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to database.txt...");
    myFile.print("Flow water: ");
    myFile.println(Flow);
    myFile.print("water level: ");
    myFile.println(Ultra);
    myFile.println("//======================= done. =====================//");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening database.txt");
  }

}

void read_sd() {
  // re-open the file for reading:
  myFile = SD.open("database.txt");
  if (myFile) {
    Serial.println("database.txt:");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening database.txt");
  }

}

