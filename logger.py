# Fetches data via MQTT and logs by adding timestamp and converting into json format and store in /usr/bin/updates/JSON
# Also after 4 jsons are created, convertes this json into .csv format and store at /usr/bin/updates/CSV

import paho.mqtt.client as mqtt
import json
import os
import time
import csv
from datetime import datetime
import pytz

# MQTT Config
BROKER = "broker.hivemq.com"
PORT = 1883
TOPIC = "vaibhav/esp32/data"

# Directory Config
JSON_DIR = "/usr/bin/updates/JSON"
CSV_DIR = "/usr/bin/updates/CSV"

os.makedirs(JSON_DIR, exist_ok=True)
os.makedirs(CSV_DIR, exist_ok=True)

# Timezone
IST = pytz.timezone("Asia/Kolkata")

# Control variables
sensor_buffer = {}   # Stores latest S1-S4
last_save_time = 0
SAVE_INTERVAL = 15   # seconds


def parse_line(line):
    """
    Parse single sensor line
    Example: S1,239.74,35.89,0.87,22.23
    """
    parts = line.strip().split(",")
    if len(parts) != 5:
        return None

    return {
        "sensor": parts[0],
        "voltage": float(parts[1]),
        "current": float(parts[2]),
        "power_factor": float(parts[3]),
        "temperature": float(parts[4])
    }


def save_json():
    global sensor_buffer

    if len(sensor_buffer) != 4:
        print("Waiting for all 4 sensors...")
        return

    epoch_time = int(time.time())
    ist_time = datetime.fromtimestamp(epoch_time, IST).strftime('%Y-%m-%d %H:%M:%S')

    file_path = os.path.join(JSON_DIR, f"{epoch_time}.json")

    data = {
        "timestamp": ist_time,
        "epoch": epoch_time,
        "data": sensor_buffer
    }

    with open(file_path, "w") as f:
        json.dump(data, f, indent=4)

    print(f"JSON Saved: {file_path}")

    # Reset buffer after saving
    sensor_buffer = {}

    check_and_convert_to_csv()


def check_and_convert_to_csv():
    files = sorted([f for f in os.listdir(JSON_DIR) if f.endswith(".json")])

    if len(files) >= 4:
        convert_to_csv(files[:4])


def convert_to_csv(files):
    csv_file = os.path.join(CSV_DIR, f"{int(time.time())}.csv")

    with open(csv_file, "w", newline="") as csvfile:
        writer = csv.writer(csvfile)

        # Header
        writer.writerow([
            "timestamp",
            "S1_voltage", "S1_current", "S1_pf", "S1_temp",
            "S2_voltage", "S2_current", "S2_pf", "S2_temp",
            "S3_voltage", "S3_current", "S3_pf", "S3_temp",
            "S4_voltage", "S4_current", "S4_pf", "S4_temp"
        ])

        for file in files:
            path = os.path.join(JSON_DIR, file)

            with open(path, "r") as f:
                content = json.load(f)

                row = [content["timestamp"]]

                for sensor in ["S1", "S2", "S3", "S4"]:
                    s = content["data"].get(sensor, {})
                    row.extend([
                        s.get("voltage", ""),
                        s.get("current", ""),
                        s.get("power_factor", ""),
                        s.get("temperature", "")
                    ])

                writer.writerow(row)

    print(f"CSV Created: {csv_file}")

    # Delete used JSON files
    for file in files:
        os.remove(os.path.join(JSON_DIR, file))

    print("Deleted 4 JSON files.")


def on_connect(client, userdata, flags, rc):
    print("Connected:", rc)
    client.subscribe(TOPIC)


def on_message(client, userdata, msg):
    global last_save_time, sensor_buffer

    payload = msg.payload.decode().strip()
    print("Received:", payload)

    parsed = parse_line(payload)

    if parsed:
        sensor_buffer[parsed["sensor"]] = {
            "voltage": parsed["voltage"],
            "current": parsed["current"],
            "power_factor": parsed["power_factor"],
            "temperature": parsed["temperature"]
        }

    current_time = time.time()

    # Save every 15 sec
    if current_time - last_save_time >= SAVE_INTERVAL:
        save_json()
        last_save_time = current_time


# MQTT Setup
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, PORT, 60)

client.loop_forever()
