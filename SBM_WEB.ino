#include "DHT.h"
#include "WiFi.h"
#include <ESPAsyncWebServer.h>
#include <Adafruit_Sensor.h>

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
#define TRIGGER_PIN 12
#define ECHO_PIN 14

unsigned long previousMillis = 0;
const unsigned long interval = 1000;

const char* ssid = "testesp32coy";
const char* password = "testuser123";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const int potPin = 35;
int potValue = 0;
float voltage = 0;
float sudut = 0.0;
DHT dht(DHTPIN, DHTTYPE);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>WebSocket Sensor Data</title>
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="stylesheet" type="text/css" href="https://cdn.jsdelivr.net/gh/dundorma/SBM-FINALPROJECT-FRONTEND/style.css">
  <script src="http://cdn.rawgit.com/Mikhus/canvas-gauges/gh-pages/download/2.1.7/all/gauge.min.js"></script>
</head>
<body>
  <div class="topnav">
    <h1>ESP WEB SERVER GAUGES</h1>
  </div>
  <h1>Sensor Data</h1>
  <div class="content">
    <div class="card-grid">
      <div class="card">
        <p class="card-title">Temperature</p>
        <canvas id="temperature"></canvas>
      </div>
      <div class="card">
        <p class="card-title">Humidity</p>
        <canvas id="humidity"></canvas>
      </div>
      <div class="card">
        <p class="card-title">Jarak</p>
        <canvas id="potensiometer"></canvas>
      </div>
    </div>
  </div>
  <div class="container">
    <div class="card-grid">
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

  <script defer>
    const websocket = new WebSocket("ws://192.168.43.190/ws");

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
    units: "Meter (m)",
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

  websocket.addEventListener("message", (event) => {
    gaugeTemp.value = JSON.parse(event.data).temperature;
    gaugePotensio.value = JSON.parse(event.data).potensiometer / 40.95;
    gaugeHum.value = JSON.parse(event.data).humidity;
    
    const distance = document.querySelector("#distance");
    const gyro = document.querySelector("#gyro");
    distance.innerHTML = JSON.parse(event.data).distance;
    // gyro.innerHTML = JSON.parse(event.data).gyro;
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

void setup() {
  dht.begin();
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html);
  });

  DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
  server.begin();
  initWebSocket();

  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

float h = 0, t = 0;
float hbuff = h;
float tbuff = t;

void loop() {
  ws.cleanupClients();

  potValue = analogRead(potPin);
  int sensorValue = analogRead(34);
  voltage = sensorValue * (3.3 / 4095.0);

  h = dht.readHumidity();
  t = dht.readTemperature();

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

  if(!isnan(t)) {
    tbuff = t;
  }

  if(!isnan(h)) {
    hbuff = h;
  }

  String webSocketOutput = "";
  webSocketOutput += "{";
  webSocketOutput += "\"temperature\" : ";
  webSocketOutput += tbuff;
  webSocketOutput += ",";
  webSocketOutput += "\"humidity\" : ";
  webSocketOutput += hbuff;
  webSocketOutput += ",";
  webSocketOutput += "\"potensiometer\" : ";
  webSocketOutput += potValue;
  webSocketOutput += ",";
  webSocketOutput += "\"distance\" : ";
  webSocketOutput += distance;
  // TODO: ADD PUSH BUTTON TILT IMPLEMENTATION
  // webSocketOutput += ",";
  // webSocketOutput += "\"gyro\" : ";
  // webSocketOutput += gyro;
  webSocketOutput += "}";

  ws.textAll(webSocketOutput);
  Serial.println(webSocketOutput);
  delay(1000);
}
