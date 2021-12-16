import paho.mqtt.client as mqtt
import threading
import time
import os

choice = 1
broker = 'localhost'
port = 1883
topic = "light"
client_name = "local"

global on
on = False

global home
home = True

global auto
auto = True

global lightLevel
lightLevel = 0

# Enter your deviceIds in the array below
devices = ["8006158A6DDE90637B70339BB2C022411E44150D"]

# Enter you account token below here
token = "25f45255-ATpg2n5PNJJfUCNExRszchd"

def setup_mqtt():
    print("Starting MQTT subscription...")
    mqtt_client = mqtt.Client(client_name)
    mqtt_client.connect(broker)

    mqtt_client.loop_start()


    mqtt_client.subscribe(topic)
    mqtt_client.on_message = handle_telemetry
    
def handle_telemetry(client, userdata, message):
    payload = message.payload.decode()
    print("A message came in:", payload)

    global auto
    global lightLevel
    global on
    global home

    # not using python 3.10 for switch statements :(
    if payload == "home":
        home = True
        print("Home mode active")
    elif payload == "leaving":
        home = False
        print("Home mode disabled")
    elif payload == "auto":
        auto = True
    elif payload == "manual":
        auto = False
        if on:
            tlp(0,0)
            on = False
        else:
            tlp(1,0)
            on = True
    
    if payload.isdigit():
        lightLevel = payload

    
def tlp(choice, device_choice):
    curl = """curl -s --request POST "https://wap.tplinkcloud.com/?token="""
    curl += token
    curl += """ HTTP/1.1" \\\
--data '{"method":"passthrough", "params": {"deviceId": \""""
    curl += devices[int(device_choice)]
    curl += """\", "requestData": "{\\"system\\":{\\"set_relay_state\\":{\\"state\\":"""
    curl += str(choice)
    curl += """}}}" }}' \\\
--header "Content-Type: application/json" \\\
%s > /dev/null """
    os.system(curl)


if __name__ == '__main__':
    serverThread = threading.Thread(target=setup_mqtt())
    serverThread.setDaemon(True)
    serverThread.start()
    tlp(0,0)
    on = False
    while True:
        if home:
            if auto: 
                print(lightLevel)
                if int(lightLevel) >= 201:
                    print("Turning off")
                    tlp(0, 0)
                    on = False
                elif int(lightLevel) <= 200:
                    print("Turning on")
                    tlp(1, 0)
                    on = True
            else: 
                continue
        elif not home:
            # disable lights if not home, and pause for additional time to reduce data sent
            tlp(0,0)
            on = False
            time.sleep(10)


        time.sleep(5)
