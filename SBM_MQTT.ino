#include <WiFi.h>
#include <PubSubClient.h>
#include <dummy.h>
#include "DHT.h"
#include "WiFi.h"
#include <Adafruit_Sensor.h>

// Masukkan nama wifi dan password
const char* ssid = "bukanwarmindo";
const char* password = "orakarik";
// Masukkan ip address MQTT broker
const char* mqtt_server = "192.168.181.232";

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define TRIGGER_PIN 12
#define ECHO_PIN 14

unsigned long previousMillis = 0;
const unsigned long interval = 1000; // Interval between distance measurements (in milliseconds)

const int potPin = 35;
int potValue = 0;
float h = 0, t = 0, f = 0;
float voltage = 0;
DHT dht(DHTPIN, DHTTYPE);

String readDHTTemperature() {
  return String(t);
}

String readDHTHumidity() {
  return String(h);
}

String readLightSensor() {
  return String(voltage);
}

String readPotensiometer() {
  return String(potValue);
}


WiFiClient espClient;
PubSubClient client(espClient);

#define DHTTemperature_TOPIC    "ESP32/DHTTemperature"
#define DHTHumidity_TOPIC       "ESP32/DHTHumidity"
#define LightSensor_TOPIC       "ESP32/LightSensor"
#define Potensiometer_TOPIC     "ESP32/Potensiometer"
#define Ultrasonic_TOPIC        "ESP32/Ultrasonic"

long lastMsg = 0;
char msg[5];


void mqttconnect() {
  /* Loop until reconnected */
  while (!client.connected()) {
    Serial.print("MQTT connecting ...");
    String clientId = "ESP32Client";
    if (client.connect(clientId.c_str())) {
      Serial.println(" Connected");
    } else {
      Serial.print("failed, status code =");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}






void setup() {
  Serial.begin(115200);
  dht.begin();

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
  client.setServer(mqtt_server, 1883);

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()) {
    mqttconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();

  //if (currentMillis - previousMillis >= interval) {
   // previousMillis = currentMillis;
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  h = dht.readHumidity();
  t = dht.readTemperature();
  f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Read from Potensiometer
  potValue = analogRead(potPin);

  // Read from Photoresistor
  int sensorValue = analogRead(34);
  voltage = sensorValue * (5.0 / 1024.0);

    // Trigger the ultrasonic sensor
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(5);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(20);
  digitalWrite(TRIGGER_PIN, LOW);

  // Measure the echo pulse duration
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Convert the pulse duration to distance
  float distance = duration * 0.034 / 2;

  snprintf(msg, 5, "%f", t);
  Serial.print("t: ");
  Serial.println(t);
  client.publish(DHTTemperature_TOPIC, msg);

  snprintf(msg, 5, "%f", h);
    Serial.print("h: ");
  Serial.println(h);
  client.publish(DHTHumidity_TOPIC, msg);

  snprintf(msg, 5, "%f", voltage);
  Serial.print("light: ");
  Serial.println(voltage);
  client.publish(LightSensor_TOPIC, msg);

  snprintf(msg, 5, "%d", potValue);
  Serial.print("pot: ");
  Serial.println(potValue);
  client.publish(Potensiometer_TOPIC, msg);

  snprintf(msg, 5, "%f", distance);
  Serial.print("dist: ");
  Serial.println(distance);
  client.publish(Ultrasonic_TOPIC, msg);

  delay(1000);

 // }










}
