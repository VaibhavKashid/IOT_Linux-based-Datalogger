#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

HardwareSerial mySerial(2);  // UART2 --> Connect Arduino's Tx pin to ESP's RX2 pin

const char* ssid = "Vaibhav";
const char* password = "12345678";

// MQTT Broker
const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

String inputLine = "";

void setup_wifi() {

  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {

  while (!client.connected()) {

    Serial.print("Connecting to MQTT...");

    if (client.connect("ESP32_Client")) {

      Serial.println("Connected!");

    } else {

      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying...");

      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }
}

// TASK 1 : WiFi + MQTT  --> CORE 0
void mqttTask(void * parameter) {

  setup_wifi();

  client.setServer(mqtt_server, 1883);

  while (1) {

    if (!client.connected()) {
      reconnect();
    }

    client.loop();

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// TASK 2 : UART2 + LCD  --> CORE 1
void serialTask(void * parameter) {

  while (1) {

    // Read UART2 data
    while (mySerial.available()) {

      char c = mySerial.read();

      if (c == '\n') {

        inputLine.trim();

        if (inputLine.length() > 0) {

          Serial.print("Publishing: ");
          Serial.println(inputLine);

          // Publish MQTT
          client.publish("vaibhav/esp32/data",
                         inputLine.c_str());

          // LCD Display
          if (inputLine.startsWith("S1,")) {

            float v, i, pf, p;

            sscanf(inputLine.c_str(),
                   "S1,%f,%f,%f,%f",
                   &v, &i, &pf, &p);

            lcd.clear();

            lcd.setCursor(0, 0);
            lcd.print("V:");
            lcd.print(v);

            lcd.setCursor(9, 0);
            lcd.print("I:");
            lcd.print(i);

            lcd.setCursor(0, 1);
            lcd.print("PF:");
            lcd.print(pf);

            lcd.setCursor(9, 1);
            lcd.print("P:");
            lcd.print(p);
          }
        }

        inputLine = "";
      }

      else {

        inputLine += c;
      }
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

void setup() {

  // UART0 for debugging
  Serial.begin(9600);

  // UART2 for external device
  mySerial.begin(9600, SERIAL_8N1, 16, 17);

  // I2C
  Wire.begin(21, 22);

  // LCD
  lcd.begin(16, 2);
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("System Starting");

  // Create MQTT Task on Core 0
  xTaskCreatePinnedToCore(
    mqttTask,          // Task Function
    "MQTT_Task",       // Task Name
    10000,             // Stack Size
    NULL,              // Parameters
    1,                 // Priority
    NULL,              // Task Handle
    0                  // Core 0
  );

  // Create UART/LCD Task on Core 1
  xTaskCreatePinnedToCore(
    serialTask,
    "Serial_Task",
    10000,
    NULL,
    1,
    NULL,
    1                  // Core 1
  );
}

void loop() {

}
