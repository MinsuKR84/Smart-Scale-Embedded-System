// LCD 0x27 0x3F확인
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd1(0x27, 16, 2);
LiquidCrystal_I2C lcd2(0x3F, 16, 2);

void setup() {
  Wire.begin();

  lcd1.init();
  lcd1.backlight();

  lcd1.setCursor(0, 0);
  lcd1.print("PCB TEST OK");

  lcd1.setCursor(0, 1);
  lcd1.print("LCD OK 0x27");

  lcd2.init();
  lcd2.backlight();

  lcd2.setCursor(0, 0);
  lcd2.print("PCB TEST OK");

  lcd2.setCursor(0, 1);
  lcd2.print("LCD OK 0x3F");
}

void loop() {
}