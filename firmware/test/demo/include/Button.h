#ifndef BUTTON_H
#define BUTTON_H

enum ButtonEvent {
  BTN_NONE,
  BTN_SHORT,
  BTN_LONG
};

void Button_Init();
ButtonEvent Button_ReadTare();
ButtonEvent Button_ReadCal();
ButtonEvent Button_ReadSend();

#endif
