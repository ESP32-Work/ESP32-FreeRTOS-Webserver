#include <WiFi.h>
#include <WebServer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "DHT.h"
#include <Adafruit_Sensor.h>

#include "SuperMon.h"

#define AP_SSID "TestWebSite"
#define AP_PASS "12345678"

#define PIN_OUTPUT 26
#define PIN_FAN 27
#define PIN_LED 2
#define PIN_A0 34
#define PIN_A1 35
#define IR_SENSOR_PIN 33  
#define ULTRASONIC_TRIGGER_PIN 12  
#define ULTRASONIC_ECHO_PIN 13     
#define BUZZER_PIN 15              
#define DHT_PIN 14   
#define DHT_TYPE DHT11

DHT dht(DHT_PIN, DHT_TYPE);

int BitsA0 = 0, BitsA1 = 0;
float VoltsA0 = 0, VoltsA1 = 0;
int FanSpeed = 0;
bool LED0 = false, SomeOutput = false;
uint32_t SensorUpdate = 0;
int FanRPM = 0;
bool emergencyShutdownActive = false;
int taskCount = 0;
float temperatureDHT = 0.0;
float humidityDHT = 0.0;

char XML[2048];
char buf[32];
IPAddress Actual_IP;
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;

WebServer server(80);

TaskHandle_t mainTask;
TaskHandle_t serverTask;
TaskHandle_t Task1Handle = NULL;
TaskHandle_t Task2Handle = NULL;
TaskHandle_t Task3Handle = NULL;

void IR_Sensor_Function(void *pvParameters);
void DHT_Sensor_Function(void *pvParameters);
void Emergency_Function(void *pvParameters);
void ServerTask(void *pvParameters);  
void EmergencyShutdown();

void ADC_Function(void *pvParameters);

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
  dht.begin();
  xTaskCreatePinnedToCore(ADC_Function, "mainTask", 8192, NULL, 1, &mainTask, 0);

  // Create FreeRTOS tasks
  xTaskCreatePinnedToCore(IR_Sensor_Function, "Task1", 8192, NULL, 2, &Task1Handle, 0);
  xTaskCreatePinnedToCore(DHT_Sensor_Function, "Task2", 8192, NULL, 2, &Task2Handle, 0);
  xTaskCreatePinnedToCore(Emergency_Function, "Task3", 8192, NULL, 3, &Task3Handle, 0);

  // Create a server task pinned to core 1
  xTaskCreatePinnedToCore(ServerTask, "serverTask", 4096, NULL, 3, &serverTask, 1);
}

void loop() {
  // Since the server is now in its task, loop() can be left empty
  // or used for other non-blocking tasks if needed.
}

void ADC_Function(void *pvParameters) {
  taskCount++;
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

  // Map FanSpeed to LED brightness
  int ledBrightness = map(FanSpeed, 0, 255, 0, 255);

  // Set LED brightness 
  analogWrite(PIN_FAN, ledBrightness);

  // Respond with the updated RPM value
  server.send(200, "text/plain", String(FanSpeed));
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

  strcat(XML, "<EMERGENCY_MODE>");
  if (emergencyShutdownActive) {
    strcat(XML, "1");
  } else {
    strcat(XML, "0");
  }
  strcat(XML, "</EMERGENCY_MODE>\n");
  strcat(XML, "<CORE0_STATUS>");
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING && uxTaskGetNumberOfTasks() > 0) {
    strcat(XML, "1");
  } else {
    strcat(XML, "0");
  }
  strcat(XML, "</CORE0_STATUS>\n");

  strcat(XML, "<CORE1_STATUS>");
  if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING && uxTaskGetNumberOfTasks() > 1) {
    // char a= Serial.read();
    // if(a == 'C'){
    //   strcat(XML, "0");
    // }
    strcat(XML, "1");
  } else {
    strcat(XML, "0");
  }
  strcat(XML, "</CORE1_STATUS>\n");
  // Append temperature and humidity data to XML
  strcat(XML, "<DHT_READINGS>\n");

  // Append temperature value
  strcat(XML, "<TEMP>");
  sprintf(buf, "%.2f", temperatureDHT);
  strcat(XML, buf);
  strcat(XML, "</TEMP>\n");

  // Append humidity value
  strcat(XML, "<HUMIDITY>");
  sprintf(buf, "%.2f", humidityDHT);
  strcat(XML, buf);
  strcat(XML, "</HUMIDITY>\n");

  strcat(XML, "</DHT_READINGS>\n");
  strcat(XML, "<TASK_COUNT>");
  strcat(XML, String(taskCount).c_str());
  strcat(XML, "</TASK_COUNT>\n");

  strcat(XML, "</Data>\n");
  Serial.println(XML);
  server.send(200, "text/xml", XML);
}

void ServerTask(void *pvParameters) {
  taskCount++;
  while (true) {
    // Update the server task core indicators
    server.handleClient();
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}


void IR_Sensor_Function(void *pvParameters) {
  taskCount++;
  pinMode(IR_SENSOR_PIN, INPUT);

  while (1) {
    // Read data from the analog IR sensor
    int irSensorValue = analogRead(IR_SENSOR_PIN);

    // Process the sensor data
    if (irSensorValue < 500) {
      Serial.println("IR sensor detected an obstacle!");
    } else {
      Serial.println("No obstacle detected.");
    }
    vTaskDelay(pdMS_TO_TICKS(50));  // Adjust the delay as needed
  }
  
}

void DHT_Sensor_Function(void *pvParameters) {
  taskCount++;
  while (1) {
    temperatureDHT = dht.readTemperature();
    humidityDHT = dht.readHumidity();

    Serial.print("Task2 - Temperature: ");
    Serial.print(temperatureDHT);
    Serial.print(" °C, Humidity: ");
    Serial.print(humidityDHT);
    Serial.println(" %");
    // Update the server with temperature and humidity values
    SendXML(); 
    Serial.println("Task2 - display is updated");
    vTaskDelay(pdMS_TO_TICKS(100));  // Adjust the delay as needed
  }
  
}

void Emergency_Function(void *pvParameters) {
  taskCount++;
  pinMode(ULTRASONIC_TRIGGER_PIN, OUTPUT);
  pinMode(ULTRASONIC_ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  while (1) {
    // Trigger ultrasonic sensor
    digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(ULTRASONIC_TRIGGER_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(ULTRASONIC_TRIGGER_PIN, LOW);

    // Read the ultrasonic sensor echo
    long duration = pulseIn(ULTRASONIC_ECHO_PIN, HIGH);
    // Calculate distance in centimeters
    float distance = duration * 0.034 / 2;
    Serial.print("Distance: ");
    Serial.println(distance);
    char a= Serial.read();
    // Check if an obstacle is detected (adjust the distance threshold as needed)
    if (distance < 20) {
    // if (a == 'E') {
      // Activate emergency mode only if not already active
      if (!emergencyShutdownActive) {
        emergencyShutdownActive = true;
        EmergencyShutdown();
        digitalWrite(BUZZER_PIN, HIGH);
      }
    } else {
      // Deactivate emergency mode only if it was active
      if (emergencyShutdownActive) {
        emergencyShutdownActive = false;
        digitalWrite(BUZZER_PIN, LOW);
        // Add any post-emergency code here if needed
      }
    }
    
    vTaskDelay(pdMS_TO_TICKS(100));  // Adjust the delay as needed
  }
}

void EmergencyShutdown() {
  // Execute emergency shutdown procedures
    Serial.println("Task3 - EMERGENCY protocol is being executed...");
  // Send XML data to update the server
  server.send(200, "text/plain", "");
}
