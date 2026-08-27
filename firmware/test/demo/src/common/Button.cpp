#include "Button.h"
#include "PinMap.h"
#include <Arduino.h>

static const uint32_t DEBOUNCE_MS = 5;
static const uint32_t LONGPRESS_MS = 2000;

enum {
  BTN_IDX_TARE = 0,
  BTN_IDX_CAL,
  BTN_IDX_SEND,
  BTN_COUNT
};

struct ButtonState {
  uint8_t pin;
  bool lastReading;
  bool stableState;
  uint32_t lastDebounceTime;
  uint32_t pressStartTime;
  bool longFired;
  bool shortFired;
};

static ButtonState buttons[BTN_COUNT] = {
  {PIN_SW_TARE, HIGH, HIGH, 0, 0, false, false},
  {PIN_SW_CAL, HIGH, HIGH, 0, 0, false, false},
  {PIN_SW_SEND, HIGH, HIGH, 0, 0, false, false}
};

void Button_Init() {
  for (uint8_t i = 0; i < BTN_COUNT; i++) {
    // Active LOW buttons. INPUT_PULLUP makes the input stable without external pull-up resistors.
    pinMode(buttons[i].pin, INPUT_PULLUP);

    bool now = digitalRead(buttons[i].pin);
    buttons[i].lastReading = now;
    buttons[i].stableState = now;
    buttons[i].lastDebounceTime = millis();
    buttons[i].pressStartTime = 0;
    buttons[i].longFired = false;
    buttons[i].shortFired = false;
  }
}

static ButtonEvent readButtonTare(ButtonState& button) {
  bool reading = digitalRead(button.pin);

  if (reading != button.lastReading) {
    button.lastDebounceTime = millis();
  }
  button.lastReading = reading;

  if (millis() - button.lastDebounceTime >= DEBOUNCE_MS) {
    if (reading != button.stableState) {
      button.stableState = reading;

      if (button.stableState == LOW) {
        button.pressStartTime = millis();
        button.longFired = false;
        if (!button.shortFired) {
          button.shortFired = true;
          return BTN_SHORT;
        }
      } else {
        button.shortFired = false;
      }
    }

    if (button.stableState == LOW && !button.longFired) {
      if (millis() - button.pressStartTime >= LONGPRESS_MS) {
        button.longFired = true;
        return BTN_LONG;
      }
    }
  }

  return BTN_NONE;
}

static ButtonEvent readButtonImmediate(ButtonState& button) {
  bool reading = digitalRead(button.pin);

  if (reading != button.lastReading) {
    button.lastDebounceTime = millis();
  }
  button.lastReading = reading;

  if (millis() - button.lastDebounceTime >= DEBOUNCE_MS) {
    if (reading != button.stableState) {
      button.stableState = reading;

      if (button.stableState == LOW) {
        if (!button.shortFired) {
          button.shortFired = true;
          return BTN_SHORT;
        }
      } else {
        button.shortFired = false;
      }
    }
  }

  return BTN_NONE;
}

ButtonEvent Button_ReadTare() {
  return readButtonTare(buttons[BTN_IDX_TARE]);
}

ButtonEvent Button_ReadCal() {
  return readButtonImmediate(buttons[BTN_IDX_CAL]);
}

ButtonEvent Button_ReadSend() {
  return readButtonImmediate(buttons[BTN_IDX_SEND]);
}
