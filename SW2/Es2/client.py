import requests
import json


def menu():
    print()
    print("Retrieve registering information:           registering")
    print("Retrieve all the registered devices:        devices")
    print("Retrieve all the registered users:          users")
    print("Retrieve a single device:                   device")
    print("Retrieve a single user:                     user")
    print("Exit program:                               exit")

def main():
    ip_address = "192.168.100.126"

    while True:
        menu()
        command = input("Enter command: ")
        if command not in ["registering", "devices", "users", "device", "user", "exit"]:
            print("ERROR: invalid command")
        elif command == "exit":
            break
        else:
            try:
                if command == "registering":
                    r = requests.get(f"http://{ip_address}:8080/")
                    print(json.dumps(json.loads(r.text)["subscriptions"]["MQTT"], indent=2))
                elif command == "devices":
                    r = requests.get(f"http://{ip_address}:8080/devices")
                    print(json.dumps(json.loads(r.text), indent=2))
                elif command == "users":
                    r = requests.get(f"http://{ip_address}:8080/users")
                    print(json.dumps(json.loads(r.text), indent=2))
                elif command == "device":
                    command = input("Enter deviceID: ")
                    r = requests.get(f"http://{ip_address}:8080/devices/{command}")
                    print(json.dumps(json.loads(r.text), indent=2))
                elif command == "user":
                    command = input("Enter userID: ")
                    r = requests.get(f"http://{ip_address}:8080/users/{command}")
                    print(json.dumps(json.loads(r.text), indent=2))
            except json.decoder.JSONDecodeError:
                print("Error decoding returned json")


if __name__ == '__main__':
    main()

