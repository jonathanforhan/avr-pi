#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
static const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
static LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void setup() {
    lcd.begin(16, 2);
    lcd.print("Hello, avr-pi!  ");
}

void loop() {
    lcd.setCursor(0, 1);
    lcd.print("run avr on pi!  ");
    delay(500);
    lcd.setCursor(0, 1);
    lcd.print("emulators rock! ");
    delay(500);
    lcd.setCursor(0, 1);
    lcd.print("made with <3    ");
    delay(500);
}
