#include <ESP8266WiFi.h>
#include <PubSubClient.h>
const char* ssid = "silver";
const char* password =  "025405730";

const char* mqttServer = "192.168.1.43";
const int mqttPort = 1883;
const char* mqttUser = "s5710122";
const char* mqttPassword = "ap18659993";

WiFiClient espClient;
PubSubClient client(espClient);

//Pin ที่ต่อไฟ LED
const int led = 16;

void setup() {
  Serial.begin(115200);
  pinMode(led, OUTPUT);

  //สั่งให้ไฟดับก่อน
  digitalWrite(led, LOW);

  //เชื่อมต่อ wifi
  WiFi.begin(ssid, password);


  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

  //connect ไปที่ mqtt broker
  client.setServer(mqttServer, mqttPort);

  //กำหนด function เมื่อมีการส่งข้อมูลมาจาก MQTT
  client.setCallback(callback);


  while (!client.connected()) { //วนลูปจนกว่าจะต่อสำเร็จ
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
  client.publish("esp/test", "Hello from ESP8266");

  //subscribe topic "esp/test"
  client.subscribe("esp/test");

}

//เมื่อมีข้อมูลกลับมาจาก MQTT จะโดนส่งกลับมาที่ Method นี้
void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);

    //หาก message ที่ส่งกลับมามีค่าเป็น 0 ที่ array index ที่ 10 จะสั่งให้ไฟดับ เช่น {'hello':'0'}
    //หาก message ที่ส่งกลับมาเป็น 1 จะสั่งให้ไฟติด เช่น {'hello':'1'}
    //ตรงนี้แล้วแต่เราจะกำหนดครับ แต่ผมอยากทำให้ง่าย ๆ ก่อนเลยใช้วิธีการ fix ค่าไว้ครับ
    if (i == 10) {
      if ((char)payload[i] == '0') { //turn off light
        digitalWrite(led, LOW);
      } else {
        digitalWrite(led, HIGH);
      }
    }
  }



  Serial.println();
  Serial.println("-----------------------");

}

void loop() {
  client.loop();

}
