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
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <LinkedList.h>
#include "functions.h"
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void mostrarDatoEnRecuadro(int posX, int posY, const String &dato);
void handleSensorData();

/**
 * The above code defines four interrupt service routines for different timers in C++.
 */
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

/**
 * The setup function initializes various components and timers for the program.
 */
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
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextSize(1); 
  display.setTextColor(WHITE);

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

/**
 * The loop function checks for various interrupt flags and performs corresponding actions such as
 * handling sensor data, scanning I2C and SPI devices, displaying data on a screen, retrieving data
 * from a database, and updating information from an API.
 */
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
    display.clearDisplay(); 

    int cantidadDatos = activeItems.size();
    for (int i = 0; i < 6; i++)
    {
      int posX = (i % 3) * (SCREEN_WIDTH / 3);  
      int posY = (i / 3) * (SCREEN_HEIGHT / 2); 

      if (i < cantidadDatos)
      {
        String dato = activeItems.get(i);
        mostrarDatoEnRecuadro(posX, posY, dato);
      }
      else
      {
        int recuadroAncho = SCREEN_WIDTH / 3;
        int recuadroAlto = SCREEN_HEIGHT / 2;
        display.drawRect(posX, posY, recuadroAncho, recuadroAlto, WHITE);
      }
    }

    display.display(); 
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

/**
 * The function displays a given string in a rectangle on a screen.
 * 
 * @param posX The x-coordinate of the top-left corner of the rectangle where the data will be
 * displayed.
 * @param posY The y-coordinate of the top-left corner of the rectangle where the data will be
 * displayed.
 * @param dato A constant reference to a String variable that contains the data to be displayed inside
 * the rectangle.
 */
void mostrarDatoEnRecuadro(int posX, int posY, const String &dato)
{
  int recuadroAncho = SCREEN_WIDTH / 3;
  int recuadroAlto = SCREEN_HEIGHT / 2;
  display.drawRect(posX, posY, recuadroAncho, recuadroAlto, WHITE);
  int textPosX = posX + (recuadroAncho / 2) - (dato.length() * 3); 
  int textPosY = posY + (recuadroAlto / 2) - 8;                
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(textPosX, textPosY);
  display.print(dato);
}

/**
 * This function reads data from BMP280 and HTU21D sensors, displays the data on an OLED screen,
 * publishes the data to MQTT topics, and prints the data to the serial monitor.
 */
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
    float temperature = bmp280.readTemperature();
    float pressure = bmp280.readPressure();
    float altitud = bmp280.readAltitude();
    bmp280.begin(0x76);
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Sensor BMP280");
    display.print("Temperature: ");
    display.print(temperature);
    display.println(" *C");
    display.print("Pressure: ");
    display.print(pressure * 0.01);
    display.println("mbar");
    display.print("Altitude: ");
    display.print(altitud);
    display.println(" m");
    Serial.println("Se detectó el sensor BMP280.");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    sensor_bmp["temperature"] = temperature;
    Serial.println(" *C");
    Serial.print("Presión: ");
    Serial.print(pressure * 0.01);
    sensor_bmp["Pressure"] = pressure;
    Serial.println(" mbar");
    Serial.print("Altitud: ");
    Serial.print(altitud);
    sensor_bmp["Altitude"] = altitud;
    Serial.println(" m");
    Serial.println();
    serializeJson(sensor_bmp, String_sensor_bmp);
    PublishMqtt(String_sensor_bmp.c_str(), BMP_MQTT_TOPIC);
    bmp280Detected = true;
    display.display();
    delay(2500);
  }

  Wire.beginTransmission(HTU21DF_I2CADDR);
  if (Wire.endTransmission() == 0)
  {
    float temperature = htu21d.readTemperature();
    float humidity = htu21d.readHumidity();

    htu21d.begin();
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Sensor HTU21D");
    display.print("Temperature: ");
    display.print(temperature);
    display.println(" *C");
    display.print("Humidity: ");
    display.print(humidity);
    display.println(" %");
    Serial.println("Se detectó el sensor HTU21D.");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    sensor_htu["humidity"] = humidity;
    Serial.println(" %");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    sensor_htu["temperature"] = temperature;
    Serial.println(" *C");
    Serial.println();
    serializeJson(sensor_htu, String_sensor_htu);
    PublishMqtt(String_sensor_htu.c_str(), HTU_MQTT_TOPIC);
    htu21dDetected = true;
    display.display();
    delay(2500);
  }

  if (!htu21dDetected && !bmp280Detected && currentMillis > 2000)
  {
    Serial.println("No se detectó ningún sensor.");
  }
}
