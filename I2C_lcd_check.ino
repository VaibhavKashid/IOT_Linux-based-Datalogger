//Simple Arduino code for checking I2C module works properly
// Connect SDA to A4 pin, SCL to A5 pin of Arduino Uno

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

// Set the LCD address to 0x27(or 0X3F) for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() 
{
  Serial.begin(9600);
  lcd.backlight();     // Turn on the backlight
  lcd.begin(16, 2);
}

void loop() 
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("I2C LCD Started");
  lcd.setCursor(1, 1);
  lcd.print("Row 2");
  Serial.println("I2C started");
  delay(1000);
}
