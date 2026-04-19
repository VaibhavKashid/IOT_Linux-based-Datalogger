// Using Arduino as simulator for 4 sensors - S1,S2,S3 and S4
// This sends data over UART to ESP32 (UART0) in format - S1,233.91,29.89,0.94,49.61 (Sensor number, Voltage, Current, PF, Temp)
// Similar for sensors S2,S3 and S4. 

void setup() 
{
  Serial.begin(9600); //Initialize serial communication, in our case, UART
  randomSeed(analogRead(0)); // Seed for randomness
}

float randomFloat(float minVal, float maxVal) 
{
  return minVal + ((float)random(0, 10000) / 10000.0) * (maxVal - minVal);   //Generate random integers within range and convert to float
}

void loop() 
{

  for (int i = 1; i <= 4; i++)     // For 4 sensors
  {      

    float voltage = randomFloat(230.00, 250.99); //Voltage range
    float current = randomFloat(20.00, 40.99);  //Current range
    float pf      = randomFloat(0.85, 0.99); //PF range
    float temp    = randomFloat(20.00, 55.00); //Temp range

    // Format output (CSV style for easy parsing)
    Serial.print("S"); Serial.print(i); //Sensor number
    Serial.print(",");

    Serial.print(voltage, 2); Serial.print(","); // Here 2 is number of decimals
    Serial.print(current, 2); Serial.print(",");
    Serial.print(pf, 2); Serial.print(",");
    Serial.print(temp, 2);

    Serial.println();
  }

  //Serial.println("----"); // Frame separator

  delay(2000);
}
