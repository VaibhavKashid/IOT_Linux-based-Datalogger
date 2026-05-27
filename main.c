//To be uploaded on ESP32 while interfaced to 16X2 LCD display using I2C
//Receives data through UART2
// Implemented RTOS

#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define RXD2 16
#define TXD2 17

LiquidCrystal_I2C lcd(0x27, 16, 2);
HardwareSerial mySerial(2);

const char* ssid = "Vaibhav";
const char* password = "12345678";

const char* mqtt_server = "broker.hivemq.com";

WiFiClient espClient;
PubSubClient client(espClient);

QueueHandle_t mqttQueue;

// Structure for queue
typedef struct {
  char message[100];
} mqttMessage;

void setup_wifi() {

  Serial.println("Connecting WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected");
  Serial.println(WiFi.localIP());
}

void reconnect() {

  while (!client.connected()) {

    Serial.print("Connecting MQTT...");

    if (client.connect("ESP32_Client")) {

      Serial.println("Connected");

    } else {

      Serial.print("Failed rc=");
      Serial.println(client.state());

      vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
  }
}

// ================= MQTT TASK =================
void mqttTask(void * parameter) {

  mqttMessage rxMsg;

  setup_wifi();

  client.setServer(mqtt_server, 1883);

  while (1) {

    if (!client.connected()) {
      reconnect();
    }

    client.loop();

    // Receive data from queue
    if (xQueueReceive(mqttQueue, &rxMsg, 10) == pdPASS) {

      Serial.print("Publishing: ");
      Serial.println(rxMsg.message);

      client.publish("vaibhav/esp32/data",
                     rxMsg.message);
    }

    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}

// ================= UART + LCD TASK =================
void serialTask(void * parameter) {

  char inputLine[100];
  int index = 0;

  while (1) {

    while (mySerial.available()) {

      char c = mySerial.read();

      // End of line
      if (c == '\n') {

        inputLine[index] = '\0';

        if (strlen(inputLine) > 0) {

          mqttMessage txMsg;

          strcpy(txMsg.message, inputLine);

          // Send to MQTT queue
          xQueueSend(mqttQueue, &txMsg, portMAX_DELAY);

          // ===== LCD Display =====
          if (strncmp(inputLine, "S1,", 3) == 0) {

            float v, i, pf, p;

            sscanf(inputLine,
                   "S1,%f,%f,%f,%f",
                   &v, &i, &pf, &p);

            // Instead of lcd.clear()
            lcd.setCursor(0, 0);
            lcd.print("V:");
            lcd.print(v, 1);
            lcd.print("   ");

            lcd.setCursor(9, 0);
            lcd.print("I:");
            lcd.print(i, 1);
            lcd.print("   ");

            lcd.setCursor(0, 1);
            lcd.print("PF:");
            lcd.print(pf, 2);
            lcd.print(" ");

            lcd.setCursor(9, 1);
            lcd.print("P:");
            lcd.print(p, 1);
            lcd.print("   ");
          }
        }

        index = 0;
      }

      else {

        // Prevent buffer overflow
        if (index < sizeof(inputLine) - 1) {

          inputLine[index++] = c;
        }
      }
    }

    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void setup() {

  Serial.begin(115200);

  mySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);

  Wire.begin(21, 22);

  lcd.begin(16, 2);
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("System Starting");

  // Create Queue
  mqttQueue = xQueueCreate(10, sizeof(mqttMessage));

  // MQTT Task
  xTaskCreatePinnedToCore(
    mqttTask,
    "MQTT_Task",
    10000,
    NULL,
    1,
    NULL,
    0
  );

  // UART Task
  xTaskCreatePinnedToCore(
    serialTask,
    "Serial_Task",
    10000,
    NULL,
    1,
    NULL,
    1
  );
}

void loop() {

}
