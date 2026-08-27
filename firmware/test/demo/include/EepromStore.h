#ifndef EEPROM_STORE_H
#define EEPROM_STORE_H

#include "DualScale.h"

namespace EepromStore {
  void LoadAll(DualScale& scale);
  void SaveAll(const DualScale& scale);
}

#endif
