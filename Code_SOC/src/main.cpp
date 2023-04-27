#include <Wire.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <SPI.h>
#include <AsyncMqttClient.h>
#include <ArduinoJson.h>
#include <Adafruit_HTU21DF.h>
#include <Adafruit_BMP280.h>
#include "config.h"
#include "MQTT.hpp"
#include "ESP32_Utils.hpp"
#include "ESP32_Utils_MQTT_Async.hpp"
#include <list>
#define BMP280_ADDRESS (0x76)
#define HTU21DF_I2CADDR (0x40)

struct Item {
  String name;
  String type_connection;
  String direction;
  String description;
  String data_measure;
  String frequency_data;
};
       
Adafruit_HTU21DF htu21d = Adafruit_HTU21DF();
Adafruit_BMP280 bmp280;
HTTPClient http;
hw_timer_t * timer = NULL;
String lastItem,currentItem = "";

std::list<String> activeItems,activeItemsSPI;
std::list<Item> itemList;
std::list<Item> commonItems;

void scanSPI();
void i2c_Scanner();
void handleSensorData();
void getAllItems();

void IRAM_ATTR onTimer() {
  interruptFlag = true;
}
void IRAM_ATTR onTimerScanner() {
  interruptFlag_scanner = true;
}

void IRAM_ATTR onTimerListBD(){
  interruptFlag_BD =true;
}

void setup() {

  Serial.begin(9600);
  Wire.begin();
  SPI.begin();
  WiFi.onEvent(WiFiEvent);
  InitMqtt();
  ConnectWiFi_STA();
  http.begin(url);

  /*TIMERS*/
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, time_presenceI2C, true);
  timerAlarmEnable(timer);

  timer = timerBegin(1, 80, true);
  timerAttachInterrupt(timer, &onTimerScanner, true);
  timerAlarmWrite(timer, time_scanner, true);
  timerAlarmEnable(timer);

  timer = timerBegin(2, 80, true);
  timerAttachInterrupt(timer, &onTimerListBD, true);
  timerAlarmWrite(timer, time_scanner_bd, true);
  timerAlarmEnable(timer);
}

void loop() {
  if (interruptFlag) {
    interruptFlag = false;
    handleSensorData();
  }
  
  if(interruptFlag_scanner){
    interruptFlag_scanner = false;
    i2c_Scanner();
    scanSPI();
  }
  
  
  if(interruptFlag_BD){
    interruptFlag_BD = false;
     getAllItems();
     Serial.println("Dispositivos registrados en BD:");
    for (const auto& item : itemList) {
      Serial.print("Name: ");
      Serial.print(item.name);
      Serial.print(", Tipo Conexión: ");
      Serial.print(item.type_connection);
      Serial.print(", Address: ");
      Serial.print(item.direction);
      Serial.print(", Medida: ");
      Serial.print(item.data_measure);
      Serial.print(", Description: ");
      Serial.print(item.description);
      Serial.print(", Frecuencia: ");
      Serial.println(item.frequency_data);
      frecuencia_data = item.frequency_data;
  }
  itemList.clear();
  }
  
}


void i2c_Scanner() {
  activeItems.clear();
  byte error, address;
  int nDevices;
  nDevices = 0;
  Serial.println("Dispositivos encontrados en el bus I2C:");
  for (address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      deviceAddress = "0x" + String(address, HEX);
      activeItems.push_back("0X"+String(address));
      nDevices++;
      Serial.println("0X" + String(address, HEX));
      String url_GET = "http://192.168.0.120:5000/sensores/direction/" + deviceAddress;
    }
  }
  Serial.println("Total de dispositivos encontrados: " + String(nDevices));
}



void handleSensorData() {
  unsigned long currentMillis = millis();

  StaticJsonDocument<200> sensor_htu;
  StaticJsonDocument<200> sensor_bmp;
  String String_sensor_htu;
  String String_sensor_bmp;

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
void getAllItems() {
  int httpCode = http.GET();
  String payload = http.getString();

  if (httpCode == HTTPCODE_SUCCESS) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    int size = doc.size();
    for (int i = 0; i < size; i++) {
      JsonObject item = doc[i];
      Item newItem; // Crear un nuevo objeto Item
      newItem.name = item["name"].as<String>();
      newItem.type_connection = item["type_connection"].as<String>();
      newItem.direction = item["direction"].as<String>();
      newItem.description = item["description"].as<String>();
      itemList.push_back(newItem); // Agregar el nuevo Item a la lista
    }
  }
}

void scanSPI() {
  activeItemsSPI.clear();
  byte i;
  byte error, address;
  int nDevices_spi;
  nDevices_spi = 0;
  Serial.println("Scanning SPI Devices...");

  // Establecer los pines SPI dedicados para el bus SPI
  SPI.begin(GPIO_NUM_18, GPIO_NUM_19, GPIO_NUM_23, GPIO_NUM_5);

  for(address = 1; address <= 127; address++ ) {
    error = 0;
    digitalWrite(GPIO_NUM_5, LOW);
    error = SPI.transfer(address);
    digitalWrite(GPIO_NUM_5, HIGH);

    if (error == 0) {
      nDevices_spi++;
      activeItemsSPI.push_back("0X" + String(address, HEX));
      //Serial.print(address, DEC);
      //Serial.println("0X" + String(address, HEX) + ")");
    }
  }
  Serial.println("Total de dispositivos i2c encontrados: " + String(nDevices_spi));
  SPI.end();
}