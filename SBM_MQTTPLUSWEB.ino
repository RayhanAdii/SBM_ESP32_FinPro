#include <WiFi.h>
#include <PubSubClient.h>
#include <dummy.h>
#include "DHT.h"
#include "WiFi.h"
#include <Adafruit_Sensor.h>
#include <ESPAsyncWebServer.h>

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

/// WEB PART
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>WebSocket Sensor Data</title>
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="stylesheet" type="text/css" href="https://cdn.jsdelivr.net/gh/dundorma/SBM-FINALPROJECT-FRONTEND/style.css">
  <script src="http://cdn.rawgit.com/Mikhus/canvas-gauges/gh-pages/download/2.1.7/all/gauge.min.js"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/Chart.js/2.9.4/Chart.js"></script>
</head>
<body>
  <div class="topnav">
    <h1>ESP WEB SERVER GAUGES</h1>
  </div>
  <h2>Sensor Data</h2>
  
    <div class="main-container">
      <div class="content-sensor">
        <div class="card-content-sensor">
          <div class="card">
            <p class="card-title">Suhu</p>
            <canvas id="temperature"></canvas>
          </div>
          <div class="card">
            <p class="card-title">Kelembapan</p>
            <canvas id="humidity"></canvas>
          </div>
          <div class="card">
            <p class="card-title">Jarak</p>
            <canvas id="potensiometer"></canvas>
          </div>
        </div>
      </div>
      <div class="content-distance">
        <div class="card-content-distance">
          <div class="card">
            <p class="card-title">Jarak dari Benda di Depan</p>
            <p id="distance"></p>
          </div>
          <div class="card">
            <p class="card-title">Sudut Gyro</p>
            <p id="gyro"></p>
          </div>
        </div>
      </div>
      <div class="content-velocity">
        <div class="card-content-velocity">
          <div class="card card-velocity">
            <p class="card-title">Chart Velocity</p>
            <canvas id="chart-velocity"></canvas>
          </div>
          <div class="card">
            <p class="card-title">Jarak yang Ditempuh</p>
            <p id="jarak-ditempuh"></p>
          </div>
        </div>
      </div>
      <footer>
        Made by: Rayhan Adi W., Moh. Rizky Arif, Daffa Kamal, Laily Medha Nur
        I., and Salma Asma N.
      </footer>
    </div>

  <script defer>
    const websocket = new WebSocket("ws://192.168.0.100/ws");

    var gaugeTemp = new LinearGauge({
    renderTo: 'temperature',
    width: 120,
    height: 400,
    units: "Temperature C",
    minValue: 0,
    startAngle: 90,
    ticksAngle: 180,
    maxValue: 40,
    colorValueBoxRect: "#049faa",
    colorValueBoxRectEnd: "#049faa",
    colorValueBoxBackground: "#f1fbfc",
    valueDec: 2,
    valueInt: 2,
    majorTicks: [
        "0",
        "5",
        "10",
        "15",
        "20",
        "25",
        "30",
        "35",
        "40"
    ],
    minorTicks: 4,
    strokeTicks: true,
    highlights: [
        {
            "from": 30,
            "to": 40,
            "color": "rgba(200, 50, 50, .75)"
        }
    ],
    colorPlate: "#fff",
    colorBarProgress: "#CC2936",
    colorBarProgressEnd: "#049faa",
    borderShadowWidth: 0,
    borders: false,
    needleType: "arrow",
    needleWidth: 2,
    needleCircleSize: 7,
    needleCircleOuter: true,
    needleCircleInner: false,
    animationDuration: 1500,
    animationRule: "linear",
    barWidth: 10,
  }).draw();

  // Create Humidity Gauge
  var gaugeHum = new RadialGauge({
    renderTo: 'humidity',
    width: 300,
    height: 300,
    units: "Humidity (%)",
    minValue: 0,
    maxValue: 100,
    colorValueBoxRect: "#049faa",
    colorValueBoxRectEnd: "#049faa",
    colorValueBoxBackground: "#f1fbfc",
    valueInt: 2,
    majorTicks: [
        "0",
        "20",
        "40",
        "60",
        "80",
        "100"

    ],
    minorTicks: 4,
    strokeTicks: true,
    highlights: [
        {
            "from": 80,
            "to": 100,
            "color": "#03C0C1"
        }
    ],
    colorPlate: "#fff",
    borderShadowWidth: 0,
    borders: false,
    needleType: "line",
    colorNeedle: "#007F80",
    colorNeedleEnd: "#007F80",
    needleWidth: 2,
    needleCircleSize: 3,
    colorNeedleCircleOuter: "#007F80",
    needleCircleOuter: true,
    needleCircleInner: false,
    animationDuration: 1500,
    animationRule: "linear"
  }).draw();

  // Create Humidity Gauge
  var gaugePotensio = new RadialGauge({
    renderTo: 'potensiometer',
    width: 300,
    height: 300,
    units: "Meter / second (m/s)",
    minValue: 0,
    maxValue: 100,
    colorValueBoxRect: "#049faa",
    colorValueBoxRectEnd: "#049faa",
    colorValueBoxBackground: "#f1fbfc",
    valueInt: 2,
    majorTicks: [
        "0",
        "20",
        "40",
        "60",
        "80",
        "100"

    ],
    minorTicks: 4,
    strokeTicks: true,
    highlights: [
        {
            "from": 80,
            "to": 100,
            "color": "#03C0C1"
        }
    ],
    colorPlate: "#fff",
    borderShadowWidth: 0,
    borders: false,
    needleType: "line",
    colorNeedle: "#007F80",
    colorNeedleEnd: "#007F80",
    needleWidth: 2,
    needleCircleSize: 3,
    colorNeedleCircleOuter: "#007F80",
    needleCircleOuter: true,
    needleCircleInner: false,
    animationDuration: 500,
    animationRule: "linear"
  }).draw();

  let xValues = [0];
  let yValues = [0];
  let jarakDitempuh = 0;
  let timer_buff = 1;

  websocket.addEventListener("message", (event) => {
    gaugeTemp.value = JSON.parse(event.data).temperature;
    gaugePotensio.value = JSON.parse(event.data).potensiometer / 40.95;
    gaugeHum.value = JSON.parse(event.data).humidity;

    jarakDitempuh += JSON.parse(event.data).potensiometer / 40.95;
    
    const distance = document.querySelector("#distance");
    const gyro = document.querySelector("#gyro");
    const jarakDitempuhHTML = document.querySelector("#jarak-ditempuh");
    distance.innerHTML = JSON.parse(event.data).distance;
    jarakDitempuhHTML.innerHTML = jarakDitempuh;
    gyro.innerHTML = JSON.parse(event.data).gyro;


    /// Velocity Chart
    xValues.push(timer_buff);
    yValues.push(JSON.parse(event.data).potensiometer / 40.95);
    timer_buff +=1;
    new Chart("chart-velocity", {
      type: "line",
      data: {
        labels: xValues.slice(-30),
        datasets: [{
          data: yValues.slice(-30),
          borderColor: "red",
          fill: false
        }]
      },
      options: {
        legend: {display: false},
        animation: false
      }
    });
    
  });
  </script>
</body>
</html>
)rawliteral";

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
  void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      break;
  }
}

void initWebSocket() {
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}
///

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

  /// WEB SETUP PART
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
  initWebSocket();
  /// ***************

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

    /// WEB LOOP PART
    ws.cleanupClients();
    String webSocketOutput = "";
    webSocketOutput += "{";
    webSocketOutput += "\"temperature\" : ";
    webSocketOutput += t;
    webSocketOutput += ",";
    webSocketOutput += "\"humidity\" : ";
    webSocketOutput += h;
    webSocketOutput += ",";
    webSocketOutput += "\"potensiometer\" : ";
    webSocketOutput += potValue;
    webSocketOutput += ",";
    webSocketOutput += "\"distance\" : ";
    webSocketOutput += distance;
    // TODO: ADD PUSH BUTTON TILT IMPLEMENTATION
    webSocketOutput += ",";
    webSocketOutput += "\"gyro\" : ";
    webSocketOutput += tiltValue;
    webSocketOutput += "}";

    ws.textAll(webSocketOutput);
    Serial.println(webSocketOutput);
    ///
  }




}
