/*
* The `getAllItems()` function is making an HTTP GET request to retrieve a JSON payload from a server. It then deserializes the JSON payload into a `DynamicJsonDocument` object and iterates through each item in the document. 
* For each item, it creates a new `Item` object and populates its properties with the corresponding values from the JSON. It then adds the new `Item` object to a list called `itemList` and adds the `direction` property to another list called `directions`.
*/
void getAllItems()
{
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

/*
* The `i2c_Scanner()` function is scanning the I2C bus for connected devices. It iterates through each possible I2C address and checks if a device is present at that address. 
* If a device is found, its address is added to a list called `activeItems`. The function also creates a JSON object with the device's address and a frequency value, serializes it into a string, and publishes it to an MQTT topic. 
* Finally, the function prints the total number of devices found and their addresses to the serial monitor.
*/
void i2c_Scanner()
{
  int nDevices = 0;
  std::vector<String> deviceAddresses;

  activeItems.clear();

  Serial.println("I2C Devices Founded on the I2C Bus:");
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

  Serial.println("I2C Devices Connected: " + String(nDevices));

  for (int i = 0; i < activeItems.size(); i++)
  {
    Serial.println(activeItems.get(i));
  }
}

/*
* The `scanSPI()` function is scanning the SPI bus for connected devices. It iterates through each possible SPI address and checks if a device is present at that address. If a device is found, it increments the `deviceCount` variable. 
* If a device is not found, it increments the `disconnectedCount` variable. Finally, the function prints the total number of devices found and their addresses to the serial monitor.
*/
void scanSPI()
{
  byte deviceCount = 0;
  byte disconnectedCount = 0;
  Serial.println("Searching on the SPI Bus:");
  for (byte i = 0; i <= 0x7F; i++)
  {
    SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
    digitalWrite(SS, LOW);
    byte status = SPI.transfer(i);
    digitalWrite(SS, HIGH);
    SPI.endTransaction(); 
    if (status != 0xFF)
    { 
      if (i < 16)
      {
        
      }
      
      deviceCount++;
    }
    else
    {
      disconnectedCount++;
    }
  }
  if (deviceCount == 0)
  {
    Serial.println("Any SPI devices connected");
  }
  else
  {
    Serial.print(deviceCount);
    Serial.println(" devices connected by SPI.");
    if (disconnectedCount > 0)
    {
      Serial.print(disconnectedCount);
      Serial.println(" devices disconnected.");
    }
  }
}

/* 
* The `printElementsAPI()` function is making an HTTP GET request to an API endpoint for each active item in the `activeItems` list. It then retrieves a JSON payload from the server and deserializes it into a `DynamicJsonDocument` object. 
* It iterates through each item in the document and creates a new `Item` object with the `frequency_data` property from the JSON. It then adds the `frequency_data` value to a list called `frecuencyList`. Finally, the function prints the response payload and the highest frequency value in the `frecuencyList` to the serial monitor.
*/
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
        Serial.println("Response API REST:");
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
        Serial.println("Error in the GET request");
      }

      http.end();
    }
  }
}

/*
* The `updateFrecuency()` function is iterating through each item in the `frecuencyList` and finding the highest frequency value. It then converts the highest frequency value to a float and calculates the corresponding frequency in microseconds. 
* Finally, it prints the highest frequency value and the corresponding frequency in microseconds to the serial monitor.
*/
void updateFrecuency()
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
  Serial.println("Higher Frecuency :");
  Serial.println(maxElementString);
  float floatValue = maxElementString.toFloat();
  frecuenciaActual_New = static_cast<int>(1 / floatValue * 1000000);
  Serial.println("Higher Frecuency in microseconds :");
  Serial.println(frecuenciaActual_New);
}