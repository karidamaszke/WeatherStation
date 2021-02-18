import os
import http.client
import urllib.parse
import time
import subprocess

API_KEY = "PROVIDE_YOUR_API_KEY_HERE"


def send_data_to_cloud(temperature: float, pressure: float, humidity: float):
    headers = {"Content-typZZe": "application/x-www-form-urlencoded","Accept": "text/plain"}
    connection = http.client.HTTPConnection("api.thingspeak.com:80")
    params = urllib.parse.urlencode({
        'field1': temperature,
        'field2': pressure,
        'field3': humidity,
        'key':API_KEY
    }) 

    try:
        connection.request("POST", "/update", params, headers)
        response = connection.getresponse()

        connection.close()

    except Exception as ex:
        print("Connection failed! " + str(ex))


def read_from_file(name: str) -> float:
    if os.path.isfile(name):
        with open(name, 'r') as file:
            return float(file.read())
        os.remove(name)
    else:
        return 0.0


def read_data():
    while True:
        try:
            subprocess.run('./bme280')

            temperature = read_from_file('temperature')
            pressure = read_from_file('pressure')
            humidity = read_from_file('humidity')

            send_data_to_cloud(temperature, pressure, humidity)
            
            print("==========================================================================\n")
            
            time.sleep(59 * 60)

        except Exception as ex:
            print("Exception occured! " + str(ex))
            time.sleep(1)


if __name__ == "__main__":
    read_data()
