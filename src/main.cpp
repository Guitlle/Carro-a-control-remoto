#include <Arduino.h>
#include "heltec.h"
#include "ESPAsyncWebServer.h"
#include <WiFi.h>
#include <string>
#include <map>
#include "index_html.h"
#include "settings.h"

using namespace std;

AsyncWebServer server(80);
std::map<string, string> KeyStore;

void shell(String command);
void notFound(AsyncWebServerRequest *request);
int getIntParam(AsyncWebServerRequest *request, String param, int defValue);
void serialOnReceive();
void indexHtml(AsyncWebServerRequest *request);
void listDir(fs::FS &fs, const char * dirname, uint8_t levels);

void setup() {
  Heltec.begin(true /*DisplayEnable Enable*/, false /*LoRa Disable*/, true /*Serial Enable*/);

  Serial.printf("Total heap: %d\r\n", ESP.getHeapSize());
  Serial.printf("Free heap: %d\r\n", ESP.getFreeHeap());
  Serial.printf("Total PSRAM: %d\r\n", ESP.getPsramSize());
  byte* psdRamBuffer = (byte*)ps_malloc(4000000);
  Serial.printf("Free PSRAM: %d\r\n", ESP.getFreePsram());

  Heltec.display->init();
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);

  Serial.println("Ready");
  Serial.onReceive(serialOnReceive);

  // WiFi.softAP("CARRITO-TEST", "cadbafac");
  IPAddress local_IP(192, 168, 0, 129);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("WIFI STA Failed to configure");
  }
  WiFi.begin(SSID, PASSWORD);
  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    attempt += 1;
    delay(500);
    Heltec.display->clear();
    Heltec.display->drawString(0, 0, String("WiFi: Connection attempt " + attempt));
    Heltec.display->display();
  }

  delay(1000);

  Heltec.display->clear();
  Heltec.display->drawString(0, 0, String("WiFi: ") + local_IP.toString());
  //Heltec.display->drawString(0, 12, "Password: cadbafac");
  //Heltec.display->drawString(0, 24, WiFi.softAPIP().toString());
  Heltec.display->display();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/html", INDEX_HTML);
  });
  server.on("/setupPWM", HTTP_GET, [](AsyncWebServerRequest *request){
    int channel, frequency, resolution;
    channel = getIntParam(request, "channel", 0);
    frequency = getIntParam(request, "freq", 0);
    resolution = getIntParam(request, "res", 0);
    ledcSetup(channel, frequency, resolution);
    Serial.printf("\tledcSetup(%d, %d, %d); \n", channel, frequency, resolution);
    request->send(200, "text/plain", "ok");
  });
  server.on("/writePWM", HTTP_GET, [](AsyncWebServerRequest *request){
    int channel, duty;
    channel = getIntParam(request, "channel", 0);
    duty = getIntParam(request, "duty", 0);
    ledcWrite(channel, duty);
    Serial.printf("\tledcWrite(%d, %d); \n", channel, duty);
    request->send(200, "text/plain", "ok");
  });
  server.on("/pinPWM", HTTP_GET, [](AsyncWebServerRequest *request){
    int channel, pin;
    channel = getIntParam(request, "channel", 0);
    pin = getIntParam(request, "pin", 0);
    ledcAttachPin(pin, channel);
    Serial.printf("\tledcAttachPin(%d, %d); \n", pin, channel);
    request->send(200, "text/plain", "ok");
  });
  server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request){
    Serial.printf("LOG: %s \n", request->getParam("message")->value());
    request->send(200, "text/plain", "ok");
  });
  server.on("/keyStore/set", HTTP_GET, [](AsyncWebServerRequest *request){
    String key = request->getParam("key")->value(), value = request->getParam("value")->value();
    Serial.printf("STORE %s => %s \n", key, value);
    KeyStore[key.c_str()] = value.c_str();
    request->send(200, "text/plain", "ok");
  });
  server.on("/keyStore/get", HTTP_GET, [](AsyncWebServerRequest *request){
    String key = request->getParam("key")->value();
    string value = KeyStore[key.c_str()];
    Serial.printf("GET %s => %s \n", key, value);
    request->send(200, "text/plain", value.data());
  });
  server.on("/shell", HTTP_GET, [](AsyncWebServerRequest *request){
    String command = request->getParam("command")->value();
    Serial.printf("SHELL (%s) \n", command);
    shell(command);
    request->send(200, "text/plain", "ok");
  });
  server.onNotFound(notFound);
  server.begin();
}

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
}
int getIntParam(AsyncWebServerRequest *request, String param, int defValue) {
  if (request->hasParam(param)) {
    return request->getParam(param)->value().toInt();
  }
  else {
    return defValue;
  }
}
String serialBuffer = String("");
void serialOnReceive() {
  int reading;
  while(true) {
    reading = Serial.read();
    if (reading == -1) return;
    Serial.write((char)reading);

    if (reading == '\n') {
      shell(serialBuffer);
      serialBuffer.clear();
    } else {
      serialBuffer += (char) reading;
    }
  }
}

int shellGetInt(String command, int curpos) {
  int endpos, value;
  String param;
  endpos = command.indexOf(' ', curpos);
  param = command.substring(curpos, endpos);
  value = param.toInt();

  return value;
}
int* shellGetInts(String command, int n) {
  int* ints = new int[n];
  int pos = 0;
  for (int i=0; i<n; i++) {
    pos = command.indexOf(' ', pos) + 1;
    ints[i] = shellGetInt(command, pos);
  }
  return ints;
}
String* shellGetStrs(String command, int n) {
  String* strs = new String[n];
  int pos = 0, pos2;
  for (int i=0; i<n; i++) {
    pos = command.indexOf(' ', pos) + 1;
    pos2 = command.indexOf(' ', pos) + 1;
    strs[i] = command.substring(pos, pos2);
  }
  return strs;
}
void shell(String _command) {
  String command = _command + " ";
  int strpos = 0, strnextpos = 0, size;
  size = command.length();
  strnextpos = command.indexOf(' ', 0);
  if(command.startsWith("getKeyValue ")) {
    String *params = shellGetStrs(command, 1);
    Serial.printf("\t %s => %s \n", params[0], KeyStore[params[0].c_str()].data());
  } else if(command.startsWith("setKeyValue ")) {
    String *params = shellGetStrs(command, 2);
    KeyStore[params[0].c_str()] = params[1].c_str();
    Serial.printf("\t %s => %s \n", params[0], KeyStore[params[0].c_str()].data());
  } else if(command.startsWith("ledcSetup ")) {
    int * params = shellGetInts(command, 3);
    if (params[1] <= 0 || params[1] <= 0) return;
    ledcSetup(params[0], (double) params[1], params[2]);
    Serial.printf("\tledcSetup(%d, %d, %d); \n", params[0], params[1], params[2]);
  } else if(command.startsWith("ledcWrite ")) {
    int * params = shellGetInts(command, 2);
    if (params[0] <= 0) return;
    ledcWrite(params[0], params[1]);
    Serial.printf("\tledcWrite(%d, %d); \n", params[0], params[1]);
  } else if(command.startsWith("ledcPin ")) {
    int * params = shellGetInts(command, 2);
    if (params[1] <= 0) return;
    ledcAttachPin(params[0], params[1]);
    Serial.printf("\tledcAttachPin(%d, %d); \n", params[0], params[1]);
  } else if(command.startsWith("status ")) {
    Serial.printf("Total heap: %d\r\n", ESP.getHeapSize());
    Serial.printf("Free heap: %d\r\n", ESP.getFreeHeap());
    Serial.printf("Total PSRAM: %d\r\n", ESP.getPsramSize());
    Serial.printf("Free PSRAM: %d\r\n", ESP.getFreePsram());
  } else if(command.startsWith("led ")) {
    ledcAttachPin(25, 1);
    ledcSetup(1, 5000, 8);
    for(uint8_t i=0; i < 10; i++) {
      ledcWrite(1, (10*i) % 200);
      delay(10);
    }
    ledcWrite(1, 0);
  } else {
    Serial.println("Unknown command " + command);
  }
}

void loop() { }
