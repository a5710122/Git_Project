#include <ESP8266WiFi.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
const char* ssid = "Maka";
const char* password = "";

// Config MQTT Server
#define mqtt_server "172.19.201.179"
#define mqtt_port 1883
#define mqtt_user "mymqtt"
#define mqtt_password "myraspi"

#define LED_PIN D1

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  pinMode(LED_PIN, OUTPUT);
  
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.subscribe("mynew/test");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
      return;
    }
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  String msg = "";
  int i=0;
  while (i<length) msg += (char)payload[i++];
  if (msg == "GET") {
    client.publish("mynew/test", (digitalRead(LED_PIN) ? "LEDON" : "LEDOFF"));
    Serial.println("Send !");
    return;
  }
  digitalWrite(LED_PIN, (msg == "LEDON" ? HIGH : LOW));
  Serial.println(msg);
}
