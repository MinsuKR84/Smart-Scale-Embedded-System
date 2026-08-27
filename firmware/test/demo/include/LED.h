#ifndef LED_MODULE_H
#define LED_MODULE_H

enum LedState {
  LEDSTATE_OFF,
  LEDSTATE_IDLE,
  LEDSTATE_PROCESSING,
  LEDSTATE_SUCCESS,
  LEDSTATE_ERROR
};

void LED_Init();
void LED_AllOff();
void LED_Set(LedState state);
void LED_BlinkSuccess();
void LED_BlinkError();

#endif
