#ifndef COMM_WIZ550IO_H
#define COMM_WIZ550IO_H

#include <Arduino.h>

bool Wiz550_Begin();
bool Wiz550_IsLinkOk();
bool Wiz550_WaitLink(uint16_t timeoutMs);
String Wiz550_LocalIpText();
bool Wiz550_SendRequest(const String& request, String& response, uint16_t timeoutMs);

#endif
