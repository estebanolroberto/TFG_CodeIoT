import paho.mqtt.client as mqtt
import requests
import json

# URL base de la API REST local
API_BASE_URL = "http://localhost:5000/"

# método para conectar
def on_connect(client, user, flags, rc):
    print("Connect with result code: " + str(rc))
    client.subscribe("sensorHTU")
    client.subscribe("topic_pir_2")
    client.subscribe("topic_potent_2")

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
        
    elif msg.topic == "topic_pir_2":
        collection_name = "pir"
        # Convertir el mensaje de MQTT a un objeto JSON con el campo de presencia
        message_json = json.loads(msg.payload)
        presence = message_json["presence"]
        # Preparar el objeto JSON con los datos del sensor PIR
        data = {
            "sensor_type": "PIR",
            "presence": presence
            }
    elif msg.topic == "topic_potent_2":
        collection_name = "potent"
        # Convertir el mensaje de MQTT a un objeto JSON con el campo de potencia
        message_json = json.loads(msg.payload)
        power = message_json["power"]
        # Preparar el objeto JSON con los datos del sensor de potencia
        data = {
            "sensor_type": "POTENT",
            "power": power
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
client.connect("192.168.0.118", 1883, 60)

client.loop_forever()
