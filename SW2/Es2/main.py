import requests
import json
import sys

ip_address = "192.168.100.126"

def main(argv):
    if len(argv) > 0:
        try:
            if argv[0] == 'registering':
                r = requests.get(f"http://{ip_address}:8080/")
                print(json.dumps(json.loads(r.text)["subscriptions"]["MQTT"], indent=2))
            elif argv[0] == 'devices':
                r = requests.get(f"http://{ip_address}:8080/devices")
                print(json.dumps(json.loads(r.text), indent=2))
            elif argv[0] == 'users':
                r = requests.get(f"http://{ip_address}:8080/users")
                print(json.dumps(json.loads(r.text), indent=2))
            elif argv[0] == 'device' and len(argv) == 2:
                r = requests.get(f"http://{ip_address}:8080/devices/{argv[1]}")
                print(json.dumps(json.loads(r.text), indent=2))
            elif argv[0] == 'user' and len(argv) == 2:
                r = requests.get(f"http://{ip_address}:8080/users/{argv[1]}")
                print(json.dumps(json.loads(r.text), indent=2))
            else:
                print("Wrong command")
        except json.decoder.JSONDecodeError:
            print("Error decoding json")
    else:
        print("Error: miss arg")


if __name__ == '__main__':
    main(sys.argv[1:])
