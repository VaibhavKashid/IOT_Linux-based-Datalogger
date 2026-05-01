//Note: This code is for Arduino interfaced with 16X2 LCD with push button connected between digital pin 4 and GND.
// Code for LCD UI --> Changes state of inverter (ON/OFF) when we press pushbutton. Also backlight turns ON/OFF accordingly. 
#include<Wire.h>
#include<LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
bool state = 1;   //Stores state of button
#define BTN 4    

void setup() 
{
  Serial.begin(9600);
  pinMode(BTN, INPUT_PULLUP); //No external pull-up resistors required
  lcd.init();
  lcd.setCursor(0,0);
  lcd.clear();
  lcd.print("Inverter: ON");
  lcd.setCursor(10, 1);
  lcd.print("OFF");

}

void loop() {
  if(digitalRead(BTN)==false)      //Pin is active high due to internal pull-up resistor
  {
    state = !state;    //Toggles state 
    if(state==1)
    {
      lcd.backlight();
      lcd.setCursor(14,0);
      lcd.print("<");
      lcd.setCursor(14, 1);
      lcd.print("  ");
      Serial.println("Button True ");
    }
    else
    {
      lcd.noBacklight();
      lcd.setCursor(14, 1);
      lcd.print("<");
      lcd.setCursor(14, 0);
      lcd.print("  ");
      Serial.println("Button False");
    }
    delay(500);
  }
}
