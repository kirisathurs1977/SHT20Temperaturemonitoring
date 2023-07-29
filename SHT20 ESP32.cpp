#include <DFRobot_SHT20.h>

/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com
*********/

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include "DFRobot_SHT20.h"
DFRobot_SHT20 sht20;
// Replace the next variables with your SSID/Password combination
const char* ssid = "SenzMate";
const char* password = "fibeR@SenzMate";

//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "mqtt.senzmate.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;


// LED Pin
const int ledPin = 2;
#define TEMPERATURE_SENSOR_ADDRESS 0x40
#define TEMPERATURE_REGISTER 0x39

void setup() {
  Serial.begin(115200);
  Wire.begin();
  sht20.initSHT20(); // Init SHT20 Sensor
  delay(100);
  sht20.checkSHT20(); // Check SHT20 Sensor
  // default settings
  // (you can also pass in a Wire library object like &Wire2)

  setup_wifi();

  Serial.println("point_1");

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  Serial.println("point_2");

  pinMode(ledPin, OUTPUT);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;

  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  // Feel free to add more if statements to control more GPIOs with MQTT

  // If a message is received on the topic esp32/output, you check if the message is either "on" or "off".
  // Changes the output state according to the message
  if (String(topic) == "kri/esp32/output") {
    Serial.print("Changing output to ");
    if (messageTemp == "on") {
      Serial.println("on");
      digitalWrite(ledPin, HIGH);
    }
    else if (messageTemp == "off") {
      Serial.println("off");
      digitalWrite(ledPin, LOW);
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("kri/esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void loop() {
  Wire.beginTransmission(TEMPERATURE_SENSOR_ADDRESS);
  Wire.write(TEMPERATURE_REGISTER);
  Wire.endTransmission();

  // Read temperature
  Wire.requestFrom(TEMPERATURE_SENSOR_ADDRESS, 2);
  byte msb = Wire.read();
  byte lsb = Wire.read();

  // Convert temperature to degrees Celsius
  float temperature = ((msb << 8) | lsb) / 100.0;

  // Print temperature
  Serial.print("T: ");
  Serial.print(temperature);
  Serial.println(" °C");
  float humd = sht20.readHumidity(); // Read Humidity
  float temp = sht20.readTemperature(); // Read Temperature
  Serial.print("Temperature:");
  Serial.print(temp, 1);
  Serial.print("C");
  Serial.print(" Humidity:");
  Serial.print(humd, 1);
  Serial.print("%");
  Serial.println();
  delay(1000);
  char message[50];
  snprintf(message, sizeof(message), "Temperature: %.2f °C T:%.2f °C H:%.2f %", temperature,temp,humd);


  delay(1000); // Wait for 1 second
  Serial.println("point_3");

  if (!client.connected()) {
    reconnect();
  }

  Serial.println("point_4");

  client.loop();

  Serial.println("point_5");

  client.publish("kri/esp32/data", message);
}