// 2점 보정 factor를 계산 후 총 무게와 L/R 무게를 LCD에 표시
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"
#include <math.h>

// =====================
// ATmega328P-AU / Arduino Uno 기준 핀맵
// =====================
#define HX_L_DT    3
#define HX_L_SCK   4

#define HX_R_DT    A0
#define HX_R_SCK   A1

// =====================
// LCD 0x27
// =====================
LiquidCrystal_I2C lcd(0x27, 16, 2);

// =====================
// HX711
// =====================
HX711 scaleL;
HX711 scaleR;

// =====================
// Calibration Config
// =====================
const float CAL_WEIGHT = 155.0f;

// 평균 샘플 수
const byte AVG_TARE_SAMPLES = 25;
const byte AVG_CAL_SAMPLES  = 40;
const byte AVG_LIVE_SAMPLES = 5;

// factor 정상 범위
// 기존 값이 -0.005 근처였으므로 넉넉하게 설정
const float FACTOR_ABS_MIN = 0.000001f;
const float FACTOR_ABS_MAX = 0.100000f;

// det 너무 작을 때 판정 기준
const float CAL_DET_REL_EPS = 0.000001f;

// 기존 참고값
float factorL = -0.005395f;
float factorR = -0.005014f;

long offsetL = 0;
long offsetR = 0;

bool factorInRange(float f) {
  float af = fabsf(f);
  return (af >= FACTOR_ABS_MIN) && (af <= FACTOR_ABS_MAX);
}

bool detTooSmall(float det, float ref) {
  if (ref <= 1.0f) return true;
  return fabsf(det) < (CAL_DET_REL_EPS * ref);
}

void lcdPrint2(const char* line1, const char* line2) {
  char buf1[17];
  char buf2[17];

  snprintf(buf1, sizeof(buf1), "%-16.16s", line1);
  snprintf(buf2, sizeof(buf2), "%-16.16s", line2);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(buf1);
  lcd.setCursor(0, 1);
  lcd.print(buf2);
}

void lcdPrintFactor(const char* title, float value) {
  char line2[17];
  dtostrf(value, 12, 6, line2);
  lcdPrint2(title, line2);
}

void showCountdown(const char* line1, int sec) {
  char line2[17];

  for (int i = sec; i > 0; i--) {
    snprintf(line2, sizeof(line2), "WAIT %2d sec", i);
    lcdPrint2(line1, line2);
    delay(1000);
  }
}

bool readAverageSafe(HX711& scale, long& value, byte times) {
  if (!scale.is_ready()) {
    return false;
  }

  value = scale.read_average(times);
  return true;
}

bool measureNetLR(byte samples, long& netL, long& netR) {
  long rawL = 0;
  long rawR = 0;

  bool okL = readAverageSafe(scaleL, rawL, samples);
  bool okR = readAverageSafe(scaleR, rawR, samples);

  if (!okL || !okR) {
    return false;
  }

  netL = rawR - offsetR;
  netR = rawL - offsetL;

  return true;
}

bool tareBoth() {
  long rawL = 0;
  long rawR = 0;

  bool okL = readAverageSafe(scaleL, rawL, AVG_TARE_SAMPLES);
  bool okR = readAverageSafe(scaleR, rawR, AVG_TARE_SAMPLES);

  if (!okL || !okR) {
    if (!okL && !okR) {
      lcdPrint2("TARE FAIL", "L/R NOT READY");
    } 
    else if (!okR) {
      // scaleR = 실제 LEFT
      lcdPrint2("TARE FAIL", "L NOT READY");
    } 
    else {
      // scaleL = 실제 RIGHT
      lcdPrint2("TARE FAIL", "R NOT READY");
    }

    delay(2000);
    return false;
  }

  offsetL = rawL;
  offsetR = rawR;

  lcdPrint2("TARE OK", "OFFSET SAVED");
  delay(1500);

  return true;
}

void showLRg(const char* title, float Lg, float Rg) {
  char line2[17];
  char lBuf[8];
  char rBuf[8];

  dtostrf(Lg, 5, 1, lBuf);
  dtostrf(Rg, 5, 1, rBuf);

  snprintf(line2, sizeof(line2), "L:%s R:%s", lBuf, rBuf);
  lcdPrint2(title, line2);
  delay(2200);
}

void showSum(const char* title, float sum) {
  char line2[17];
  char sumBuf[10];

  dtostrf(sum, 7, 1, sumBuf);
  snprintf(line2, sizeof(line2), "SUM:%sg", sumBuf);

  lcdPrint2(title, line2);
  delay(1800);
}

bool doCalibration2Point() {
  // 1. 빈 상태 TARE
  lcdPrint2("REMOVE LOAD", "TARE SOON");
  showCountdown("REMOVE LOAD", 5);

  if (!tareBoth()) {
    lcdPrint2("CHECK HX711", "RESET BOARD");
    return false;
  }

  // 2. LEFT 155g 측정
  lcdPrint2("PUT 155g LEFT", "LEFT ONLY");
  showCountdown("PUT 155g LEFT", 8);

  lcdPrint2("MEASURING LEFT", "DO NOT TOUCH");
  delay(1500);

  long nL_left = 0;
  long nR_left = 0;

  if (!measureNetLR(AVG_CAL_SAMPLES, nL_left, nR_left)) {
    lcdPrint2("READ FAIL", "LEFT");
    delay(2000);
    return false;
  }

  // LEFT raw 표시
  {
    char line1[17];
    char line2[17];
    snprintf(line1, sizeof(line1), "NL:%ld", nL_left);
    snprintf(line2, sizeof(line2), "NR:%ld", nR_left);
    lcdPrint2(line1, line2);
    delay(2500);
  }

  // 3. LEFT 무게 제거
  lcdPrint2("REMOVE LEFT", "WAIT STABLE");
  showCountdown("REMOVE LEFT", 5);

  // 4. RIGHT 155g 측정
  lcdPrint2("PUT 155g RIGHT", "RIGHT ONLY");
  showCountdown("PUT 155g RIGHT", 8);

  lcdPrint2("MEASURING RIGHT", "DO NOT TOUCH");
  delay(1500);

  long nL_right = 0;
  long nR_right = 0;

  if (!measureNetLR(AVG_CAL_SAMPLES, nL_right, nR_right)) {
    lcdPrint2("READ FAIL", "RIGHT");
    delay(2000);
    return false;
  }

  // RIGHT raw 표시
  {
    char line1[17];
    char line2[17];
    snprintf(line1, sizeof(line1), "NL:%ld", nL_right);
    snprintf(line2, sizeof(line2), "NR:%ld", nR_right);
    lcdPrint2(line1, line2);
    delay(2500);
  }

  // 5. 2원 1차 방정식으로 factor 계산
  //
  // factorL*nL_left  + factorR*nR_left  = CAL_WEIGHT
  // factorL*nL_right + factorR*nR_right = CAL_WEIGHT
  //
  float det =
      (float)nL_left * (float)nR_right -
      (float)nL_right * (float)nR_left;

  float ref =
      fabsf((float)nL_left * (float)nR_right) +
      fabsf((float)nL_right * (float)nR_left);

  float fL = 0.0f;
  float fR = 0.0f;

  bool useFallback = detTooSmall(det, ref);

  if (!useFallback) {
    fL = CAL_WEIGHT * ((float)nR_right - (float)nR_left) / det;
    fR = CAL_WEIGHT * ((float)nL_left  - (float)nL_right) / det;

    if (!isfinite(fL) || !isfinite(fR) || !factorInRange(fL) || !factorInRange(fR)) {
      useFallback = true;
    }
  }

  // 6. det가 작거나 factor가 튀면 fallback
  if (useFallback) {
    lcdPrint2("det small", "fallback used");
    delay(1500);

    if (labs(nL_left) < 10000 || labs(nR_right) < 10000) {
      lcdPrint2("Fallback FAIL", "net too small");
      delay(2000);
      return false;
    }

    fL = CAL_WEIGHT / (float)nL_left;
    fR = CAL_WEIGHT / (float)nR_right;

    if (!isfinite(fL) || !isfinite(fR) || !factorInRange(fL) || !factorInRange(fR)) {
      lcdPrintFactor("fL ERR", fL);
      delay(2000);

      lcdPrintFactor("fR ERR", fR);
      delay(2000);

      lcdPrint2("FACTOR RANGE", "Retry");
      delay(2000);
      return false;
    }
  }

  factorL = fL;
  factorR = fR;

  // 7. 계산 검증
  float L_left_g  = (float)nL_left  * factorL;
  float R_left_g  = (float)nR_left  * factorR;
  float L_right_g = (float)nL_right * factorL;
  float R_right_g = (float)nR_right * factorR;

  float sum_left  = L_left_g + R_left_g;
  float sum_right = L_right_g + R_right_g;

  showLRg("LEFT 155g", L_left_g, R_left_g);
  showSum("LEFT CHECK", sum_left);

  showLRg("RIGHT 155g", L_right_g, R_right_g);
  showSum("RIGHT CHECK", sum_right);

  // 8. 최종 factor 표시
  lcdPrintFactor("FL FACTOR", factorL);
  delay(2500);

  lcdPrintFactor("FR FACTOR", factorR);
  delay(2500);

  lcdPrint2("CAL OK", "LIVE MODE");
  delay(1500);

  return true;
}

void showLiveWeightLoop() {
  char line1[17];
  char line2[17];

  while (1) {
    long netL = 0;
    long netR = 0;

    bool ok = measureNetLR(AVG_LIVE_SAMPLES, netL, netR);

    if (!ok) {
      lcdPrint2("HX711 ERROR", "CHECK MODULE");
      delay(1000);
      continue;
    }

    float weightL = (float)netL * factorL;
    float weightR = (float)netR * factorR;
    float totalWeight = weightL + weightR;

    char tBuf[8];
    char lBuf[8];
    char rBuf[8];

    dtostrf(totalWeight, 6, 1, tBuf);
    dtostrf(weightL, 5, 1, lBuf);
    dtostrf(weightR, 5, 1, rBuf);

    snprintf(line1, sizeof(line1), "TOTAL:%sg", tBuf);
    snprintf(line2, sizeof(line2), "L:%s R:%s", lBuf, rBuf);

    lcdPrint2(line1, line2);
    delay(800);
  }
}

void setup() {
  Wire.begin();

  lcd.init();
  lcd.backlight();

  lcdPrint2("HX711 2PT CAL", "LCD 0x27");
  delay(1500);

  scaleL.begin(HX_L_DT, HX_L_SCK);
  scaleR.begin(HX_R_DT, HX_R_SCK);

  if (!doCalibration2Point()) {
    lcdPrint2("CAL FAIL", "RESET BOARD");
    while (1) {
      delay(1000);
    }
  }

  showLiveWeightLoop();
}

void loop() {
}


//HX711 L/R raw 확인용
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

#define HX_L_DT  3
#define HX_L_SCK 4

#define HX_R_DT  A0
#define HX_R_SCK A1

#define LED_R 7
#define LED_G 6
#define LED_B 5

LiquidCrystal_I2C lcd(0x27, 16, 2);

HX711 scaleL;
HX711 scaleR;

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  Wire.begin();

  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0);
  lcd.print("HX711 TEST");
  lcd.setCursor(0, 1);
  lcd.print("Init...");
  delay(1000);

  scaleL.begin(HX_L_DT, HX_L_SCK);
  scaleR.begin(HX_R_DT, HX_R_SCK);
}