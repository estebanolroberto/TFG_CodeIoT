const char* ssid     = "VodafoneMobileWiFi-984998";
const char* password = "2009693581";
const char *hostname = "ESP32_CASA";
const char* url = "http://localhost:5000/sensores";

IPAddress ip(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const IPAddress MQTT_HOST(192, 168, 0, 118); //ip del broker
const int MQTT_PORT = 1883;
const char *MQTT_USER = "roberto";
const char *MQTT_PASSWORD = "1299";