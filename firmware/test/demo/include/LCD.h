#ifndef LCD_MODULE_H
#define LCD_MODULE_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "AiResult.h"

extern LiquidCrystal_I2C lcdWeight;
extern LiquidCrystal_I2C lcdStatus;

void LCD_Init(LiquidCrystal_I2C& weight, LiquidCrystal_I2C& status);
void LCD_Msg(LiquidCrystal_I2C& lcd, const String& line1, const String& line2);
void LCD_ClearAll();
void LCD_ShowBoot();
void LCD_ShowReady();
void LCD_ShowWeight(float wL, float wR, float total);
void LCD_ShowStatus(const String& line1, const String& line2);
void LCD_ShowAiResult(const AiResult& result);

#endif
