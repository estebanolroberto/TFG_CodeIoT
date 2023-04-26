const char* ssid     = "VodafoneMobileWiFi-984998";
const char* password = "2009693581";
const char *hostname = "ESP32_CASA";
const char* url = "http://192.168.0.120:5000/sensores";
const char *HTU_MQTT_TOPIC = "sensorHTU";
const char *BMP_MQTT_TOPIC = "sensorBMP";   

IPAddress ip(192, 168, 1, 200);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

const IPAddress MQTT_HOST(192, 168, 0, 120); //ip del broker
const int MQTT_PORT = 1883;
const char *MQTT_USER = "roberto";
const char *MQTT_PASSWORD = "1299";


unsigned long time_presenceI2C= 20000000;
unsigned long time_scanner= 10000000;
unsigned long time_scanner_bd= 25000000;
int HTTPCODE_SUCCESS = 200;


bool htu21dDetected = false;
bool bmp280Detected = false;
volatile bool interruptFlag = false;
volatile bool interruptFlag_scanner = false;
volatile bool interruptFlag_BD = false;