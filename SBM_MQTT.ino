#include <WiFi.h>
#include <PubSubClient.h>
#include <dummy.h>
#include "DHT.h"
#include "WiFi.h"
#include <Adafruit_Sensor.h>

//definisi wifi
const char* ssid = "E-49 BARU";
const char* password = "pogungbaru";
//digunakan local host laptop dan digunakan untuk nanti membuka Dashboard di Handphone
const char* mqtt_server = "192.168.0.110";

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define TRIGGER_PIN 5
#define ECHO_PIN 18

const int buttonPinLeft = 21; // increment button
const int buttonPinFront = 22; // decrement button
const int buttonPinRight = 23; // decrement button

volatile int tiltValue = 0;
int buttonStateLeft = 0;
int buttonStateFront = 0;
int buttonStateRight = 0;
int lastButtonStateLeft = 0;
int lastButtonStateFront = 0;
int lastButtonStateRight = 0;
unsigned long lastDebounceTimeLeft = 0;
unsigned long lastDebounceTimeFront = 0;
unsigned long lastDebounceTimeRight = 0;
const int debounceDelay = 50;


const int potPin = 35;
int potValue = 0;
float h = 0, t = 0, f = 0;
float voltage = 0;
DHT dht(DHTPIN, DHTTYPE);

const unsigned long delayInterval = 1000;  // Blink interval in milliseconds

unsigned long previousMillis = 0;

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
#define Tilt_TOPIC              "ESP32/Tilt"
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

  // initialize buttons and interrupt pin as inputs
  pinMode(buttonPinLeft, INPUT_PULLUP);
  pinMode(buttonPinFront, INPUT_PULLUP);
  pinMode(buttonPinRight, INPUT_PULLUP);

}

void loop() {
  // put your main code here, to run repeatedly:

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= delayInterval) {
    previousMillis = currentMillis;  // Update the previous timestamp

  if (!client.connected()) {
    mqttconnect();
  }
  client.loop();

  //================================================================
  // DHT22 TEMPERATURE AND HUMIDITY SENSOR
  //================================================================

  h = dht.readHumidity();
  t = dht.readTemperature();
  f = dht.readTemperature(true);

  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  //================================================================
  // POTENSIOMETER
  //================================================================


  // Read from Potensiometer
  potValue = analogRead(potPin);

  //================================================================
  // ULTRASONIC SENSOR
  //================================================================

  // Trigger the ultrasonic sensor
  digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);

  // Measure the echo pulse duration
  long duration = pulseIn(ECHO_PIN, HIGH);

  // Convert the pulse duration to distance
  float distance = duration * 0.034 / 2;

  //================================================================
  // BUTTON LEFT
  //================================================================

    int readingLeft = digitalRead(buttonPinLeft);

    // check if increment button state has changed
    if (readingLeft != lastButtonStateLeft) {
      lastDebounceTimeLeft = millis();
    }

    // check if debounce delay has passed
    if ((millis() - lastDebounceTimeLeft) >= debounceDelay) {
      // update button state only if it has been stable for the debounce duration
      buttonStateLeft = readingLeft;

      // do something when increment button is pressed
      if (buttonStateLeft == LOW) {
        tiltValue -= 15;
      }
    }

  // update last increment button state
  lastButtonStateLeft = readingLeft;

  //================================================================
  // BUTTON FRONT
  //================================================================

  int readingFront = digitalRead(buttonPinFront);

  // check if increment button state has changed
  if (readingFront != lastButtonStateFront) {
    lastDebounceTimeFront = millis();
  }

  // check if debounce delay has passed
  if ((millis() - lastDebounceTimeFront) >= debounceDelay) {
    // update button state only if it has been stable for the debounce duration
    buttonStateFront = readingFront;

    // do something when increment button is pressed
    if (buttonStateFront == LOW) {
      tiltValue = 0;
    }
  }

  // update last increment button state
  lastButtonStateFront = readingFront;

  //================================================================
  // BUTTON RIGHT
  //================================================================

  int readingRight = digitalRead(buttonPinRight);

  // check if increment button state has changed
  if (readingRight != lastButtonStateRight) {
    lastDebounceTimeRight = millis();
  }

  // check if debounce delay has passed
  if ((millis() - lastDebounceTimeRight) >= debounceDelay) {
    // update button state only if it has been stable for the debounce duration
    buttonStateRight = readingRight;

    // do something when increment button is pressed
    if (buttonStateRight == LOW) {
      tiltValue += 15;
    }
  }

  // update last increment button state
  lastButtonStateRight = readingRight;


  //================================================================
  // PRINT AT SERIAL AND SEND TO MQTT CLIENT
  //================================================================

  snprintf(msg, 5, "%f", t);
  Serial.print("t: ");
  Serial.println(t);
  client.publish(DHTTemperature_TOPIC, msg);

  snprintf(msg, 5, "%f", h);
    Serial.print("h: ");
  Serial.println(h);
  client.publish(DHTHumidity_TOPIC, msg);

  snprintf(msg, 5, "%d", potValue);
  Serial.print("pot: ");
  Serial.println(potValue);
  client.publish(Potensiometer_TOPIC, msg);

  snprintf(msg, 5, "%f", distance);
  Serial.print("dist: ");
  Serial.println(distance);
  client.publish(Ultrasonic_TOPIC, msg);

  snprintf(msg, 5, "%d", tiltValue);
  Serial.print("tilt Value: ");
  Serial.println(tiltValue);
  client.publish(Tilt_TOPIC, msg);

  }




}
