import paho.mqtt.client as mqtt
import requests
import json

# URL base de la API REST local
API_BASE_URL = "http://localhost:5000/"

# método para conectar
def on_connect(client, user, flags, rc):
    print("Connect with result code: " + str(rc))
    client.subscribe("sensorHTU")
    client.subscribe("sensorBMP")

# método para recibir mensajes
def on_message(client, user, msg):
    print("MESSAGE RECEIVED from TOPIC " + msg.topic + " : " + str(msg.payload))

    # Obtener el nombre de la colección de la URL en función del tema MQTT
    collection_name = ""
    if msg.topic == "sensorHTU":
        collection_name = "sensorHTU"
        # Convertir el mensaje de MQTT a un objeto JSON con los campos de temperatura y humedad
        message_json = json.loads(msg.payload)
        temperature = message_json["temperature"]
        humidity = message_json["humidity"]
        # Preparar el objeto JSON con los datos del sensor HTU
        data = {
            "sensor_type": "HTU",
            "temperature": temperature,
            "humidity": humidity
            }
        
    elif msg.topic == "sensorBMP":
        collection_name = "sensorBMP"
        # Convertir el mensaje de MQTT a un objeto JSON con el campo de presencia
        message_json = json.loads(msg.payload)
        temperature_bmp = message_json["temperature"]
        pressure_bmp = message_json["pressure"]
        altitude_bmp = message_json["altitude"]
        # Preparar el objeto JSON con los datos del sensor PIR
        data = {
            "sensor_type": "BMP",
            "temperature": temperature_bmp,
            "pressure": pressure_bmp,
            "altitude": altitude_bmp,
            }

    # Generar la URL de la API REST local en función del tema MQTT y la colección correspondiente
    api_url = API_BASE_URL + collection_name

    # Realizar solicitud POST a la API REST local
    response = requests.post(api_url, json=data)

    # Manejar la respuesta de la API
    if response.status_code == 200:
        print("POST request successful")
    else:
        print("Error in POST request")

# crear objeto y callbacks
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# conexión al broker
client.username_pw_set("roberto", "1299")
client.connect("192.168.0.120", 1883, 60)

client.loop_forever()
