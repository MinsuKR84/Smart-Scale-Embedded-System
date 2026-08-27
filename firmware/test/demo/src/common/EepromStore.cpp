#include "EepromStore.h"
#include "Config.h"
#include <EEPROM.h>
#include <math.h>
#include <stdlib.h>

struct StoreData {
  uint32_t magic;
  float factorL;
  float factorR;
  float a_coef;
  float b_coef;
  long offsetL;
  long offsetR;
};

static const uint32_t STORE_MAGIC = 0x41534331UL;

static bool saneFactor(float v) {
  return !isnan(v) && !isinf(v) && fabs(v) > 0.0000001f && fabs(v) < 1000000.0f;
}

static bool saneOffset(long v) {
  return labs(v) < 9000000L;
}

void EepromStore::LoadAll(DualScale& scale) {
  StoreData data;
  EEPROM.get(0, data);
  if (data.magic != STORE_MAGIC) {
    return;
  }

  scale.factorL = saneFactor(data.factorL) ? data.factorL : DEFAULT_FACTOR_L;
  scale.factorR = saneFactor(data.factorR) ? data.factorR : DEFAULT_FACTOR_R;
  scale.a_coef = saneFactor(data.a_coef) ? data.a_coef : DEFAULT_A_COEF;
  scale.b_coef = saneFactor(data.b_coef) ? data.b_coef : DEFAULT_B_COEF;
  scale.offsetL = saneOffset(data.offsetL) ? data.offsetL : 0;
  scale.offsetR = saneOffset(data.offsetR) ? data.offsetR : 0;
}

void EepromStore::SaveAll(const DualScale& scale) {
  StoreData data = {
    STORE_MAGIC,
    scale.factorL,
    scale.factorR,
    scale.a_coef,
    scale.b_coef,
    scale.offsetL,
    scale.offsetR
  };
  EEPROM.put(0, data);
}
