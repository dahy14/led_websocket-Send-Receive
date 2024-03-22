

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>

// pinout
#define pot1 32
#define pot2 34
#define pot3 35

#define led1 5
#define led2 18
#define led3 19

// channels
int channel1 = 0;
int channel2 = 1;
int channel3 = 2;

//  PWM values
int val1 = 0;
int val2 = 0;
int val3 = 0;

// network credentials
const char *ssid = "Naypes Residence ";
const char *password = "BWCTnaPLDT1234!";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create a WebSocket object
AsyncWebSocket ws("/ws");

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables
unsigned long lastTime = 0;

// Get Sensor Readings and return JSON object
String getSensorReadings()
{
  readings["pot1"] = String(analogRead(pot1));
  readings["pot2"] = String(analogRead(pot2));
  readings["pot3"] = String(analogRead(pot3));
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

// pwm initialization
void initPWM()
{
  int refreshRate = 5000;
  int resolution = 8;
  ledcSetup(channel1, refreshRate, resolution);
  ledcSetup(channel2, refreshRate, resolution);
  ledcSetup(channel3, refreshRate, resolution);
  ledcAttachPin(led1, channel1);
  ledcAttachPin(led2, channel2);
  ledcAttachPin(led3, channel3);
}

// Initialize SPIFFS
void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}

// Initialize WiFi
void initWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print('.');
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void notifyClients(String sensorReadings)
{
  ws.textAll(sensorReadings);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
  AwsFrameInfo *info = (AwsFrameInfo *)arg;

  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
  {
    // data[len] = 0;
    String message = (char *)data;
    //  Check if the message is "getReadings"
    if (strcmp((char *)data, "getReadings") == 0)
    {
      // if it is, send current sensor readings

      // happens once like tf
      String sensorReadings = getSensorReadings();
      notifyClients(sensorReadings);
    }
    else
    { // if not, parse the message and set the PWM values
      JSONVar json = JSON.parse(message);
      String led1Value = json["led1"];
      String led2Value = json["led2"];
      String led3Value = json["led3"];

      val1 = led1Value.toInt();
      val2 = led2Value.toInt();
      val3 = led3Value.toInt();
      val1 = map(val1, 0, 255, 255, 0);
      val2 = map(val2, 0, 255, 255, 0);
      val3 = map(val3, 0, 255, 255, 0);

      Serial.printf("led1: %d, led2: %d, led3: %d\n", val1, val2, val3);
    }
    //}
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
  case WS_EVT_CONNECT:
    Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
    break;
  case WS_EVT_DISCONNECT:
    Serial.printf("WebSocket client #%u disconnected\n", client->id());
    break;
  case WS_EVT_DATA:
    handleWebSocketMessage(arg, data, len);
    break;
  case WS_EVT_PONG:
  case WS_EVT_ERROR:
    break;
  }
}

void initWebSocket()
{
  ws.onEvent(onEvent);
  server.addHandler(&ws);
}

void setup()
{
  Serial.begin(115200);
  pinMode(pot1, INPUT);
  pinMode(pot2, INPUT);
  pinMode(pot3, INPUT);
  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);
  pinMode(led3, OUTPUT);
  initPWM();
  initWiFi();
  initSPIFFS();
  initWebSocket();

  // Web Server Root URL
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/index.html", "text/html"); });

  server.serveStatic("/", SPIFFS, "/");

  // Start server
  server.begin();
}

void loop()
{
  if ((millis() - lastTime) > 100)
  {
    String sensorReadings = getSensorReadings();
    notifyClients(sensorReadings);
    lastTime = millis();
  }
  ledcWrite(channel1, val1);
  ledcWrite(channel2, val2);
  ledcWrite(channel3, val3);
  ws.cleanupClients();
}