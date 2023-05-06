#define BMP280_ADDRESS (0x77)
#define HTU21DF_I2CADDR (0x40)       
Adafruit_HTU21DF htu21d = Adafruit_HTU21DF();
Adafruit_BMP280 bmp280;
HTTPClient http;
hw_timer_t * timer = NULL;
const char* ssid     = "VodafoneMobileWiFi-984998";
const char* password = "2009693581";
const char *hostname = "ESP32_CASA";
const char* url = "http://192.168.0.121:5000/devices";
const char* apiUrl = "http://192.168.0.121:5000/devices/direction/";
const char *HTU_MQTT_TOPIC = "sensorHTU";
const char *BMP_MQTT_TOPIC = "sensorBMP";
const char *DEVICES_MQTT_TOPIC ="devices_connected";   

IPAddress ip(192, 168, 0, 222);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

const IPAddress MQTT_HOST(192, 168, 0, 121); //ip del broker
const int MQTT_PORT = 1883;
const char *MQTT_USER = "roberto";
const char *MQTT_PASSWORD = "1299";


unsigned long timeCollectData= 65000000; //microsegundos   0,05hz
unsigned long time_scanner_Devices= 60000000;
unsigned long time_scanner_bd= 33000000;
unsigned long timePrintInformation = 72000000;
unsigned long frecuenciaActual = 30000000;

int HTTPCODE_SUCCESS = 200;
int maxFrequency = 0;
const int MAX_DEVICES = 20; 

struct Item {
  String name;
  String type_device;
  String type_connection;
  String direction;
  String description;
  String data_measure;
  String frequency_data;
  String min_valueData;
  String max_valueData;
  String units;
  String manufacturer;
  String deploymentDate;
  String interrupt_pin;
};

bool htu21dDetected = false;
bool bmp280Detected = false;
volatile bool interruptFlag = false;
volatile bool interruptFlag_scanner = false;
volatile bool interruptFlag_BD = false;
volatile bool interruptFlagGetInformationAPI = false;
volatile bool interruptFlag_ChangeFrecuency = false;
String deviceAddress,frecuencia_data;
String maxFreq;
String lastItem,currentItem = "";
LinkedList<String> activeItemsSPI,activeItems,soc_contains,frecuencyList;
LinkedList<Item> itemList;

