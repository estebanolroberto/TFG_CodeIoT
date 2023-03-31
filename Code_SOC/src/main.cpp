#include <Wire.h>
#include <WiFi.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_BMP280.h>
#include "config.h" // Sustituir con datos de vuestra red
#include "MQTT.hpp"
#include "ESP32_Utils.hpp"
#include "ESP32_Utils_MQTT_Async.hpp"
const char *HTU_MQTT_TOPIC = "sensorHTU";
const char *BMP_MQTT_TOPIC = "bmp280";

Adafruit_HTU21DF htu21d = Adafruit_HTU21DF();
Adafruit_BMP280 bmp280;

unsigned long lastRead = 0;
bool htu21dDetected = false;
bool bmp280Detected = false;
bool noSensorDetected = false;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  WiFi.onEvent(WiFiEvent);
	InitMqtt();
	ConnectWiFi_STA();
}

void loop() {
  unsigned long currentMillis = millis();
  StaticJsonDocument<200> sensor_htu;
  StaticJsonDocument<200> sensor_bmp;
  String String_sensor_htu;
  String String_sensor_bmp;
  if (currentMillis - lastRead >= 5000) {
    lastRead = currentMillis;
    if (!htu21dDetected && !htu21d.begin()) {
      if (bmp280.begin()) {
        Serial.println("Se detectó el sensor BMP280.");
        Serial.print("Temperatura: ");
        Serial.print(bmp280.readTemperature());
        sensor_bmp["temperature"] = bmp280.readTemperature();
        Serial.println(" *C");
        Serial.print("Presión: ");
        Serial.print(bmp280.readPressure());
        sensor_bmp["pressure"] = bmp280.readPressure();
        Serial.println(" Pa");
        Serial.println();
        serializeJson(sensor_bmp, String_sensor_bmp);
        PublishMqtt(String_sensor_bmp.c_str(), BMP_MQTT_TOPIC);
        bmp280Detected = true;
      }
    } else if (!bmp280Detected && !bmp280.begin()) {
      if (htu21d.begin()) {
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
    } else if (!htu21dDetected && !bmp280Detected && !noSensorDetected && currentMillis > 2000) {
      Serial.println("No se detectó ningún sensor.");
      noSensorDetected = true;
    }
  }
}