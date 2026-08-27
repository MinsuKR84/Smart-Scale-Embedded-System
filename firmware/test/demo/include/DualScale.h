#ifndef DUAL_SCALE_H
#define DUAL_SCALE_H

#include <Arduino.h>
#include "HX711.h"

struct ScaleReading {
  float wL;
  float wR;
  float total;
  bool valid;
};

class DualScale {
public:
  long offsetL;
  long offsetR;
  float factorL;
  float factorR;
  float a_coef;
  float b_coef;

  DualScale();
  void begin(uint8_t dtL, uint8_t sckL, uint8_t dtR, uint8_t sckR);
  bool waitReady(uint16_t timeout_ms);
  bool isLeftReady();
  bool isRightReady();
  bool isStable(float& avg_out);
  void tare(uint8_t avg);
  bool measureNetLR(uint8_t samples, long& outNetL, long& outNetR);
  bool lastRaw(long& outRawL, long& outRawR) const;
  ScaleReading readOnce(uint8_t samples);

private:
  HX711 scaleL;
  HX711 scaleR;
  long rawLLast;
  long rawRLast;
};

extern DualScale gScale;

#endif
