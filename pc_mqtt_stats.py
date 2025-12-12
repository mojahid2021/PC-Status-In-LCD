
import time, json, socket, platform
import psutil
import paho.mqtt.client as mqtt

MQTT_BROKER = "localhost"   # set to broker IP if using remote broker
MQTT_PORT = 1883
TOPIC = "pc/stats"
INTERVAL = 1.0  # seconds

def get_stats():
    cpu = psutil.cpu_percent(interval=None)
    mem = psutil.virtual_memory()
    uptime = int(time.time() - psutil.boot_time())
    return {
        "timestamp": int(time.time()),
        "cpu_percent": cpu,
        "mem_percent": mem.percent,
        "uptime": uptime
    }

def on_publish(client, userdata, mid, reason_codes, properties):
    pass  # Optional: print("Message published, mid=", mid)

def main():
    client = mqtt.Client(client_id="pc-stats-publisher", callback_api_version=mqtt.CallbackAPIVersion.VERSION2)
    client.on_publish = on_publish
    rc = client.connect(MQTT_BROKER, MQTT_PORT, 60)
    if rc != 0:
        print(f"MQTT connect failed, rc={rc}")
        return
    print("MQTT connected")
    client.loop_start()
    try:
        while True:
            client.loop()
            stats = get_stats()
            payload = json.dumps(stats)
            client.publish(TOPIC, payload, qos=0, retain=False)
            print("Published:", payload)
            time.sleep(INTERVAL)
    except KeyboardInterrupt:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()
PY