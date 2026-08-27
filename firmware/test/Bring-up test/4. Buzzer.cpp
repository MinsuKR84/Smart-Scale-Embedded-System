// D8 수동 부저 확인
#include <Arduino.h>

#define BUZZER 8   // PB0

void setup() {
  pinMode(BUZZER, OUTPUT);
}

void loop() {
  tone(BUZZER, 2048);
  delay(1000);

  noTone(BUZZER);
  delay(1000);
}