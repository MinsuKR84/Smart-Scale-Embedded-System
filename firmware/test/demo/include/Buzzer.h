#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>

void Buzzer_Init();
void Buzzer_Beep(uint16_t duration);
void Buzzer_Click();
void Buzzer_Success();
void Buzzer_Error();

#endif
