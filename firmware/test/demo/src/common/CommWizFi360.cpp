#include "CommWizFi360.h"
#include "Config.h"

static void clearRx() {
  while (Serial.available()) {
    Serial.read();
  }
}

static String readRx(uint16_t timeoutMs) {
  String rx;
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    while (Serial.available()) {
      rx += char(Serial.read());
      if (rx.length() > 220) {
        return rx;
      }
    }
  }
  return rx;
}

static String sendAT(const String& cmd, uint16_t timeoutMs) {
  clearRx();
  Serial.print(cmd);
  Serial.print("\r\n");
  return readRx(timeoutMs);
}

static bool waitForAny(const String& rx, const char* key1, const char* key2 = nullptr, const char* key3 = nullptr) {
  return (key1 && rx.indexOf(key1) >= 0) ||
         (key2 && rx.indexOf(key2) >= 0) ||
         (key3 && rx.indexOf(key3) >= 0);
}

static String serverIpText() {
  IPAddress ip = RPI_SERVER_IP;
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

static String extractQuotedIp(const String& rx) {
  int sta = rx.indexOf("STAIP");
  if (sta < 0) {
    sta = rx.indexOf("+CIFSR:STAIP");
  }
  if (sta < 0) {
    return "";
  }
  int firstQuote = rx.indexOf('"', sta);
  int secondQuote = rx.indexOf('"', firstQuote + 1);
  if (firstQuote < 0 || secondQuote < 0) {
    return "";
  }
  return rx.substring(firstQuote + 1, secondQuote);
}

static String extractAnyIp(const String& rx) {
  String quoted = extractQuotedIp(rx);
  if (quoted.length() > 0) {
    return quoted;
  }

  for (uint16_t i = 0; i < rx.length(); i++) {
    if (!isDigit(rx[i])) {
      continue;
    }

    String token;
    uint8_t dots = 0;
    uint16_t j = i;
    while (j < rx.length() && (isDigit(rx[j]) || rx[j] == '.')) {
      if (rx[j] == '.') {
        dots++;
      }
      token += rx[j++];
    }
    if (dots == 3) {
      return token;
    }
  }

  return "";
}

bool WizFi_Begin() {
  Serial.begin(WIFI_BAUD);
  delay(2500);
  String rx = sendAT("AT", 2000);
  if (rx.indexOf("OK") < 0) {
    return false;
  }
  sendAT("ATE0", 2000);
  rx = sendAT("AT+CWMODE=1", 3000);
  if (rx.indexOf("OK") < 0) {
    return false;
  }
  rx = sendAT("AT+CIPMUX=0", 3000);
  return rx.indexOf("OK") >= 0;
}

bool WizFi_JoinAP() {
  String cmd = String("AT+CWJAP=\"") + WIFI_SSID + "\",\"" + WIFI_PASS + "\"";
  String rx = sendAT(cmd, 25000);
  return waitForAny(rx, "WIFI GOT IP", "GOT IP", "OK");
}

String WizFi_GetStaIp() {
  String rx = sendAT("AT+CIFSR", 5000);
  return extractAnyIp(rx);
}

bool WizFi_ParseIPD(const String& rx, String& payload) {
  int ipd = rx.indexOf("+IPD,");
  if (ipd < 0) {
    return false;
  }
  int comma = rx.indexOf(',', ipd);
  int colon = rx.indexOf(':', comma);
  if (comma < 0 || colon < 0) {
    return false;
  }
  int len = rx.substring(comma + 1, colon).toInt();
  payload = rx.substring(colon + 1, colon + 1 + len);
  payload.trim();
  return payload.length() > 0;
}

bool WizFi_SendRequest(const String& request, String& payload, uint16_t timeoutMs) {
  payload = "";
  sendAT("AT+CIPCLOSE", 1200);
  delay(200);

  String cip = String("AT+CIPSTART=\"TCP\",\"") + serverIpText() + "\"," + RPI_SERVER_PORT;
  String rx = sendAT(cip, 10000);
  if (!waitForAny(rx, "CONNECT", "ALREADY", "OK")) {
    payload = "WIFI TCP FAIL\nIP/port check";
    return false;
  }

  String sendCmd = String("AT+CIPSEND=") + String(request.length());
  rx = sendAT(sendCmd, 3000);
  if (rx.indexOf('>') < 0) {
    payload = "WIFI SEND FAIL\nNo prompt";
    sendAT("AT+CIPCLOSE", 1200);
    return false;
  }

  clearRx();
  Serial.print(request);
  rx = readRx(timeoutMs);
  if (rx.indexOf("SEND OK") < 0) {
    payload = "WIFI SEND FAIL\nNo SEND OK";
    sendAT("AT+CIPCLOSE", 1200);
    return false;
  }

  if (!WizFi_ParseIPD(rx, payload)) {
    payload = "WIFI NO REPLY\nServer ACK?";
    sendAT("AT+CIPCLOSE", 1200);
    return false;
  }
  sendAT("AT+CIPCLOSE", 1200);
  return true;
}
