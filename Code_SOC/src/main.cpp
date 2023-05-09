#include <Wire.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <LinkedList.h>
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

LinkedList<String> directions;
void frecuenciasActualizada();
void printElementsAPI();
void scanSPI();
void i2c_Scanner();
void handleSensorData();
void getAllItems();

void IRAM_ATTR onTimerDataDevices()
{
  interruptFlag = true;
}
void IRAM_ATTR onTimerScannerDevices()
{
  interruptFlag_scanner = true;
}

void IRAM_ATTR onTimerListBDDevices()
{
  interruptFlag_BD = true;
}

void IRAM_ATTR onTimerGetInformationAPI()
{
  interruptFlagGetInformationAPI = true;
}

void setup()
{

  Serial.begin(9600);
  Wire.begin();
  SPI.begin();
  WiFi.onEvent(WiFiEvent);
  InitMqtt();
  ConnectWiFi_STA();
  http.begin(url);
  pinMode(SS, OUTPUT);

  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimerDataDevices, true);
  timerAlarmWrite(timer, timeCollectData, true);
  timerAlarmEnable(timer);

  timer1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer1, &onTimerScannerDevices, true);
  timerAlarmWrite(timer1, time_scanner_Devices, true);
  timerAlarmEnable(timer1);

  timer2 = timerBegin(2, 80, true);
  timerAttachInterrupt(timer2, &onTimerListBDDevices, true);
  timerAlarmWrite(timer2, time_scanner_bd, true);
  timerAlarmEnable(timer2);

  timer3 = timerBegin(3, 80, true);
  timerAttachInterrupt(timer3, &onTimerGetInformationAPI, true);
  timerAlarmWrite(timer3, timePrintInformation, true);
  timerAlarmEnable(timer3);
}

void loop()
{
  if (interruptFlag)
  {
    interruptFlag = false;
    handleSensorData();
  }

  if (interruptFlag_scanner)
  {
    interruptFlag_scanner = false;
    i2c_Scanner();
    scanSPI();
  }

  if (interruptFlag_BD)
  {
    interruptFlag_BD = false;
    getAllItems();
  }

  if (interruptFlagGetInformationAPI)
  {
    interruptFlagGetInformationAPI = false;
    printElementsAPI();
    frecuenciasActualizada();
    timerAlarmWrite(timer, frecuenciaActual_New, true);
  }
}


void frecuenciasActualizada()
{

  for (int i = 0; i < frecuencyList.size(); i++)
  {
    String elementString = frecuencyList.get(i);
    double element = elementString.toDouble();
    if (element > maxElement)
    {
      maxElement = element;
      maxElementString = elementString;
    }
  }
  Serial.println("Frecuencia mas alta");
  Serial.println(maxElementString);
  float floatValue = maxElementString.toFloat();
  frecuenciaActual_New = static_cast<int>(1 / floatValue * 1000000);
  Serial.println("Frecuencia Actual en microsegundos");
  Serial.println(frecuenciaActual_New);
}

void printElementsAPI()
{
  frecuencyList.clear();
  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http_api;

    for (int i = 0; i < activeItems.size(); i++)
    {
      char path[128];
      strcpy(path, apiUrl);
      strcat(path, activeItems.get(i).c_str());

      http.begin(path);
      Serial.println(path);
      int httpCode = http.GET();

      if (httpCode > 0)
      {
        String payload = http.getString();
        Serial.println("Respuesta de la API REST:");
        Serial.println(payload);

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, payload);
        int size = doc.size();
        for (int i = 0; i < size; i++)
        {
          JsonObject item = doc[i];
          Item newItem_freq;
          newItem_freq.frequency_data = item["frequency_data"].as<String>();
          frecuencyList.add(newItem_freq.frequency_data);
        }
      }
      else
      {
        Serial.println("Error en la solicitud GET");
      }

      http.end();
    }
  }
}

void i2c_Scanner()
{
  int nDevices = 0;
  std::vector<String> deviceAddresses; // vector para almacenar las direcciones de los dispositivos encontrados

  activeItems.clear();

  Serial.println("Dispositivos encontrados en el bus I2C:");
  for (byte address = 1; address < 127; address++)
  {
    Wire.beginTransmission(address);
    byte error = Wire.endTransmission();
    if (error == 0)
    {
      String deviceAddress = "0X" + String(address, HEX);
      activeItems.add(deviceAddress);
      deviceAddresses.push_back(deviceAddress);

      StaticJsonDocument<200> device_connected;
      device_connected["direction"] = deviceAddress;
      device_connected["actual_frecuency"] = frecuenciaActual_New;

      String String_devices_connected;
      serializeJson(device_connected, String_devices_connected);
      PublishMqtt(String_devices_connected.c_str(), DEVICES_MQTT_TOPIC);

      nDevices++;
    }
    if (nDevices == MAX_DEVICES)
      break;
  }

  String devicesStr = "";
  for (int i = 0; i < deviceAddresses.size(); i++)
  {
    devicesStr += deviceAddresses[i];
    if (i < deviceAddresses.size() - 1)
    {
      devicesStr += ", ";
    }
  }

  Serial.println("Total de dispositivos I2C encontrados: " + String(nDevices));

  for (int i = 0; i < activeItems.size(); i++)
  {
    Serial.println(activeItems.get(i));
  }
}


void handleSensorData()
{
  unsigned long currentMillis = millis();

  StaticJsonDocument<200> sensor_htu;
  StaticJsonDocument<200> sensor_bmp;
  String String_sensor_htu;
  String String_sensor_bmp;

  Wire.beginTransmission(BMP280_ADDRESS);
  if (Wire.endTransmission() == 0)
  {
    bmp280.begin(0x76);
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
  if (Wire.endTransmission() == 0)
  {
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

  if (!htu21dDetected && !bmp280Detected && currentMillis > 2000)
  {
    Serial.println("No se detectó ningún sensor.");
  }
}
void getAllItems()
{
  // directions.clear();
  int httpCode = http.GET();
  String payload = http.getString();

  if (httpCode == HTTPCODE_SUCCESS)
  {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload);
    int size = doc.size();
    for (int i = 0; i < size; i++)
    {
      JsonObject item = doc[i];
      Item newItem;
      newItem.name = item["name"].as<String>();
      newItem.type_connection = item["type_connection"].as<String>();
      newItem.direction = item["direction"].as<String>();
      newItem.description = item["description"].as<String>();
      itemList.add(newItem);
      directions.add(newItem.direction);
    }
  }
}

void scanSPI()
{
  byte deviceCount = 0;
  byte disconnectedCount = 0;
  Serial.println("Buscando Dispositivos  en el bus SPI:");
  for (byte i = 0; i <= 0x7F; i++)
  {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(SS, LOW);
    byte status = SPI.transfer(i);
    digitalWrite(SS, HIGH);
    SPI.endTransaction(); // Finalizar la transacción SPI
    if (status != 0xFF)
    { // Si el dispositivo responde
      // Serial.print("Dispositivo encontrado en la dirección 0x");
      if (i < 16)
      {
        // Serial.print("0");
      }
      // Serial.println(i, HEX);
      deviceCount++;
    }
    else
    {
      disconnectedCount++;
    }
  }
  if (deviceCount == 0)
  {
    Serial.println("No se encontraron dispositivos conectados por SPI");
  }
  else
  {
    Serial.print(deviceCount);
    Serial.println(" dispositivos conectados por SPI.");
    if (disconnectedCount > 0)
    {
      Serial.print(disconnectedCount);
      Serial.println(" dispositivos desconectados.");
    }
  }
}