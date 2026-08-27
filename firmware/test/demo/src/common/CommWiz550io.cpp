#include "CommWiz550io.h"
#include "Config.h"
#include "PinMap.h"
#include <SPI.h>
#include <Ethernet3.h>

static byte mac[] = {0x02, 0x11, 0x22, 0x33, 0x44, 0x58};
static EthernetClient client;
static bool ethernetBegun = false;

static uint8_t w5500ReadCommon(uint16_t addr) {
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  digitalWrite(PIN_WIZ_CS, LOW);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  SPI.transfer(0x00);
  uint8_t value = SPI.transfer(0x00);
  digitalWrite(PIN_WIZ_CS, HIGH);
  SPI.endTransaction();
  return value;
}

bool Wiz550_Begin() {
  pinMode(PIN_WIZ_RST, OUTPUT);
  pinMode(PIN_WIZ_CS, OUTPUT);
  digitalWrite(PIN_WIZ_CS, HIGH);
  digitalWrite(PIN_WIZ_RST, LOW);
  delay(20);
  digitalWrite(PIN_WIZ_RST, HIGH);
  delay(150);

  SPI.begin();
  Ethernet.setCsPin(PIN_WIZ_CS);
  Ethernet.begin(mac, W5500_LOCAL_IP, W5500_SUBNET, W5500_GATEWAY, W5500_DNS_IP);
  Ethernet.setRtTimeOut(3000);
  Ethernet.setRtCount(4);
  client.stop();
  delay(1500);
  ethernetBegun = true;
  return w5500ReadCommon(0x0039) == 0x04 && Wiz550_WaitLink(3000);
}

bool Wiz550_IsLinkOk() {
  return (w5500ReadCommon(0x002E) & 0x01) != 0;
}

bool Wiz550_WaitLink(uint16_t timeoutMs) {
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    if (Wiz550_IsLinkOk()) {
      return true;
    }
    delay(100);
  }
  return false;
}

String Wiz550_LocalIpText() {
  IPAddress ip = ethernetBegun ? Ethernet.localIP() : W5500_LOCAL_IP;
  return String(ip[0]) + "." + String(ip[1]) + "." + String(ip[2]) + "." + String(ip[3]);
}

bool Wiz550_SendRequest(const String& request, String& response, uint16_t timeoutMs) {
  response = "";

  if (!Wiz550_IsLinkOk()) {
    if (!Wiz550_Begin()) {
      response = "W5500 NO LINK\nCheck cable";
      return false;
    }
  }

  client.stop();
  delay(500);

  bool connected = false;
  for (uint8_t attempt = 0; attempt < 2; attempt++) {
    if (!Wiz550_IsLinkOk()) {
      response = "W5500 NO LINK\nCheck cable";
      return false;
    }
    if (client.connect(RPI_SERVER_IP, RPI_SERVER_PORT) > 0) {
      connected = true;
      delay(300);
      break;
    }
    client.stop();
    delay(1000);
  }

  if (!connected) {
    response = "W5500 TCP FAIL\nIP/port check";
    return false;
  }

  if (client.print(request) == 0 && !client.connected()) {
    response = "W5500 SEND FAIL\nTCP closed";
    client.stop();
    return false;
  }

  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
    while (client.available()) {
      char c = client.read();
      if (c == '\n') {
        response.trim();
        client.stop();
        return response.length() > 0;
      }
      response += c;
      if (response.length() > 96) {
        response.trim();
        client.stop();
        return true;
      }
    }
    if (!client.connected() && !client.available()) {
      break;
    }
    delay(10);
  }

  response = "W5500 NO REPLY\nServer ACK?";
  client.stop();
  return false;
}
