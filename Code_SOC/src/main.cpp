#include <Wire.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_BMP280.h>
#include "config.h" // Sustituir con datos de vuestra red
#include "MQTT.hpp"
#include "ESP32_Utils.hpp"
#include "ESP32_Utils_MQTT_Async.hpp"


#define BMP280_ADDRESS (0x76)
#define HTU21DF_I2CADDR (0x40)
const char *HTU_MQTT_TOPIC = "sensorHTU";
const char *BMP_MQTT_TOPIC = "sensorBMP";          
Adafruit_HTU21DF htu21d = Adafruit_HTU21DF();
Adafruit_BMP280 bmp280;
HTTPClient http;
hw_timer_t * timer = NULL;

bool htu21dDetected = false;
bool bmp280Detected = false;
volatile bool interruptFlag = false;

String URL = url;
String lastItem = "";
String currentItem = "";
int HTTPCODE_SUCCESS = 200;
long time_presenceI2C= 5000000;
 
 
void IRAM_ATTR onTimer() {
  interruptFlag = true;
}

void setup() {

  Serial.begin(9600);
  Wire.begin();
  WiFi.onEvent(WiFiEvent);
  InitMqtt();
  ConnectWiFi_STA();
  http.begin(URL);
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, time_presenceI2C, true);
  timerAlarmEnable(timer);
}

void loop() {

  if (interruptFlag) {
    interruptFlag = false;
    handleSensorData();
  }
}

void handleSensorData() {
  unsigned long currentMillis = millis();

  StaticJsonDocument<200> sensor_htu;
  StaticJsonDocument<200> sensor_bmp;
  String String_sensor_htu;
  String String_sensor_bmp;

  currentItem = getLastItem();
  if (currentItem != lastItem) {
    Serial.println("Nuevo elemento añadido: " + currentItem);
    lastItem = currentItem;
  }

  Wire.beginTransmission(BMP280_ADDRESS);
  if (Wire.endTransmission() == 0) {
    bmp280.begin();
    Serial.println("Se detectó el sensor BMP280.");
    Serial.print("Temperatura: ");
    Serial.print(bmp280.readTemperature());
    sensor_bmp["temperature"] = bmp280.readTemperature();
    Serial.println(" *C");
    Serial.print("Presión: ");
    Serial.print(bmp280.readPressure() * 0.01);
    sensor_bmp["pressure"] = bmp280.readPressure();
    Serial.println(" mbar");
    Serial.print("Altitud: ");
    Serial.print(bmp280.readAltitude());
    sensor_bmp["altitude"] = bmp280.readAltitude();
    Serial.println(" m");
    Serial.println();
    serializeJson(sensor_bmp, String_sensor_bmp);
    PublishMqtt(String_sensor_bmp.c_str(), BMP_MQTT_TOPIC);
    bmp280Detected = true;
  }

  Wire.beginTransmission(HTU21DF_I2CADDR);
  if (Wire.endTransmission() == 0) {
    htu21d.begin();
    Serial.println("Se detectó el sensor HTU21D.");
    Serial.print("Humedad: ");
    Serial.print(htu21d.readHumidity());
    sensor_htu["humidity"] = htu21d.readHumidity();
    Serial.println(" %");
    Serial.print("Temperatura: ");
    Serial.print(htu21d.readTemperature());
    sensor_htu["temperature"] = htu21d.readTemperature();
    Serial.println(" *C");
    Serial.println();
    serializeJson(sensor_htu, String_sensor_htu);
    PublishMqtt(String_sensor_htu.c_str(), HTU_MQTT_TOPIC);
    htu21dDetected = true;
  }

  if (!htu21dDetected && !bmp280Detected && currentMillis > 2000) {
    Serial.println("No se detectó ningún sensor.");
  }
}
String getLastItem() {
  
  int httpCode = http.GET();
  String payload = http.getString();


  String lastItemValue = "";
  if (httpCode == HTTPCODE_SUCCESS) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    int size = doc.size();
    JsonObject lastItem = doc[size - 1];
    String name = lastItem["name"].as<String>();
    String type_connection = lastItem["type_connection"].as<String>();
    String direction = lastItem["direction"].as<String>();
    String description = lastItem["description"].as<String>();
    lastItemValue="Name: "+ name +", Tipo Conexión: "+type_connection+" Address:  "+direction + "Description: "+description;
  }
  return lastItemValue;
}