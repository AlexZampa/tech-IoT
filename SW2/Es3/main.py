import requests
import json
import time

def update(url):
    r = requests.post(url)
    if r.status_code == 200:
        print("Update OK")

def main():
    ip_address = "192.168.1.11"
    r = requests.get(f"http://{ip_address}:8080/")
    j = json.loads(r.text)
    devices_endpoint = j["subscriptions"]["REST"]["device"]
    r = requests.post(devices_endpoint, json.dumps({
        "end-points": [
            "/ciao"
        ],
        "resources": [
            "Temperature",
            "Humidity"
        ]
    }))
    j = json.loads(r.text)
    deviceID = j["deviceID"]
    while True:
        time.sleep(60)
        update(f"{devices_endpoint}/{deviceID}")

if __name__ == '__main__':
    main()
