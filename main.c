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
const char* mqtt_topic = "vaibhav/esp32/data";

WiFiClient espClient;
PubSubClient client(espClient);

QueueHandle_t mqttQueue;

// ====================================================
// Queue Structure
// ====================================================

typedef struct
{
    char message[600];
} mqttMessage;

// ====================================================
// LCD Storage
// ====================================================

struct MeterData
{
    float v1;
    float i1;
    float freq;
};

MeterData meter[5];

// ====================================================
// WiFi
// ====================================================

void setup_wifi()
{
    Serial.println("Connecting WiFi");

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        vTaskDelay(500 / portTICK_PERIOD_MS);
        Serial.print(".");
    }

    Serial.println();
    Serial.println("WiFi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

// ====================================================
// MQTT Reconnect
// ====================================================

void reconnect()
{
    while (!client.connected())
    {
        Serial.print("Connecting MQTT...");

        if (client.connect("ESP32_Client"))
        {
            Serial.println("Connected");
        }
        else
        {
            Serial.print("Failed rc=");
            Serial.println(client.state());

            vTaskDelay(2000 / portTICK_PERIOD_MS);
        }
    }
}

// ====================================================
// MQTT Task - Core 0
// ====================================================

void mqttTask(void * parameter)
{
    mqttMessage rxMsg;

    setup_wifi();

    client.setServer(mqtt_server, 1883);

    // Increase MQTT packet size
    client.setBufferSize(1024);

    while (1)
    {
        if (!client.connected())
        {
            reconnect();
        }

        client.loop();

        if (xQueueReceive(mqttQueue, &rxMsg, 10) == pdPASS)
        {
            Serial.print("MQTT Connected: ");
            Serial.println(client.connected());

            Serial.print("Payload Length: ");
            Serial.println(strlen(rxMsg.message));

            bool result = client.publish(
                mqtt_topic,
                rxMsg.message
            );

            Serial.print("Publish Result: ");
            Serial.println(result ? "SUCCESS" : "FAILED");

            if (!result)
            {
                Serial.print("MQTT State: ");
                Serial.println(client.state());
            }

            Serial.println("Published Payload:");
            Serial.println(rxMsg.message);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// ====================================================
// UART + LCD Task - Core 1
// ====================================================

void serialTask(void * parameter)
{
    char inputLine[250];
    int index = 0;

    uint32_t lastLCDUpdate = 0;
    uint8_t lcdMeter = 1;

    while (1)
    {
        while (mySerial.available())
        {
            char c = mySerial.read();

            if (c == '\r')
                continue;

            if (c == '\n')
            {
                inputLine[index] = '\0';

                Serial.print("UART RX: ");
                Serial.println(inputLine);

                if (strlen(inputLine) > 0)
                {
                    int devType;

                    float dc_v, dc_i;
                    float v1, v2, v3;
                    float v12, v23, v31;
                    float i1, i2, i3;
                    float pf1, pf2, pf3;
                    float var1, var2, var3;
                    float freq;

                    int fields = sscanf(
                        inputLine,
                        "%d,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f",
                        &devType,
                        &dc_v,
                        &dc_i,
                        &v1,
                        &v2,
                        &v3,
                        &v12,
                        &v23,
                        &v31,
                        &i1,
                        &i2,
                        &i3,
                        &pf1,
                        &pf2,
                        &pf3,
                        &var1,
                        &var2,
                        &var3,
                        &freq
                    );

                    Serial.print("Fields Parsed = ");
                    Serial.println(fields);

                    if (fields == 19)
                    {
                        // Store data for LCD
                        if (devType >= 1 && devType <= 4)
                        {
                            meter[devType].v1 = v1;
                            meter[devType].i1 = i1;
                            meter[devType].freq = freq;
                        }

                        char jsonPayload[600];

                        snprintf(
                            jsonPayload,
                            sizeof(jsonPayload),

                            "{"
                            "\"devType\":%d,"
                            "\"dc_v\":%.2f,"
                            "\"dc_i\":%.2f,"
                            "\"v1\":%.2f,"
                            "\"v2\":%.2f,"
                            "\"v3\":%.2f,"
                            "\"v12\":%.2f,"
                            "\"v23\":%.2f,"
                            "\"v31\":%.2f,"
                            "\"i1\":%.2f,"
                            "\"i2\":%.2f,"
                            "\"i3\":%.2f,"
                            "\"pf1\":%.2f,"
                            "\"pf2\":%.2f,"
                            "\"pf3\":%.2f,"
                            "\"var1\":%.2f,"
                            "\"var2\":%.2f,"
                            "\"var3\":%.2f,"
                            "\"freq\":%.2f"
                            "}",

                            devType,
                            dc_v,
                            dc_i,
                            v1,
                            v2,
                            v3,
                            v12,
                            v23,
                            v31,
                            i1,
                            i2,
                            i3,
                            pf1,
                            pf2,
                            pf3,
                            var1,
                            var2,
                            var3,
                            freq
                        );

                        mqttMessage txMsg;

                        strcpy(
                            txMsg.message,
                            jsonPayload
                        );

                        Serial.println("Queue Send");

                        xQueueSend(
                            mqttQueue,
                            &txMsg,
                            portMAX_DELAY
                        );
                    }
                    else
                    {
                        Serial.println("ERROR: Expected 19 fields");
                    }
                }

                index = 0;
            }
            else
            {
                if (index < sizeof(inputLine) - 1)
                {
                    inputLine[index++] = c;
                }
            }
        }

        // ====================================================
        // LCD UPDATE EVERY 2 SECONDS
        // ====================================================

        if (millis() - lastLCDUpdate >= 2000)
        {
            lastLCDUpdate = millis();

            lcd.clear();

            lcd.setCursor(0, 0);
            lcd.print("M:");
            lcd.print(lcdMeter);
            lcd.print(" V:");
            lcd.print(meter[lcdMeter].v1, 0);

            lcd.setCursor(0, 1);
            lcd.print("I:");
            lcd.print(meter[lcdMeter].i1, 1);

            lcdMeter++;

            if (lcdMeter > 4)
            {
                lcdMeter = 1;
            }
        }

        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

// ====================================================
// Setup
// ====================================================

void setup()
{
    // UART0 -> Serial Monitor
    Serial.begin(115200);

    // UART2 -> Arduino
    mySerial.begin(
        115200,
        SERIAL_8N1,
        RXD2,
        TXD2
    );

    Wire.begin(21, 22);

    lcd.begin(16, 2);
    lcd.backlight();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("System Starting");

    mqttQueue = xQueueCreate(
        10,
        sizeof(mqttMessage)
    );

    // MQTT Task on Core 0
    xTaskCreatePinnedToCore(
        mqttTask,
        "MQTT_Task",
        10000,
        NULL,
        1,
        NULL,
        0
    );

    // UART + LCD Task on Core 1
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

// ====================================================
// Loop
// ====================================================

void loop()
{
}
