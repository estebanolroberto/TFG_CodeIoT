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

Adafruit_SSD1306 *pDisplay = new Adafruit_SSD1306 (SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
void displayDataInBox(int posX, int posY, const String &dato,Adafruit_SSD1306 *pDisplay);
void handleSensorData();
void showScreen(int screenIndex);
void handleButton();
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
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(POTENTIOMETER_PIN, INPUT);
  pDisplay->begin(SSD1306_SWITCHCAPVCC, 0x3C);
  pDisplay->clearDisplay();
  pDisplay->setTextSize(1); 
  pDisplay->setTextColor(WHITE);
  showScreen(currentScreen);
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
  
    int potValue = analogRead(POTENTIOMETER_PIN);
    brightness = map(potValue, 0, 4096, 0, 255);
    pDisplay->dim(brightness);
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
    handleButton();
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

void handleButton() {
  // Leer el estado actual del botón
  int buttonState = digitalRead(BUTTON_PIN);

  // Verificar si el botón ha sido presionado
  if (buttonState == LOW) {
    // Esperar a que se suelte el botón
    while (digitalRead(BUTTON_PIN) == LOW) {
      delay(10);
    }

    // Cambiar a la siguiente pantalla
    currentScreen = (currentScreen + 1) % (activeItems.size() / 2);

    // Actualizar el índice de inicio de los datos
    startIndex = currentScreen * 2;

    // Mostrar la pantalla actualizada
    showScreen(currentScreen);
  } else {
    static unsigned long lastScreenChangeTime = 0;
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - lastScreenChangeTime;

    if (elapsedTime >= 1500) {
      // Cambiar a la siguiente pantalla
      currentScreen = (currentScreen + 1) % (activeItems.size() / 2);

      // Actualizar el índice de inicio de los datos
      startIndex = currentScreen * 2;

      // Mostrar la pantalla actualizada
      showScreen(currentScreen);

      // Actualizar el tiempo del último cambio de pantalla
      lastScreenChangeTime = currentTime;
    }
  }
}

void showScreen(int screenIndex) {
  // Limpiar el display
  pDisplay->clearDisplay();

  // Dibujar las líneas verticales de las celdas
  for (int col = 1; col < MAX_COLUMNS; col++) {
    int x = col * CELL_WIDTH;
    pDisplay->drawFastVLine(x, 0, SCREEN_HEIGHT, WHITE);
  }

  // Dibujar las líneas horizontales de las celdas
  for (int row = 1; row < MAX_ROWS; row++) {
    int y = row * CELL_HEIGHT;
    pDisplay->drawFastHLine(0, y, SCREEN_WIDTH, WHITE);
  }

  // Calcular el índice de inicio de los datos en la linked list
  int startIndex = screenIndex * (MAX_COLUMNS * MAX_ROWS);

  // Mostrar los datos en las celdas correspondientes
  pDisplay->setTextSize(1);
  pDisplay->setTextColor(WHITE);
  pDisplay->setTextWrap(false);

  for (int row = 0; row < MAX_ROWS; row++) {
    for (int col = 0; col < MAX_COLUMNS; col++) {
      // Calcular el índice del dato en la linked list
      int dataIndex = startIndex + (row * MAX_COLUMNS) + col;

      // Verificar si el índice está dentro del rango válido
      if (dataIndex < activeItems.size()) {
        // Obtener el dato de la linked list
        String data = activeItems.get(dataIndex);

        // Calcular la posición de la celda
        int cellX = col * CELL_WIDTH;
        int cellY = row * CELL_HEIGHT;

        // Mostrar el dato en la celda
        int textX = cellX + (CELL_WIDTH - (data.length() * 6)) / 2; // Asumiendo fuente de ancho fijo de 6 píxeles por carácter
        int textY = cellY + (CELL_HEIGHT - 8) / 2; // Altura de fuente de 8 píxeles
        pDisplay->setCursor(textX, textY);
        pDisplay->println(data);
      }
    }
  }

  // Actualizar el display
  pDisplay->display();
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
  display->drawRect(posX, posY, boxWidth, boxHigh, WHITE);
  int textPosX = posX + (boxWidth / 2) - (data.length() * 3); 
  int textPosY = posY + (boxHigh / 2) - 8;                
  display->setTextColor(WHITE);
  display->setTextSize(1);
  display->setCursor(textPosX, textPosY);
  display->print(data);
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
    pDisplay->println("Any sensor detected.");
    pDisplay->display();
  }
}
