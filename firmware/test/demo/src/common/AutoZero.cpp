#include "AutoZero.h"
#include "Config.h"
#include "LCD.h"
#include <Arduino.h>

static AutoZeroState gAutoZeroState;

static bool nearZero(int g) {
  return abs(g) <= AUTO_ZERO_BAND_G;
}

void AutoZero_Init() {
  AutoZero_Init(gAutoZeroState, 0);
}

void AutoZero_Init(AutoZeroState& st, int initialAzG) {
  unsigned long now = millis();
  st.lastAutoZeroMs = now - AUTO_ZERO_MIN_INTERVAL_MS;
  st.lastStableChkMs = 0;
  st.stableCached = true;
  st.lastAzG = initialAzG;
  st.lastMoveMs = now;
  st.uiReturnMs = 0;
  st.justFired = false;
}

void AutoZero_Update() {
  ScaleReading r = gScale.readOnce(HX711_READ_SAMPLES);
  if (!r.valid) {
    return;
  }
  AutoZero_Update(gAutoZeroState, gScale, int(r.total), &lcdStatus, true, HX711_READ_SAMPLES);
}

void AutoZero_Update(AutoZeroState& st,
                     DualScale& scale,
                     int az_g,
                     LiquidCrystal_I2C* lcdStatusPtr,
                     bool allowUiMsg,
                     uint8_t tareAvgSamples) {
  const unsigned long now = millis();
  st.justFired = false;

  if (now - st.lastStableChkMs >= AUTO_ZERO_STABLE_CHECK_MS) {
    st.lastStableChkMs = now;
    // AutoZero_Update is called after a valid weight read in the demo loop.
    // Keep this non-blocking; near-zero and no-move filters are the stability gate.
    st.stableCached = true;
  }

  if (!nearZero(az_g)) {
    st.lastAzG = az_g;
    st.lastMoveMs = now;
    return;
  }

  if (abs(az_g - st.lastAzG) >= AUTO_ZERO_MOVE_THRESHOLD_G) {
    st.lastAzG = az_g;
    st.lastMoveMs = now;
  }

  const bool noMoveEnough = now - st.lastMoveMs >= AUTO_ZERO_NO_MOVE_MS;
  const bool coolDone = now - st.lastAutoZeroMs >= AUTO_ZERO_MIN_INTERVAL_MS;

  if (noMoveEnough && st.stableCached && coolDone) {
    if (lcdStatusPtr && allowUiMsg) {
      LCD_Msg(*lcdStatusPtr, F("AUTO ZERO"), F("Tare..."));
      st.uiReturnMs = now + AUTO_ZERO_UI_RETURN_MS;
    }

    scale.tare(tareAvgSamples);

    st.lastAutoZeroMs = now;
    st.lastAzG = 0;
    st.lastMoveMs = now;
    st.justFired = true;
  }
}

bool AutoZero_UiReturnDue() {
  return AutoZero_UiReturnDue(gAutoZeroState);
}

bool AutoZero_UiReturnDue(const AutoZeroState& st) {
  if (st.uiReturnMs == 0) {
    return false;
  }
  return long(millis() - st.uiReturnMs) >= 0;
}

void AutoZero_ClearUiReturn() {
  AutoZero_ClearUiReturn(gAutoZeroState);
}

void AutoZero_ClearUiReturn(AutoZeroState& st) {
  st.uiReturnMs = 0;
}
