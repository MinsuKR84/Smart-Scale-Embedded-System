#include "Buzzer.h"
#include "PinMap.h"

static const uint16_t BUZZER_FREQ_HZ = 2048;

void Buzzer_Init() {
  pinMode(PIN_BUZZER, OUTPUT);
  noTone(PIN_BUZZER);
}

void Buzzer_Beep(uint16_t duration) {
  tone(PIN_BUZZER, BUZZER_FREQ_HZ);
  delay(duration);
  noTone(PIN_BUZZER);
}

void Buzzer_Click() {
  Buzzer_Beep(45);
}

void Buzzer_Success() {
  for (uint8_t i = 0; i < 2; i++) {
    Buzzer_Beep(60);
    delay(80);
  }
}

void Buzzer_Error() {
  for (uint8_t i = 0; i < 3; i++) {
    Buzzer_Beep(80);
    delay(90);
  }
}
