#include "LCD.h"
#include "Config.h"
#include <Wire.h>

LiquidCrystal_I2C lcdWeight(LCD_WEIGHT_ADDR, LCD_COLS, LCD_ROWS);
LiquidCrystal_I2C lcdStatus(LCD_STATUS_ADDR, LCD_COLS, LCD_ROWS);

static String fitLine(const String& line) {
  return line.substring(0, LCD_COLS);
}

void LCD_Init(LiquidCrystal_I2C& weight, LiquidCrystal_I2C& status) {
  Wire.begin();
  weight.init();
  weight.backlight();
  status.init();
  status.backlight();
}

void LCD_Msg(LiquidCrystal_I2C& lcd, const String& line1, const String& line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(fitLine(line1));
  lcd.setCursor(0, 1);
  lcd.print(fitLine(line2));
}

void LCD_ClearAll() {
  lcdWeight.clear();
  lcdStatus.clear();
}

void LCD_ShowBoot() {
  LCD_Msg(lcdWeight, "SMART SCALE", "BOOTING...");
  LCD_Msg(lcdStatus, "PCB POWER OK", "INIT CHECK...");
}

void LCD_ShowReady() {
  LCD_Msg(lcdWeight, "L:0.0 R:0.0", "W: 0g");
  LCD_Msg(lcdStatus, "T:Tare(L:Save)", "C:Cal S:Send");
}

void LCD_ShowWeight(float wL, float wR, float total) {
  long totalRounded = total >= 0 ? long(total + 0.5f) : long(total - 0.5f);
  String line1 = String("L:") + String(wL, 1) + " R:" + String(wR, 1);
  String line2 = String("TOT:") + String(totalRounded) + "g";
  LCD_Msg(lcdWeight, line1, line2);
}

void LCD_ShowStatus(const String& line1, const String& line2) {
  LCD_Msg(lcdStatus, line1, line2);
}

void LCD_ShowAiResult(const AiResult& result) {
  LCD_Msg(lcdStatus, AiResult_Line1(result), AiResult_Line2(result));
}
