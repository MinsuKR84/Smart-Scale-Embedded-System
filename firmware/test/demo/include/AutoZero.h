#ifndef AUTO_ZERO_H
#define AUTO_ZERO_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include "DualScale.h"

struct AutoZeroState {
  unsigned long lastAutoZeroMs;
  unsigned long lastStableChkMs;
  bool stableCached;
  int lastAzG;
  unsigned long lastMoveMs;
  unsigned long uiReturnMs;
  bool justFired;
};

void AutoZero_Init();
void AutoZero_Init(AutoZeroState& st, int initialAzG);
void AutoZero_Update();
void AutoZero_Update(AutoZeroState& st,
                     DualScale& scale,
                     int az_g,
                     LiquidCrystal_I2C* lcdStatus,
                     bool allowUiMsg,
                     uint8_t tareAvgSamples);
bool AutoZero_UiReturnDue();
bool AutoZero_UiReturnDue(const AutoZeroState& st);
void AutoZero_ClearUiReturn();
void AutoZero_ClearUiReturn(AutoZeroState& st);

#endif
