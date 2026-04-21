//Test code for Switch (only for ESP32)
#define SWITCH_PIN 4     // D4 pin (GPIO 4)
#define LED_PIN 2        // Built-in LED (usually GPIO 2)

void setup() {
  Serial.begin(9600);
  pinMode(SWITCH_PIN, INPUT_PULLUP);  // Enable internal pull-up
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  int switchState = digitalRead(SWITCH_PIN);

  if (switchState == LOW) {   // Button pressed
    digitalWrite(LED_PIN, HIGH);  // LED ON
    Serial.println("ON");
  } else {
    digitalWrite(LED_PIN, LOW);   // LED OFF
    Serial.println("OFF");
  }
}
