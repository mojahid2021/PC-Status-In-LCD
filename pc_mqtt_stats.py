
import time, json, socket, platform
import psutil
import paho.mqtt.client as mqtt

MQTT_BROKER = "localhost"   # set to broker IP if using remote broker
MQTT_PORT = 1883
TOPIC = "pc/stats"
INTERVAL = 2.0  # seconds

def get_stats():
    cpu = psutil.cpu_percent(interval=None)
    mem = psutil.virtual_memory()
    disk = psutil.disk_usage('/')
    uptime = int(time.time() - psutil.boot_time())
    hostname = socket.gethostname()
    system = platform.system() + " " + platform.release()
    return {
        "host": hostname,
        "system": system,
        "cpu_percent": round(cpu,1),
        "mem_total": mem.total,
        "mem_used": mem.used,
        "mem_percent": round(mem.percent,1),
        "disk_total": disk.total,
        "disk_used": disk.used,
        "disk_percent": round(disk.percent,1),
        "uptime": uptime
    }

def main():
    client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2, "pc-stats-publisher")
    client.connect(MQTT_BROKER, MQTT_PORT, 60)
    client.loop_start()
    try:
        while True:
            stats = get_stats()
            payload = json.dumps(stats)
            client.publish(TOPIC, payload, qos=0, retain=False)
            time.sleep(INTERVAL)
    except KeyboardInterrupt:
        client.loop_stop()
        client.disconnect()

if __name__ == "__main__":
    main()
PY