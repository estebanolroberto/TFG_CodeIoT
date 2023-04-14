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
const char *HTU_MQTT_TOPIC = "sensorHTU";
const char *BMP_MQTT_TOPIC = "sensorBMP";

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

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(url);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Respuesta de la API REST:");
      Serial.println(payload);

      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload);

      // Obtener los datos de interés del JSON y mostrarlos por Serial
      String sensor_name = doc["name"];
      String type_connection = doc["type_connection"];
      String direccion = doc["direccion"];
      String description = doc["description"];
      Serial.print("Nombre del sensor: ");
      Serial.print(sensor_name);
      Serial.println();
      Serial.print("Type connection: ");
      Serial.print(type_connection);
      Serial.println();
      Serial.print("direccion: ");
      Serial.print(direccion);
      Serial.println();
      Serial.print("Description: ");
      Serial.print(description);
      Serial.println();
    } else {
      Serial.printf("Error al hacer la petición HTTP: %d\n", httpCode);
    }

    http.end();
  }
  unsigned long currentMillis = millis();
  StaticJsonDocument<200> sensor_htu;
  StaticJsonDocument<200> sensor_bmp;
  String String_sensor_htu;
  String String_sensor_bmp;

  if (currentMillis - lastRead >= 5000) {
    lastRead = currentMillis;

    // Check if BMP280 sensor is connected
    Wire.beginTransmission(0x76);
    if (Wire.endTransmission() == 0) {
      bmp280.begin(0x76);
      Serial.println("Se detectó el sensor BMP280.");
      Serial.print("Temperatura: ");
      Serial.print(bmp280.readTemperature());
      sensor_bmp["temperature"] = bmp280.readTemperature();
      Serial.println(" *C");
      Serial.print("Presión: ");
      Serial.print(bmp280.readPressure()*0.01);
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

    // Check if HTU21D sensor is connected
    Wire.beginTransmission(0x40);
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

    // Check if no sensor is detected
    if (!htu21dDetected && !bmp280Detected && !noSensorDetected && currentMillis > 2000) {
      Serial.println("No se detectó ningún sensor.");
      noSensorDetected = true;
    }
  }
}