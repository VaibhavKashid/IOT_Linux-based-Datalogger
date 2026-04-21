//Connect I2C module to 16X2 display
//Connect SCL pin to D22 and SCL pin to D21
//Also connect Vcc to Vin and Gnd to Gnd
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Change 0x27 to 0x3F if needed
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() 
{
  Wire.begin(21, 22);      // SDA, SCL for ESP32
  lcd.begin(16, 2);        // Initialize LCD
  lcd.backlight();         // Turn ON backlight
}

void loop() 
{
  lcd.setCursor(0, 0);
  lcd.print("Hello ESP32");

  lcd.setCursor(1, 1);
  lcd.print("I2C LCD Ready");
}
