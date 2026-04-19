#include <WiFi.h>
#include <PubSubClient.h>

//  WiFi Credentials
const char* ssid = "Vaibhav";
const char* password = "12345678";

//  MQTT Broker
const char* mqtt_server = "broker.hivemq.com";  // Public broker

WiFiClient espClient;
PubSubClient client(espClient);

String inputLine = "";

//  Connect to WiFi
void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

//Connect to MQTT
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");

    if (client.connect("ESP32_Client")) {
      Serial.println("Connected!");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying...");
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(9600);   // UART0 for receiving + debugging

  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //  Read UART0 data
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      inputLine.trim();

      if (inputLine.length() > 0) {
        Serial.print("Publishing: ");
        Serial.println(inputLine);

        //  Publish to MQTT
        client.publish("vaibhav/esp32/data", inputLine.c_str());
      }

      inputLine = "";
    } else {
      inputLine += c;
    }
  }
}
