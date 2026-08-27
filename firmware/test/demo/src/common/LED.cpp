#include "LED.h"
#include "PinMap.h"
#include <Arduino.h>

void LED_Init() {
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  LED_AllOff();
}

void LED_AllOff() {
  digitalWrite(PIN_LED_R, LOW);
  digitalWrite(PIN_LED_G, LOW);
  digitalWrite(PIN_LED_B, LOW);
}

void LED_Set(LedState state) {
  LED_AllOff();
  if (state == LEDSTATE_IDLE || state == LEDSTATE_SUCCESS) {
    digitalWrite(PIN_LED_G, HIGH);
  } else if (state == LEDSTATE_PROCESSING) {
    digitalWrite(PIN_LED_B, HIGH);
  } else if (state == LEDSTATE_ERROR) {
    digitalWrite(PIN_LED_R, HIGH);
  }
}

void LED_BlinkSuccess() {
  for (uint8_t i = 0; i < 2; i++) {
    LED_Set(LEDSTATE_SUCCESS);
    delay(120);
    LED_AllOff();
    delay(120);
  }
  LED_Set(LEDSTATE_IDLE);
}

void LED_BlinkError() {
  for (uint8_t i = 0; i < 3; i++) {
    LED_Set(LEDSTATE_ERROR);
    delay(140);
    LED_AllOff();
    delay(140);
  }
}
