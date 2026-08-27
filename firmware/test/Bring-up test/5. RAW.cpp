// HX711 L/R raw 확인용
// PCB 실제 좌/우 기준으로 LCD 표시 반전

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

#define HX_L_DT  3
#define HX_L_SCK 4

#define HX_R_DT  A0
#define HX_R_SCK A1

#define LED_R 7
#define LED_G 6
#define LED_B 5

LiquidCrystal_I2C lcd(0x27, 16, 2);

HX711 scaleL;
HX711 scaleR;

void setup() {

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  Wire.begin();

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("HX711 TEST");

  lcd.setCursor(0, 1);
  lcd.print("Init...");

  delay(1000);

  // 핀 연결은 기존 그대로
  scaleL.begin(HX_L_DT, HX_L_SCK);
  scaleR.begin(HX_R_DT, HX_R_SCK);
}

void loop() {

  // PCB에서 실제 좌/우가 반대로 연결되어 있으므로
  // scaleR을 실제 LEFT,
  // scaleL을 실제 RIGHT로 사용

  bool leftReady  = scaleR.is_ready();  // 실제 LEFT
  bool rightReady = scaleL.is_ready();  // 실제 RIGHT

  long leftRaw = 0;
  long rightRaw = 0;

  // 실제 LEFT
  if (leftReady) {
    leftRaw = scaleR.read_average(10);
  }

  // 실제 RIGHT
  if (rightReady) {
    rightRaw = scaleL.read_average(10);
  }

  lcd.clear();

  // 실제 LEFT 표시
  lcd.setCursor(0, 0);
  lcd.print("L:");

  if (leftReady) {
    lcd.print(leftRaw);
  } else {
    lcd.print("NO");
  }

  // 실제 RIGHT 표시
  lcd.setCursor(0, 1);
  lcd.print("R:");

  if (rightReady) {
    lcd.print(rightRaw);
  } else {
    lcd.print("NO");
  }

  // LED도 실제 좌/우 기준으로 표시
  digitalWrite(LED_R, leftReady ? HIGH : LOW);
  digitalWrite(LED_G, rightReady ? HIGH : LOW);

  delay(500);
}