#include "DemoSendAiResult.h"

#include "AiResult.h"
#include "AutoZero.h"
#include "Button.h"
#include "Buzzer.h"
#include "CommWiz550io.h"
#include "CommWizFi360.h"
#include "Config.h"
#include "DualScale.h"
#include "EepromStore.h"
#include "LCD.h"
#include "LED.h"

#include <math.h>

static const uint32_t NET_UI_SHOW_MS = 8000UL;
static const uint32_t AI_REPLY_TIMEOUT_MS = 60000UL;
static const uint32_t BUTTON_WEIGHT_IGNORE_MS = 500UL;
static const uint8_t BUTTON_RECOVERY_SAMPLES = 10;
static const uint8_t DISPLAY_SAMPLES = 1;
static const float ZERO_SNAP_G = 2.0f;
static const float DISPLAY_HOLD_BAND_G = 1.0f;

static AutoZeroState autoZeroState;
static uint32_t lastMeasureMs = 0;
static uint32_t netUiUntil = 0;
static uint32_t weightIgnoreUntil = 0;
static bool initialized = false;
static bool wifiReady = false;
static bool netUiOverlay = false;
static bool needWeightRecoveryAverage = false;
static bool displayTotalInitialized = false;
static int displayTotalInt = 0;

static void showReadyStatus() {
  LCD_ShowStatus("T:Tare(L:Save)", "S:Send AI");
}

static void showHxReadyStatus() {
  const bool leftReady = gScale.isLeftReady();
  const bool rightReady = gScale.isRightReady();

  if (!leftReady && !rightReady) {
    LCD_ShowStatus("HX711 NOT READY", "L:NO R:NO");
  } else if (!leftReady) {
    LCD_ShowStatus("HX711 NOT READY", "L:NO R:OK");
  } else {
    LCD_ShowStatus("HX711 NOT READY", "L:OK R:NO");
  }
}

static bool statusUiNormal() {
  return !netUiOverlay;
}

static void startNetOverlay(uint32_t ms = NET_UI_SHOW_MS) {
  netUiOverlay = true;
  netUiUntil = millis() + ms;
}

static void updateStatusUi() {
  if (netUiOverlay && long(millis() - netUiUntil) >= 0) {
    netUiOverlay = false;
    LED_Set(LEDSTATE_IDLE);
    showReadyStatus();
  }
}

static void resetStableDisplayTotal() {
  displayTotalInitialized = false;
}

static int stableDisplayTotal(float totalWeight) {
  if (!displayTotalInitialized) {
    displayTotalInt = int(lroundf(totalWeight));
    displayTotalInitialized = true;
    return displayTotalInt;
  }

  if (fabs(totalWeight - float(displayTotalInt)) > DISPLAY_HOLD_BAND_G) {
    displayTotalInt = int(lroundf(totalWeight));
  }

  return displayTotalInt;
}

static float snapZero(float value) {
  return fabs(value) < ZERO_SNAP_G ? 0.0f : value;
}

static void showWeight(const ScaleReading& r) {
  const float wL = roundf(snapZero(r.wL) * 10.0f) / 10.0f;
  const float wR = roundf(snapZero(r.wR) * 10.0f) / 10.0f;
  const int total = stableDisplayTotal(snapZero(r.total));

  lcdWeight.setCursor(0, 0);
  lcdWeight.print(F("L:"));
  lcdWeight.print(wL, 1);
  lcdWeight.print(F("g R:"));
  lcdWeight.print(wR, 1);
  lcdWeight.print(F("g   "));

  lcdWeight.setCursor(0, 1);
  lcdWeight.print(F("TOT:"));
  lcdWeight.print(total);
  lcdWeight.print(F("g           "));
}

static void startButtonWeightIgnore() {
  weightIgnoreUntil = millis() + BUTTON_WEIGHT_IGNORE_MS;
  needWeightRecoveryAverage = true;
}

static bool isWeightIgnoreActive() {
  return needWeightRecoveryAverage && long(millis() - weightIgnoreUntil) < 0;
}

static bool readCurrentWeight(float& outWeight, uint8_t samples) {
  outWeight = 0.0f;

  if (!gScale.waitReady(500)) {
    return false;
  }

  ScaleReading value = gScale.readOnce(samples);
  if (!value.valid) {
    return false;
  }

  outWeight = roundf(snapZero(value.total) * 10.0f) / 10.0f;
  return true;
}

static String buildRequest(float weight) {
  return String("CAPTURE,W=") + String(int(lroundf(weight))) + "\n";
}

static bool initDemoMode4() {
  LCD_Msg(lcdWeight, F("SMART SCALE"), F("INIT..."));
  LCD_ShowStatus("POWER CHECK", "Init...");
  LED_Set(LEDSTATE_PROCESSING);
  Buzzer_Click();
  delay(800);

  LED_Set(LEDSTATE_PROCESSING);
  LCD_ShowStatus("HX711 CHECK", "WAIT READY");
  if (!gScale.waitReady(3000)) {
    LED_Set(LEDSTATE_ERROR);
    Buzzer_Error();
    showHxReadyStatus();
    delay(1500);
  } else {
    LCD_ShowStatus("HX711 CHECK", "L/R OK");
    delay(800);
  }

#if !MOCK_AI_RESULT
  LED_Set(LEDSTATE_PROCESSING);
  LCD_ShowStatus("W5500 CHECK", "Init...");
  bool w5500Ready = Wiz550_Begin();
  if (w5500Ready) {
    LCD_ShowStatus("W5500 IP", Wiz550_LocalIpText());
  } else {
    LCD_ShowStatus("W5500 FAIL", "Check cable");
    LED_Set(LEDSTATE_ERROR);
    Buzzer_Error();
  }
  delay(1200);

  LED_Set(LEDSTATE_PROCESSING);
  LCD_ShowStatus("WiFi CHECK", "Init aux...");
  wifiReady = WizFi_Begin();
  if (wifiReady) {
    wifiReady = WizFi_JoinAP();
  }
  if (wifiReady) {
    LCD_ShowStatus("WiFi IP", WizFi_GetStaIp());
  } else {
    LCD_ShowStatus("WiFi FAIL", "Fallback off");
    LED_Set(LEDSTATE_ERROR);
    Buzzer_Error();
  }
  delay(1200);
#endif

  gScale.tare(DEMO_TARE_AVG_SAMPLES);
  AutoZero_Init(autoZeroState, 0);
  resetStableDisplayTotal();

  LED_Set(LEDSTATE_IDLE);
  LCD_Msg(lcdWeight, F("L:0.0g R:0.0g"), F("TOT:0g"));
  showReadyStatus();
  return true;
}

static void handleTareButton() {
  ButtonEvent tare = Button_ReadTare();
  if (tare != BTN_SHORT && tare != BTN_LONG) {
    return;
  }

  startButtonWeightIgnore();
  LED_Set(LEDSTATE_PROCESSING);
  LCD_ShowStatus(tare == BTN_LONG ? "SAVE TARE..." : "MANUAL TARE...", "Zeroing...");
  gScale.tare(DEMO_TARE_AVG_SAMPLES);

  if (tare == BTN_LONG) {
    EepromStore::SaveAll(gScale);
    Buzzer_Success();
    LCD_ShowStatus("Saved OK", "Offset stored");
  } else {
    Buzzer_Click();
    LCD_ShowStatus("TARE Done", "W:0.0g");
  }

  LED_Set(LEDSTATE_IDLE);
  resetStableDisplayTotal();
  AutoZero_Init(autoZeroState, 0);
  showWeight({0.0f, 0.0f, 0.0f, true});
  startNetOverlay(1200);
}

static void sendAnalysisRequest() {
  startButtonWeightIgnore();
  LED_Set(LEDSTATE_PROCESSING);
  LCD_ShowStatus("SEND PRESSED", "Stabilizing...");

  while (isWeightIgnoreActive()) {
    delay(5);
  }

  float sendWeight = 0.0f;
  if (!readCurrentWeight(sendWeight, BUTTON_RECOVERY_SAMPLES)) {
    Buzzer_Error();
    LED_Set(LEDSTATE_ERROR);
    LCD_ShowStatus("WEIGHT READ FAIL", "Try again");
    startNetOverlay(2500);
    return;
  }

  const String request = buildRequest(sendWeight);
  String payload;
  bool ok = false;
  bool usedWifi = false;

#if MOCK_AI_RESULT
  delay(2000);
  payload = "G=1,M=BEEF,C=Sirloin";
  ok = true;
#else
  ok = Wiz550_SendRequest(request, payload, AI_REPLY_TIMEOUT_MS);

#if USE_WIFI_FALLBACK
  if (!ok) {
    delay(1500);
    LCD_ShowStatus("W5500 FAIL", "Try WiFi...");
    if (!wifiReady) {
      wifiReady = WizFi_Begin() && WizFi_JoinAP();
    }
    if (wifiReady) {
      ok = WizFi_SendRequest(request, payload, AI_REPLY_TIMEOUT_MS);
      usedWifi = ok;
    }
  }
#endif
#endif

  if (ok) {
    AiResult result;
    if (ParseAiPayload(payload, result)) {
      result.weight = sendWeight;
      LCD_ShowAiResult(result);
    } else if (usedWifi) {
      LCD_ShowStatus("WIFI TX/RX OK", "RX DATA");
    } else {
      LCD_ShowStatus("W5500 TX/RX OK", "RX DATA");
    }

    Buzzer_Success();
    LED_BlinkSuccess();
    Serial.print(F("AI payload: "));
    Serial.println(payload);
    startNetOverlay(NET_UI_SHOW_MS);
  } else {
    Buzzer_Error();
    LED_BlinkError();
    delay(1500);
    LCD_ShowStatus("SEND/RX FAIL", "Check NET/RPI");
    Serial.print(F("AI fail payload: "));
    Serial.println(payload);
    startNetOverlay(2500);
  }
}

static void handleButtons() {
  handleTareButton();

  if (Button_ReadSend() == BTN_SHORT) {
    sendAnalysisRequest();
  }
}

static void updateLiveWeight() {
  if (millis() - lastMeasureMs < DEMO_TARE_UI_UPDATE_MS) {
    return;
  }
  lastMeasureMs = millis();

  if (isWeightIgnoreActive()) {
    return;
  }

  if (!gScale.waitReady(5)) {
    return;
  }

  const uint8_t samples = needWeightRecoveryAverage ? BUTTON_RECOVERY_SAMPLES : DISPLAY_SAMPLES;
  ScaleReading value = gScale.readOnce(samples);
  if (!value.valid) {
    return;
  }

  const int azG = int(lroundf(value.total));
  showWeight(value);
  needWeightRecoveryAverage = false;

  AutoZero_Update(autoZeroState, gScale, azG, &lcdStatus, statusUiNormal(), DEMO_TARE_AVG_SAMPLES);
  if (autoZeroState.justFired) {
    resetStableDisplayTotal();
    Buzzer_Click();
  }
}

void DemoSendAiResult_Run() {
  if (!initialized) {
    initialized = initDemoMode4();
    return;
  }

  handleButtons();
  updateLiveWeight();
  updateStatusUi();

  if (AutoZero_UiReturnDue(autoZeroState) && statusUiNormal()) {
    AutoZero_ClearUiReturn(autoZeroState);
    showReadyStatus();
  }
}
