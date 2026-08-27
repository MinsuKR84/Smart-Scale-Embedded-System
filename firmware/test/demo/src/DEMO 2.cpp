#include "DemoDualScale.h"
#include "Buzzer.h"
#include "Config.h"
#include "DualScale.h"
#include "LCD.h"
#include "LED.h"
#include <math.h>
#include <stdlib.h>

static void showWeightValue(float wL, float wR, float total) {
  long totalRounded = total >= 0 ? long(total + 0.5f) : long(total - 0.5f);
  String totalText = totalRounded == 0 ? "00" : String(totalRounded);
  LCD_Msg(lcdWeight,
          String("L:") + String(wL, 1) + "g R:" + String(wR, 1) + "g",
          String("TOT:") + totalText + "g");
}

static void showCountdown(const String& line1, uint8_t seconds) {
  for (int8_t i = seconds; i > 0; i--) {
    LCD_ShowStatus(line1, String("WAIT ") + String(i) + " sec");
    delay(1000);
  }
}

static void showNetRaw(const String& title, long netL, long netR) {
  LCD_ShowStatus(title, String("NL:") + String(netL));
  delay(1800);
  LCD_ShowStatus(title, String("NR:") + String(netR));
  delay(1800);
}

static bool factorInRange(float factor) {
  float absFactor = fabs(factor);
  return absFactor >= 0.000001f && absFactor <= 0.100000f;
}

static bool detTooSmall(float det, float ref) {
  if (ref <= 1.0f) {
    return true;
  }
  return fabs(det) < (0.000001f * ref);
}

static bool calculateTwoPointFactors(long nLLeft, long nRLeft, long nLRight, long nRRight) {
  float det = float(nLLeft) * float(nRRight) - float(nLRight) * float(nRLeft);
  float ref = fabs(float(nLLeft) * float(nRRight)) + fabs(float(nLRight) * float(nRLeft));

  float fL = 0.0f;
  float fR = 0.0f;
  bool useFallback = detTooSmall(det, ref);

  if (!useFallback) {
    fL = DEMO_CAL_WEIGHT_G * (float(nRRight) - float(nRLeft)) / det;
    fR = DEMO_CAL_WEIGHT_G * (float(nLLeft) - float(nLRight)) / det;
    if (!isfinite(fL) || !isfinite(fR) || !factorInRange(fL) || !factorInRange(fR)) {
      useFallback = true;
    }
  }

  if (useFallback) {
    LCD_ShowStatus("det small", "fallback used");
    delay(1500);
    if (labs(nLLeft) < DEMO_CAL_MIN_NET || labs(nRRight) < DEMO_CAL_MIN_NET) {
      LCD_ShowStatus("Fallback FAIL", "net too small");
      delay(2000);
      return false;
    }
    fL = DEMO_CAL_WEIGHT_G / float(nLLeft);
    fR = DEMO_CAL_WEIGHT_G / float(nRRight);
  }

  if (!isfinite(fL) || !isfinite(fR) || !factorInRange(fL) || !factorInRange(fR)) {
    LCD_ShowStatus("FACTOR RANGE", "Retry");
    delay(2000);
    return false;
  }

  gScale.factorL = fL;
  gScale.factorR = fR;
  return true;
}

static void showCalibrationCheck(const String& title, long netL, long netR) {
  float wL = float(netL) * gScale.factorL;
  float wR = float(netR) * gScale.factorR;
  showWeightValue(wL, wR, wL + wR);
  LCD_ShowStatus(title, String("SUM:") + String(wL + wR, 1) + "g");
  delay(2200);
}

static bool showHxReadyStatus() {
  bool leftReady = gScale.isLeftReady();
  bool rightReady = gScale.isRightReady();
  if (!leftReady && !rightReady) {
    LCD_ShowStatus("HX711 NOT READY", "L:NO R:NO");
  } else if (!leftReady) {
    LCD_ShowStatus("HX711 NOT READY", "L:NO R:OK");
  } else {
    LCD_ShowStatus("HX711 NOT READY", "L:OK R:NO");
  }
  return leftReady && rightReady;
}

static bool DemoDualScale_InitSequence() {
  LCD_Msg(lcdWeight, "SMART SCALE", "INIT...");
  LED_Set(LEDSTATE_PROCESSING);
  LCD_ShowStatus("POWER CHECK", "Init...");
  delay(1500);

  LED_Set(LEDSTATE_IDLE);
  LCD_ShowStatus("POWER CHECK", "Init OK");
  delay(1500);

  LED_Set(LEDSTATE_PROCESSING);
  LCD_ShowStatus("HX711 CHECK", "WAIT READY");
  if (!gScale.waitReady(3000)) {
    LED_Set(LEDSTATE_ERROR);
    showHxReadyStatus();
    delay(1500);
    return false;
  }

  LED_Set(LEDSTATE_IDLE);
  LCD_ShowStatus("HX711 CHECK", "L/R OK");
  delay(1500);

  showWeightValue(0.0f, 0.0f, 0.0f);
  LED_Set(LEDSTATE_PROCESSING);
  LCD_ShowStatus("ZERO SET", "No load...");
  showCountdown("REMOVE LOAD", 5);
  gScale.tare(DEMO_CAL_SAMPLES);

  showWeightValue(0.0f, 0.0f, 0.0f);
  LCD_ShowStatus("PUT 155g LEFT", "LEFT ONLY");
  showCountdown("PUT 155g LEFT", 8);

  long nLLeft = 0;
  long nRLeft = 0;
  LCD_ShowStatus("MEASURE LEFT", "DO NOT TOUCH");
  delay(1200);
  if (!gScale.measureNetLR(DEMO_CAL_SAMPLES, nLLeft, nRLeft)) {
    LED_Set(LEDSTATE_ERROR);
    showHxReadyStatus();
    delay(1500);
    return false;
  }
  showNetRaw("LEFT RAW", nLLeft, nRLeft);

  LCD_ShowStatus("REMOVE LEFT", "WAIT STABLE");
  showCountdown("REMOVE LEFT", 5);

  LCD_ShowStatus("PUT 155g RIGHT", "RIGHT ONLY");
  showCountdown("PUT 155g RIGHT", 8);

  long nLRight = 0;
  long nRRight = 0;
  LCD_ShowStatus("MEASURE RIGHT", "DO NOT TOUCH");
  delay(1200);
  if (!gScale.measureNetLR(DEMO_CAL_SAMPLES, nLRight, nRRight)) {
    LED_Set(LEDSTATE_ERROR);
    showHxReadyStatus();
    delay(1500);
    return false;
  }
  showNetRaw("RIGHT RAW", nLRight, nRRight);

  LCD_ShowStatus("CAL FACTOR", "Calculating...");
  delay(1200);
  if (!calculateTwoPointFactors(nLLeft, nRLeft, nLRight, nRRight)) {
    LED_Set(LEDSTATE_ERROR);
    return false;
  }

  showCalibrationCheck("LEFT CHECK", nLLeft, nRLeft);
  showCalibrationCheck("RIGHT CHECK", nLRight, nRRight);

  LED_Set(LEDSTATE_IDLE);
  LCD_ShowStatus("FACTOR L", String(gScale.factorL, 6));
  delay(2200);
  LCD_ShowStatus("FACTOR R", String(gScale.factorR, 6));
  delay(2200);

  LCD_ShowStatus("REMOVE WEIGHT", "Zero again");
  delay(5000);
  gScale.tare(DEMO_CAL_SAMPLES);

  showWeightValue(0.0f, 0.0f, 0.0f);
  LCD_ShowStatus("DUAL SCALE", "MEASURING...");
  Buzzer_Success();
  return true;
}

void DemoDualScale_Run() {
  static bool initialized = false;
  static unsigned long lastUpdate = 0;

  if (!initialized) {
    initialized = DemoDualScale_InitSequence();
    return;
  }

  if (millis() - lastUpdate < 500) {
    return;
  }
  lastUpdate = millis();

  long netL = 0;
  long netR = 0;
  if (!gScale.measureNetLR(HX711_READ_SAMPLES, netL, netR)) {
    LED_Set(LEDSTATE_ERROR);
    showHxReadyStatus();
    return;
  }

  ScaleReading r;
  r.wL = netL * gScale.factorL;
  r.wR = netR * gScale.factorR;
  r.total = gScale.a_coef * r.wL + gScale.b_coef * r.wR;
  r.valid = true;

  LED_Set(LEDSTATE_IDLE);
  showWeightValue(r.wL, r.wR, r.total);
  LCD_ShowStatus("DUAL SCALE", "MEASURING...");

  Serial.print(F("raw/net L="));
  long rawL = 0;
  long rawR = 0;
  gScale.lastRaw(rawL, rawR);
  Serial.print(rawL);
  Serial.print(F("/"));
  Serial.print(netL);
  Serial.print(F(" R="));
  Serial.print(rawR);
  Serial.print(F("/"));
  Serial.print(netR);
  Serial.print(F(" weight L="));
  Serial.print(r.wL, 1);
  Serial.print(F(" R="));
  Serial.print(r.wR, 1);
  Serial.print(F(" TOT="));
  Serial.println(r.total, 1);
}
