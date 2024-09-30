#include <WiFi.h>  // Correct WiFi library for ESP32
#include <WebSocketsClient.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

// Replace with your network credentials
const char* ssid = "Aimer";
const char* password = "09271290354";

// WebSocket server settings
const char* websocket_host = "192.168.58.82";
uint16_t websocket_port = 8080;
const char* websocket_url = "/";

// DHT22 Sensor Setup
#define DHTPIN 4
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// WebSocket client object
WebSocketsClient webSocket;

const char* deviceId = "ESP32_Location1";
unsigned long previousMillis = 0;
const long interval = 5000;  // Send data every 5 seconds

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("Disconnected from WebSocket server.");
      break;
    case WStype_CONNECTED:
      Serial.println("Connected to WebSocket server.");
      break;
    case WStype_TEXT:
      Serial.printf("Message from server: %s\n", payload);
      break;
    case WStype_ERROR:
      Serial.println("WebSocket error occurred.");
      break;
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected to WiFi");

  // Setup WebSocket
  webSocket.begin(websocket_host, websocket_port, websocket_url);
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    float t = dht.readTemperature();
    float h = dht.readHumidity();
    float hi = dht.computeHeatIndex(t, h, false);

    if (!isnan(t) && !isnan(h) && !isnan(hi)) {
      String jsonData = "{\"deviceId\":\"" + String(deviceId) + "\",\"temperature\":" + String(t) +
                        ",\"humidity\":" + String(h) + ",\"heat_index\":" + String(hi) + "}";

      webSocket.sendTXT(jsonData);
      Serial.println("Sent data: " + jsonData);
    } else {
      Serial.println("Failed to read from DHT sensor.");
    }
  }
}
