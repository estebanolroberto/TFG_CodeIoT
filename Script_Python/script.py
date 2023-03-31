import paho.mqtt.client as mqtt
import requests

# URL base de la API REST local
API_BASE_URL = "http://localhost:5000/"

# método para conectar
def on_connect(client, user, flags, rc):
    print("Connect with result code: " + str(rc))
    client.subscribe("sensorHTU")

# método para recibir mensajes
def on_message(client, user, msg):
    print("MESSAGE RECEIVED from TOPIC " + msg.topic + " : " + str(msg.payload))

    # Obtener el nombre de la colección de la URL en función del tema MQTT
    collection_name = ""
    if msg.topic == "sensorHTU":
        collection_name = "sensorHTU"
    elif msg.topic == "topic_pir":
        collection_name = "pir"
    elif msg.topic == "topic_potent":
        collection_name = "potent"

    # Generar la URL de la API REST local en función del tema MQTT y la colección correspondiente
    api_url = API_BASE_URL + collection_name

    # Realizar solicitud POST a la API REST local
    data = {'topic': msg.topic, 'payload': msg.payload.decode('utf-8')}
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
