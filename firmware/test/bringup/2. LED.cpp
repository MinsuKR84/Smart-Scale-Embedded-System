// LED R/G/B 순차 점등
#include <Arduino.h>

#define LED_R 7
#define LED_G 6
#define LED_B 5

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
}

void loop() {
  digitalWrite(LED_R, HIGH);
  delay(300);
  digitalWrite(LED_R, LOW);
  delay(300);

  digitalWrite(LED_G, HIGH);
  delay(300);
  digitalWrite(LED_G, LOW);
  delay(300);

  digitalWrite(LED_B, HIGH);
  delay(300);
  digitalWrite(LED_B, LOW);
  delay(300);
}