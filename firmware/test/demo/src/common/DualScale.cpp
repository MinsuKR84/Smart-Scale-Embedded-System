#include "DualScale.h"
#include "Config.h"
#include <math.h>

DualScale gScale;

DualScale::DualScale()
  : offsetL(0),
    offsetR(0),
    factorL(DEFAULT_FACTOR_L),
    factorR(DEFAULT_FACTOR_R),
    a_coef(DEFAULT_A_COEF),
    b_coef(DEFAULT_B_COEF),
    rawLLast(0),
    rawRLast(0) {}

void DualScale::begin(uint8_t dtL, uint8_t sckL, uint8_t dtR, uint8_t sckR) {
  scaleL.begin(dtL, sckL);
  scaleR.begin(dtR, sckR);
  scaleL.set_gain(128);
  scaleR.set_gain(128);
}

bool DualScale::waitReady(uint16_t timeout_ms) {
  unsigned long start = millis();
  while (millis() - start < timeout_ms) {
    if (scaleL.is_ready() && scaleR.is_ready()) {
      return true;
    }
    delay(1);
  }
  return false;
}

bool DualScale::isLeftReady() {
  return scaleL.is_ready();
}

bool DualScale::isRightReady() {
  return scaleR.is_ready();
}

bool DualScale::measureNetLR(uint8_t samples, long& outNetL, long& outNetR) {
  int64_t sumL = 0;
  int64_t sumR = 0;
  uint8_t got = 0;
  unsigned long start = millis();

  while (got < samples) {
    if (millis() - start > 3500UL) {
      break;
    }
    if (!waitReady(200)) {
      continue;
    }

    long rawL = scaleL.read();
    long rawR = scaleR.read();
    sumL += rawL;
    sumR += rawR;
    rawLLast = rawL;
    rawRLast = rawR;
    got++;
    delay(5);
  }

  if (got == 0) {
    return false;
  }

  rawLLast = long(sumL / got);
  rawRLast = long(sumR / got);
  outNetL = rawLLast - offsetL;
  outNetR = rawRLast - offsetR;
  return true;
}

bool DualScale::lastRaw(long& outRawL, long& outRawR) const {
  outRawL = rawLLast;
  outRawR = rawRLast;
  return true;
}

ScaleReading DualScale::readOnce(uint8_t samples) {
  ScaleReading reading = {0.0f, 0.0f, 0.0f, false};
  long netL = 0;
  long netR = 0;
  if (!measureNetLR(samples, netL, netR)) {
    return reading;
  }

  reading.wL = netL * factorL;
  reading.wR = netR * factorR;
  reading.total = a_coef * reading.wL + b_coef * reading.wR;
  if (isnan(reading.total) || isinf(reading.total)) {
    reading.total = 0.0f;
  }
  reading.valid = true;
  return reading;
}

bool DualScale::isStable(float& avg_out) {
  ScaleReading first = readOnce(HX711_READ_SAMPLES);
  if (!first.valid) {
    return false;
  }
  float minVal = first.total;
  float maxVal = first.total;
  float sum = first.total;

  for (uint8_t i = 1; i < HX711_STABLE_SAMPLES; i++) {
    delay(60);
    ScaleReading r = readOnce(HX711_READ_SAMPLES);
    if (!r.valid) {
      return false;
    }
    minVal = min(minVal, r.total);
    maxVal = max(maxVal, r.total);
    sum += r.total;
  }

  avg_out = sum / HX711_STABLE_SAMPLES;
  return (maxVal - minVal) <= STABLE_THRESHOLD_G;
}

void DualScale::tare(uint8_t avg) {
  if (!waitReady(800)) {
    return;
  }

  int64_t sumL = 0;
  int64_t sumR = 0;
  uint8_t got = 0;
  unsigned long start = millis();

  while (got < avg) {
    if (millis() - start > 3500UL) {
      break;
    }
    if (!waitReady(200)) {
      continue;
    }
    sumL += scaleL.read();
    sumR += scaleR.read();
    got++;
    delay(5);
  }

  if (got > 0) {
    offsetL = long(sumL / got);
    offsetR = long(sumR / got);
  }
}
