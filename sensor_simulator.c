//Code for Arduino Uno as a meter data simulator

void setup()
{
    Serial.begin(115200);
    randomSeed(analogRead(A0));
}

void loop()
{
    static uint8_t devType = 1;

    // DC Parameters
    float dc_v = random(1800, 2400) / 10.0;     // 180.0 - 240.0 V
    float dc_i = random(100, 800) / 10.0;       // 10.0 - 80.0 A

    // Phase Voltages
    float v1 = random(2200, 2400) / 10.0;     // 220.0 - 240.0 V
    float v2 = random(2200, 2400) / 10.0;
    float v3 = random(2200, 2400) / 10.0;

    // Line-to-Line Voltages
    float v12 = random(3800, 4200) / 10.0;    // 380.0 - 420.0 V
    float v23 = random(3800, 4200) / 10.0;
    float v31 = random(3800, 4200) / 10.0;

    // Phase Currents
    float i1 = random(500, 1000) / 10.0;        // 50.0 - 100.0 A
    float i2 = random(500, 1000) / 10.0;
    float i3 = random(500, 1000) / 10.0;

    // Power Factors
    float pf1 = random(85, 100) / 100.0;      // 0.85 - 0.99
    float pf2 = random(85, 100) / 100.0;
    float pf3 = random(85, 100) / 100.0;

    // Reactive Powers (VAR)
    float var1 = random(100, 500) / 10.0;     // 10.0 - 50.0 VAR
    float var2 = random(100, 500) / 10.0;
    float var3 = random(100, 500) / 10.0;

    // Frequency
    float freq = random(495, 505) / 10.0;     // 49.5 - 50.5 Hz

    // Transmit comma-separated data
    Serial.print(devType);
    Serial.print(",");

    Serial.print(dc_v, 2);
    Serial.print(",");

    Serial.print(dc_i, 2);
    Serial.print(",");

    Serial.print(v1, 2);
    Serial.print(",");

    Serial.print(v2, 2);
    Serial.print(",");

    Serial.print(v3, 2);
    Serial.print(",");

    Serial.print(v12, 2);
    Serial.print(",");

    Serial.print(v23, 2);
    Serial.print(",");

    Serial.print(v31, 2);
    Serial.print(",");

    Serial.print(i1, 2);
    Serial.print(",");

    Serial.print(i2, 2);
    Serial.print(",");

    Serial.print(i3, 2);
    Serial.print(",");

    Serial.print(pf1, 2);
    Serial.print(",");

    Serial.print(pf2, 2);
    Serial.print(",");

    Serial.print(pf3, 2);
    Serial.print(",");

    Serial.print(var1, 2);
    Serial.print(",");

    Serial.print(var2, 2);
    Serial.print(",");

    Serial.print(var3, 2);
    Serial.print(",");

    Serial.println(freq, 2);

    // Cycle through Meter IDs: 1 → 2 → 3 → 4 → 1 ...
    devType++;
    if (devType > 4)
    {
        devType = 1;
    }

    delay(1000);
}
