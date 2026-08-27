#include "DemoPowerInit.h"
#include "Buzzer.h"
#include "CommWiz550io.h"
#include "CommWizFi360.h"
#include "Config.h"
#include "DualScale.h"
#include "LCD.h"
#include "LED.h"
#include "PinMap.h"

static void showCheck(const String& line1, const String& line2) {
  LED_Set(LEDSTATE_PROCESSING);
  LCD_ShowStatus(line1, line2);
  delay(1500);
}

static void showOk(const String& line1, const String& line2) {
  LED_Set(LEDSTATE_IDLE);
  LCD_ShowStatus(line1, line2);
  delay(2000);
}

static void showFail(const String& line1, const String& line2) {
  LED_Set(LEDSTATE_ERROR);
  LCD_ShowStatus(line1, line2);
  delay(2200);
}

void DemoPowerInit_Run() {
  static bool done = false;
  if (done) {
    return;
  }
  done = true;

  LCD_ShowBoot();
  Serial.println(F("DEMO 1: PCB power and LCD init"));
  delay(1800);

  showCheck("POWER CHECK", "Init...");
  showOk("POWER CHECK", "Init OK");

  LCD_ShowStatus("LED CHECK", "Green Blue Red");
  LED_Set(LEDSTATE_IDLE);
  delay(700);
  LED_Set(LEDSTATE_PROCESSING);
  delay(700);
  LED_Set(LEDSTATE_ERROR);
  delay(700);
  LED_Set(LEDSTATE_IDLE);
  Buzzer_Click();
  delay(1000);

  showCheck("HX711 CHECK", "WAIT READY");
  bool hxReady = gScale.waitReady(2500);
  if (hxReady) {
    showOk("HX711 CHECK", "READY");
  } else {
    showFail("HX711 CHECK", "FAIL");
  }

  showCheck("W5500 CHECK", "SPI RESET...");
  bool wizChipOk = Wiz550_Begin();
  bool wizLinkOk = false;
  if (wizChipOk) {
    showCheck("W5500 CHECK", "WAIT LINK...");
    wizLinkOk = Wiz550_WaitLink(8000);
  }

  if (!wizChipOk) {
    showFail("W5500 FAIL", "SPI/RESET");
  } else if (!wizLinkOk) {
    showFail("W5500 NO LINK", "Check cable");
  } else {
    showOk("W5500 IP", Wiz550_LocalIpText());
  }

  showCheck("WIFI CHECK", "AT...");
  bool wifiAtOk = WizFi_Begin();
  bool wifiJoinOk = false;
  String wifiIp = "";
  if (wifiAtOk) {
    showCheck("WIFI CHECK", "JOIN AP...");
    wifiJoinOk = WizFi_JoinAP();
    if (wifiJoinOk) {
      showCheck("WIFI CHECK", "GET IP...");
      wifiIp = WizFi_GetStaIp();
    }
  }

  if (!wifiAtOk) {
    showFail("WIFI AT FAIL", "Check module");
  } else if (!wifiJoinOk) {
    showFail("WIFI JOIN FAIL", "Check AP");
  } else if (wifiIp.length() == 0) {
    showFail("WIFI IP FAIL", "Check CIFSR");
  } else {
    showOk("WIFI IP", wifiIp);
  }

  bool allOk = wizChipOk && wizLinkOk && wifiAtOk && wifiJoinOk && hxReady;
  LCD_ShowStatus(allOk ? "Custom PCB OK" : "CHECK FAILED", allOk ? "SYSTEM ALL READY" : "See last error");
  LCD_Msg(lcdWeight, "SMART SCALE", allOk ? "READY" : "CHECK FAIL");
  LED_Set(allOk ? LEDSTATE_IDLE : LEDSTATE_ERROR);
}
