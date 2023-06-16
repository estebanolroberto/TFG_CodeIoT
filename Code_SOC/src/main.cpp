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
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* clock=*/22, /* data=*/21, /* reset=*/255);
Adafruit_SSD1306 *pDisplay = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
void displayDataInBox(int posX, int posY, const String &dato, Adafruit_SSD1306 *pDisplay);
void handleSensorData();

int originalTextSize = 1;
int currentTextSize = originalTextSize;

volatile bool buttonPressed = false;

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
  u8g2.begin();
  SPI.begin();
  WiFi.onEvent(WiFiEvent);
  InitMqtt();
  ConnectWiFi_STA();
  http.begin(url);
  pinMode(SS, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  potentiometerValue = 0;
  pinMode(POTENTIOMETER_PIN, INPUT);
  pDisplay->begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pDisplay->clearDisplay();
  pDisplay->setTextSize(1);
  pDisplay->setTextColor(WHITE);

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
  potentiometerValue = analogRead(POTENTIOMETER_PIN);
  int brightness = map(potentiometerValue, 0, 4095, 0, 255); // Ajusta el rango según sea necesario
  pDisplay->ssd1306_command(SSD1306_SETCONTRAST);
  pDisplay->ssd1306_command(brightness);
  u8g2.setContrast(brightness);

  if (digitalRead(BUTTON_PIN) == LOW)
  {
    // Botón presionado
    delay(50); // Pequeña pausa para evitar rebotes
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      // Confirma que el botón sigue presionado después de la pausa
      currentTextSize++; // Incrementa el tamaño del texto
      if (currentTextSize > 2)
      {
        currentTextSize = originalTextSize; // Vuelve al tamaño original si se alcanza el tamaño máximo
      }

      // Aplica el nuevo tamaño del texto
      pDisplay->setTextSize(currentTextSize);

      // Espera a que el botón sea liberado
      while (digitalRead(BUTTON_PIN) == LOW)
      {
        delay(10);
      }
    }
  }

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
    pDisplay->clearDisplay();

    int amountData = activeItems.size();
    for (int i = 0; i < 6; i++)
    {
      int posX = (i % 3) * (SCREEN_WIDTH / 3);
      int posY = (i / 3) * (SCREEN_HEIGHT / 2);

      if (i < amountData)
      {
        String data = activeItems.get(i);
        displayDataInBox(posX, posY, data, pDisplay);
      }
      else
      {
        int boxWidth = SCREEN_WIDTH / 3;
        int boxHigh = SCREEN_HEIGHT / 2;
        pDisplay->drawRect(posX, posY, boxWidth, boxHigh, WHITE);
        pDisplay->setTextColor(WHITE, BLACK);
      }
    }

    pDisplay->display();
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
    updateFrecuency();
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
void displayDataInBox(int posX, int posY, const String &data, Adafruit_SSD1306 *display)
{
  int boxWidth = SCREEN_WIDTH / 3;
  int boxHigh = SCREEN_HEIGHT / 2;
  pDisplay->drawRect(posX, posY, boxWidth, boxHigh, WHITE);
  int textPosX = posX + (boxWidth / 2) - (data.length() * 3);
  int textPosY = posY + (boxHigh / 2) - 8;
  pDisplay->setTextColor(WHITE, BLACK);
  pDisplay->setTextSize(currentTextSize);
  pDisplay->setCursor(textPosX, textPosY);
  pDisplay->print(data);
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
    pDisplay->clearDisplay();
    pDisplay->setCursor(0, 0);
    pDisplay->setTextColor(WHITE, BLACK);
    pDisplay->println("Sensor BMP280");
    pDisplay->println("Address: 0x76");
    pDisplay->print("Temperature: ");
    pDisplay->print(temperature);
    pDisplay->println(" *C");
    pDisplay->print("Pressure: ");
    pDisplay->print(pressure * 0.01);
    pDisplay->println("mbar");
    pDisplay->print("Altitude: ");
    pDisplay->print(altitud);
    pDisplay->println(" m");
    Serial.println("Detected sensor BMP280.");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    sensor_bmp["temperature"] = temperature;
    sensor_bmp["direction"] = "0X76";
    Serial.println(" *C");
    Serial.print("Presión: ");
    Serial.print(pressure * 0.01);
    sensor_bmp["pressure"] = pressure;
    Serial.println(" mbar");
    Serial.print("Altitud: ");
    Serial.print(altitud);
    sensor_bmp["altitude"] = altitud;
    Serial.println(" m");
    Serial.println();
    serializeJson(sensor_bmp, String_sensor_bmp);
    PublishMqtt(String_sensor_bmp.c_str(), BMP_MQTT_TOPIC);
    bmp280Detected = true;
    pDisplay->display();
    delay(2500);
  }

  Wire.beginTransmission(HTU21DF_I2CADDR);
  if (Wire.endTransmission() == 0)
  {
    float temperature = htu21d.readTemperature();
    float humidity = htu21d.readHumidity();

    htu21d.begin();
    pDisplay->clearDisplay();
    pDisplay->setCursor(0, 0);
    pDisplay->setTextColor(WHITE, BLACK);
    pDisplay->println("Sensor HTU21D");
    pDisplay->println("Address: 0x40");
    pDisplay->print("Temperature: ");
    pDisplay->print(temperature);
    pDisplay->println(" *C");
    pDisplay->print("Humidity: ");
    pDisplay->print(humidity);
    pDisplay->println(" %");
    Serial.println("Detected sensor HTU21D.");
    Serial.print("Humidity: ");
    Serial.print(humidity);
    sensor_htu["humidity"] = humidity;
    sensor_htu["direction"] = "0X40";
    Serial.println(" %");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    sensor_htu["temperature"] = temperature;
    Serial.println(" *C");
    Serial.println();
    serializeJson(sensor_htu, String_sensor_htu);
    PublishMqtt(String_sensor_htu.c_str(), HTU_MQTT_TOPIC);
    htu21dDetected = true;
    pDisplay->display();
    delay(2500);
  }

  Wire.beginTransmission(0X3C);
  if (Wire.endTransmission() == 0)
  {
    htu21d.begin();
    pDisplay->clearDisplay();
    pDisplay->setCursor(0, 0);
    pDisplay->setTextColor(WHITE, BLACK);
    pDisplay->println("Actuator");
    pDisplay->println("Address: 0x3C");
    pDisplay->print("Screen OLED ");
    screenDetected = true;
    pDisplay->display();
    delay(2500);
  }

  if (!htu21dDetected && !bmp280Detected && !screenDetected && currentMillis > 2000)
  {
    Serial.println("Any sensor detected.");
    pDisplay->clearDisplay();
    pDisplay->setCursor(0, 0);
    pDisplay->setTextColor(WHITE, BLACK);
    pDisplay->println("Any sensor detected.");
    pDisplay->display();
  }
}
