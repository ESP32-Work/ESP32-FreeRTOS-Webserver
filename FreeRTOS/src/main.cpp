#include <WiFi.h>
#include <WebServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "SuperMon.h"

#define AP_SSID "TestWebSite"
#define AP_PASS "12345678"

#define PIN_OUTPUT 26
#define PIN_FAN 27
#define PIN_LED 2
#define PIN_A0 34
#define PIN_A1 35

int BitsA0 = 0, BitsA1 = 0;
float VoltsA0 = 0, VoltsA1 = 0;
int FanSpeed = 0;
bool LED0 = false, SomeOutput = false;
uint32_t SensorUpdate = 0;
int FanRPM = 0;

char XML[2048];
char buf[32];
IPAddress Actual_IP;
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;

WebServer server(80);

TaskHandle_t mainTask;

void mainTaskFunction(void *pvParameters);
void UpdateSlider();
void ProcessButton_1();
void ProcessButton_0();
void SendXML();
void SendWebsite();

void setup() {
  Serial.begin(115200);

  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  LED0 = false;
  digitalWrite(PIN_LED, LED0);

  ledcSetup(0, 10000, 8);
  ledcAttachPin(PIN_FAN, 0);
  ledcWrite(0, FanSpeed);

  disableCore0WDT();


  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);
  WiFi.softAPConfig(PageIP, gateway, subnet);
  delay(100);
  Actual_IP = WiFi.softAPIP();
  Serial.print("IP address: ");
  Serial.println(Actual_IP);

  server.on("/", SendWebsite);
  server.on("/xml", SendXML);
  server.on("/UPDATE_SLIDER", UpdateSlider);
  server.on("/BUTTON_0", ProcessButton_0);
  server.on("/BUTTON_1", ProcessButton_1);

  server.begin();

  xTaskCreatePinnedToCore(mainTaskFunction, "mainTask", 8192, NULL, 1, &mainTask, 0);
}

void loop() {
  server.handleClient();
}

void mainTaskFunction(void *pvParameters) {
  while (true) {
    if ((millis() - SensorUpdate) >= 50) {
      SensorUpdate = millis();
      BitsA0 = analogRead(PIN_A0);
      BitsA1 = analogRead(PIN_A1);
      VoltsA0 = BitsA0 * 3.3 / 4096;
      VoltsA1 = BitsA1 * 3.3 / 4096;
    }
    vTaskDelay(pdMS_TO_TICKS(10));  // Adjust the delay as needed
  }
}

void UpdateSlider() {
  String t_state = server.arg("VALUE");
  FanSpeed = t_state.toInt();
  Serial.print("UpdateSlider ");
  Serial.println(FanSpeed);
  ledcWrite(0, FanSpeed);

  FanRPM = map(FanSpeed, 0, 255, 0, 2400);
  strcpy(buf, "");
  sprintf(buf, "%d", FanRPM);
  sprintf(buf, buf);
  server.send(200, "text/plain", buf);
}

void ProcessButton_0() {
  LED0 = !LED0;
  digitalWrite(PIN_LED, LED0);
  Serial.print("Button 0 ");
  Serial.println(LED0);
  server.send(200, "text/plain", "");
}

void ProcessButton_1() {
  SomeOutput = !SomeOutput;
  digitalWrite(PIN_OUTPUT, SomeOutput);
  Serial.print("Button 1 ");
  Serial.println(LED0);
  server.send(200, "text/plain", "");
}

void SendWebsite() {
  Serial.println("sending web page");
  server.send(200, "text/html", PAGE_MAIN);
}

void SendXML() {
  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");
  sprintf(buf, "<B0>%d</B0>\n", BitsA0);
  strcat(XML, buf);
  sprintf(buf, "<V0>%d.%d</V0>\n", (int)(VoltsA0), abs((int)(VoltsA0 * 10) - ((int)(VoltsA0) * 10)));
  strcat(XML, buf);
  sprintf(buf, "<B1>%d</B1>\n", BitsA1);
  strcat(XML, buf);
  sprintf(buf, "<V1>%d.%d</V1>\n", (int)(VoltsA1), abs((int)(VoltsA1 * 10) - ((int)(VoltsA1) * 10)));
  strcat(XML, buf);

  if (LED0) {
    strcat(XML, "<LED>1</LED>\n");
  } else {
    strcat(XML, "<LED>0</LED>\n");
  }

  if (SomeOutput) {
    strcat(XML, "<SWITCH>1</SWITCH>\n");
  } else {
    strcat(XML, "<SWITCH>0</SWITCH>\n");
  }

  strcat(XML, "</Data>\n");
  Serial.println(XML);
  server.send(200, "text/xml", XML);
}



