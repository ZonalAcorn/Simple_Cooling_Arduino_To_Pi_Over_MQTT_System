import time
import json
import hmac
import hashlib
import base64
import urllib.parse
import serial
import paho.mqtt.client as mqtt

# ================== CONFIG ==================
# From your device connection string
IOTHUB_HOSTNAME = "Capstone-IoT-Hub.azure-devices.net"  # HostName=
DEVICE_ID = "Bob_Gateway"  # DeviceId=
DEVICE_KEY = "Du+dd+K2vmF4eK7erwghWTn0nfhuzYYGFN5QcNjeEWM="  # SharedAccessKey=

# Serial config for Arduino
SERIAL_PORT = "/dev/ttyACM0"
BAUD_RATE = 9600

# SAS token lifetime (seconds)
SAS_TTL = 3600
# ============================================


def generate_sas_token(hostname, device_id, key, ttl=SAS_TTL):
    resource_uri = f"{hostname}/devices/{device_id}"
    resource_uri_encoded = urllib.parse.quote(resource_uri.lower(), safe='')
    expiry = int(time.time()) + ttl
    string_to_sign = f"{resource_uri_encoded}\n{expiry}".encode("utf-8")
    key_bytes = base64.b64decode(key)
    signature = hmac.HMAC(key_bytes, string_to_sign, hashlib.sha256).digest()
    signature_encoded = urllib.parse.quote(
        base64.b64encode(signature), safe=''
    )
    sas_token = (
        f"SharedAccessSignature sr={resource_uri_encoded}"
        f"&sig={signature_encoded}&se={expiry}"
    )
    return sas_token


def create_mqtt_client():
    sas_token = generate_sas_token(IOTHUB_HOSTNAME, DEVICE_ID, DEVICE_KEY)
    client_id = DEVICE_ID
    username = f"{IOTHUB_HOSTNAME}/{DEVICE_ID}/?api-version=2020-09-30"
    password = sas_token
    
    client = mqtt.Client(client_id=client_id, protocol=mqtt.MQTTv311)
    client.username_pw_set(username=username, password=password)
    client.tls_set()  # use system CAs
    client.connect(IOTHUB_HOSTNAME, port=8883)
    return client


def main():
    print(f"Opening serial port {SERIAL_PORT} at {BAUD_RATE} baud...")
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=2)
    
    print("Connecting to Azure IoT Hub via MQTT...")
    client = create_mqtt_client()
    client.loop_start()
    
    print("Connected. Streaming Arduino data to IoT Hub...\n")
    
    topic = f"devices/{DEVICE_ID}/messages/events/"
    
    try:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line:
                continue
                
            print("Raw from Arduino:", line)
            
            # Expect JSON from Arduino (one JSON object per line)
            try:
                data = json.loads(line)
            except json.JSONDecodeError:
                # If Arduino sends plain text, you can wrap it instead
                print("⚠️ Not valid JSON, wrapping as text")
                data = {"raw": line}
            
            payload = json.dumps(data)
            result = client.publish(topic, payload)
            status = result[0]
            
            if status == mqtt.MQTT_ERR_SUCCESS:
                print("✅ Sent to IoT Hub:", payload)
            else:
                print("⚠️ Publish failed with status", status)
            
            time.sleep(0.1)
            
    except KeyboardInterrupt:
        print("\nStopping...")
    finally:
        client.loop_stop()
        client.disconnect()
        ser.close()

if __name__ == "__main__":
    main()
