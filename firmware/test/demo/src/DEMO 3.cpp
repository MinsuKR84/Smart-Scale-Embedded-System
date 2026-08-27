#include "DemoTareAutoZero.h"
#include "AutoZero.h"
#include "Button.h"
#include "Buzzer.h"
#include "Config.h"
#include "DualScale.h"
#include "EepromStore.h"
#include "LCD.h"
#include "LED.h"

static AutoZeroState autoZeroState;

static void showWeightValue(float wL, float wR, float total) {
  long totalRounded = total >= 0 ? long(total + 0.5f) : long(total - 0.5f);
  String totalText = totalRounded == 0 ? "00" : String(totalRounded);
  LCD_Msg(lcdWeight,
          String("L:") + String(wL, 1) + "g R:" + String(wR, 1) + "g",
          String("TOT:") + totalText + "g");
}

static void showHxReadyStatus() {
  bool leftReady = gScale.isLeftReady();
  bool rightReady = gScale.isRightReady();
  if (!leftReady && !rightReady) {
    LCD_ShowStatus("HX711 NOT READY", "L:NO R:NO");
  } else if (!leftReady) {
    LCD_ShowStatus("HX711 NOT READY", "L:NO R:OK");
  } else {
    LCD_ShowStatus("HX711 NOT READY", "L:OK R:NO");
  }
}

static bool DemoTareAutoZero_InitSequence() {
  LCD_Msg(lcdWeight, "SMART SCALE", "INIT...");
  LED_Set(LEDSTATE_PROCESSING);
  LCD_ShowStatus("POWER CHECK", "Init...");
  delay(1200);

  LED_Set(LEDSTATE_IDLE);
  LCD_ShowStatus("POWER CHECK", "Init OK");
  delay(1200);

  LED_Set(LEDSTATE_PROCESSING);
  LCD_ShowStatus("HX711 CHECK", "WAIT READY");
  if (!gScale.waitReady(3000)) {
    LED_Set(LEDSTATE_ERROR);
    showHxReadyStatus();
    return false;
  }

  LED_Set(LEDSTATE_IDLE);
  LCD_ShowStatus("HX711 CHECK", "L/R OK");
  delay(1200);

  AutoZero_Init(autoZeroState, 0);
  showWeightValue(0.0f, 0.0f, 0.0f);
  LCD_ShowStatus("TARE/AUTOZERO", "PRESS TARE");
  return true;
}

void DemoTareAutoZero_Run() {
  static bool initialized = false;
  static unsigned long lastUpdate = 0;
  static bool autoZeroUiActive = false;

  if (!initialized) {
    initialized = DemoTareAutoZero_InitSequence();
    return;
  }

  ButtonEvent tare = Button_ReadTare();
  if (tare == BTN_SHORT || tare == BTN_LONG) {
    Buzzer_Click();
    LED_Set(LEDSTATE_PROCESSING);
    LCD_ShowStatus("TARE", tare == BTN_LONG ? "Saving offset" : "Zeroing...");
    gScale.tare(DEMO_TARE_AVG_SAMPLES);
    if (tare == BTN_LONG) {
      EepromStore::SaveAll(gScale);
    }
    delay(700);
    AutoZero_Init(autoZeroState, 0);
    showWeightValue(0.0f, 0.0f, 0.0f);
    LCD_ShowStatus("TARE DONE", "W:0.0g");
    Buzzer_Success();
    LED_BlinkSuccess();
    autoZeroUiActive = true;
    autoZeroState.uiReturnMs = millis() + AUTO_ZERO_UI_RETURN_MS;
  }

  if (millis() - lastUpdate >= DEMO_TARE_UI_UPDATE_MS) {
    lastUpdate = millis();
    if (!gScale.waitReady(250)) {
      return;
    }

    long netL = 0;
    long netR = 0;
    bool ok = gScale.measureNetLR(DEMO_TARE_LIVE_SAMPLES, netL, netR);
    ScaleReading r;
    r.wL = float(netL) * gScale.factorL;
    r.wR = float(netR) * gScale.factorR;
    r.total = gScale.a_coef * r.wL + gScale.b_coef * r.wR;
    r.valid = ok;

    if (r.valid) {
      showWeightValue(r.wL, r.wR, r.total);
      LED_Set(LEDSTATE_IDLE);
      AutoZero_Update(autoZeroState, gScale, int(r.total), &lcdStatus, !autoZeroUiActive, DEMO_TARE_AVG_SAMPLES);
      if (autoZeroState.justFired) {
        LCD_ShowStatus("AUTO ZERO OK", "W:0.0g");
        autoZeroUiActive = true;
        autoZeroState.uiReturnMs = millis() + AUTO_ZERO_UI_RETURN_MS;
        Buzzer_Click();
      }
    } else {
      showHxReadyStatus();
      LED_Set(LEDSTATE_ERROR);
    }
  }

  if (AutoZero_UiReturnDue(autoZeroState)) {
    AutoZero_ClearUiReturn(autoZeroState);
    autoZeroUiActive = false;
    LCD_ShowStatus("TARE/AUTOZERO", "PRESS TARE");
  }
}
