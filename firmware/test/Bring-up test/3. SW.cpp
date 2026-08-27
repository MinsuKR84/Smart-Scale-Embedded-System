// SEND/TARE/CAL 스위치 확인
#include <Arduino.h>

#define SW_SEND 2
#define SW_TARE A2
#define SW_CAL  A3

#define LED_R 7
#define LED_G 6
#define LED_B 5

void setup() {
  pinMode(SW_SEND, INPUT);
  pinMode(SW_TARE, INPUT);
  pinMode(SW_CAL, INPUT);

  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);
}

void loop() {
  // 버튼은 누르면 LOW가 되는 구조로 테스트
  bool sendPressed = digitalRead(SW_SEND) == LOW;
  bool tarePressed = digitalRead(SW_TARE) == LOW;
  bool calPressed  = digitalRead(SW_CAL)  == LOW;

  digitalWrite(LED_R, sendPressed ? HIGH : LOW);
  digitalWrite(LED_G, tarePressed ? HIGH : LOW);
  digitalWrite(LED_B, calPressed  ? HIGH : LOW);
}
