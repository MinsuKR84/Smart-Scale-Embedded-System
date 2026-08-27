#ifndef COMM_WIZFI360_H
#define COMM_WIZFI360_H

#include <Arduino.h>

bool WizFi_Begin();
bool WizFi_JoinAP();
String WizFi_GetStaIp();
bool WizFi_SendRequest(const String& request, String& payload, uint16_t timeoutMs);
bool WizFi_ParseIPD(const String& rx, String& payload);

#endif
