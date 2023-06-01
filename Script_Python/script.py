import paho.mqtt.client as mqtt
import requests
import json

# URL base de la API REST local
API_BASE_URL = "http://192.168.0.123:5000/"

# método para conectar
def on_connect(client, user, flags, rc):
    print("Connect with result code: " + str(rc))
    client.subscribe("sensorHTU")
    client.subscribe("sensorBMP")
    client.subscribe("devices_connected")

# método para recibir mensajes
def on_message(client, user, msg):
    print("MESSAGE RECEIVED from TOPIC " + msg.topic + " : " + str(msg.payload))

    # Obtener el nombre de la colección de la URL en función del tema MQTT
    collection_name = ""
    if msg.topic == "sensorHTU":
        collection_name = "sensorHTU"
        # Convertir el mensaje de MQTT a un objeto JSON con los campos de temperatura y humedad
        try:
            message_json = json.loads(msg.payload)
            temperature = message_json["temperature"]
            humidity = message_json["humidity"]
            # Preparar el objeto JSON con los datos del sensor HTU
            data = {
                "sensor_type": "HTU",
                "direction": "0X40",
                "temperature": temperature,
                "humidity": humidity
            }
        except json.decoder.JSONDecodeError:
            print("Invalid JSON payload received from topic " + msg.topic)
            return
        
    elif msg.topic == "sensorBMP":
        collection_name = "sensorBMP"
        # Convertir el mensaje de MQTT a un objeto JSON con los campos de temperatura, presión y altitud
        try:
            message_json = json.loads(msg.payload)
            temperature_bmp = message_json["temperature"]
            pressure_bmp = message_json["pressure"]
            altitude_bmp = message_json["altitude"]
            # Preparar el objeto JSON con los datos del sensor BMP
            data = {
                "sensor_type": "BMP",
                "direction": "0X76",
                "temperature": temperature_bmp,
                "pressure": pressure_bmp,
                "altitude": altitude_bmp
            }
        except json.decoder.JSONDecodeError:
            print("Invalid JSON payload received from topic " + msg.topic)
            return
        
    elif msg.topic == "devices_connected":
        collection_name = "devices_connected"
        # Convertir el mensaje de MQTT a un objeto JSON con los campos de dirección y frecuencia
        try:
            message_json = json.loads(msg.payload)
            direction = message_json["direction"]
            frecuenciaActual = message_json["actual_frecuency"]
            # Preparar el objeto JSON con los datos de los dispositivos conectados
            data = {
                "direction": direction,
                "actual_frecuency": frecuenciaActual
            }
        except json.decoder.JSONDecodeError:
            print("Invalid JSON payload received from topic " + msg.topic)
            return

    # Generar la URL de la API REST local en función del tema MQTT y la colección correspondiente
    api_url = API_BASE_URL + collection_name

    # Realizar solicitud POST a la API REST local
    response = requests.post(api_url, json=data)

    # Manejar la respuesta de la API
    if response.status_code == 200:
        print("POST request successful")
    else:
        print("Error in POST request")


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.username_pw_set("roberto", "1299")
client.connect("192.168.0.123", 1883, 60)

client.loop_forever()
