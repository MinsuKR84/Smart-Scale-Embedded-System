// W5500 SPI/링크/IP 확인
#include <Arduino.h>
#include <SPI.h>
#include <Ethernet3.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define WIZ_RST 9
#define WIZ_CS  10

LiquidCrystal_I2C lcd(0x27, 16, 2);

byte mac[] = { 0x02, 0x11, 0x22, 0x33, 0x44, 0x58 };

IPAddress localIp(192, 168, 0, 50);
IPAddress dnsIp(8, 8, 8, 8);
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255, 255, 255, 0);

void lcdPrint2(const String& line1, const String& line2) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1.substring(0, 16));
  lcd.setCursor(0, 1);
  lcd.print(line2.substring(0, 16));
}

void wizReset() {
  digitalWrite(WIZ_RST, LOW);
  delay(200);
  digitalWrite(WIZ_RST, HIGH);
  delay(1000);
}

uint8_t w5500ReadCommon(uint16_t addr) {
  uint8_t value;

  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));

  digitalWrite(WIZ_CS, LOW);
  SPI.transfer((addr >> 8) & 0xFF);
  SPI.transfer(addr & 0xFF);
  SPI.transfer(0x00);
  value = SPI.transfer(0x00);
  digitalWrite(WIZ_CS, HIGH);

  SPI.endTransaction();

  return value;
}

bool isLinkOn() {
  uint8_t phy = w5500ReadCommon(0x002E);
  return phy & 0x01;
}

void setup() {
  pinMode(WIZ_RST, OUTPUT);
  pinMode(WIZ_CS, OUTPUT);

  digitalWrite(WIZ_CS, HIGH);
  digitalWrite(WIZ_RST, HIGH);

  Wire.begin();
  lcd.init();
  lcd.backlight();

  SPI.begin();

  lcdPrint2("WIZ550 TEST", "Reset...");
  wizReset();

  uint8_t version = w5500ReadCommon(0x0039);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("VER:0x");
  if (version < 16) lcd.print("0");
  lcd.print(version, HEX);

  if (version != 0x04) {
    lcd.setCursor(0, 1);
    lcd.print("W5500 NO RESP");
    while (1) {
      delay(1000);
    }
  }

  lcd.setCursor(0, 1);
  lcd.print("W5500 OK");
  delay(1000);

  lcdPrint2("Wait LINK...", "0 sec");

  bool linkOk = false;

  for (int i = 1; i <= 10; i++) {
    if (isLinkOn()) {
      linkOk = true;
      break;
    }

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Wait LINK...");
    lcd.setCursor(0, 1);
    lcd.print(i);
    lcd.print(" sec");

    delay(1000);
  }

  if (!linkOk) {
    lcdPrint2("LINK OFF", "Check LAN");
    while (1) {
      delay(1000);
    }
  }

  lcdPrint2("LINK ON", "Ethernet begin");
  delay(1000);

  Ethernet.init(WIZ_CS);
  Ethernet.begin(mac, localIp, dnsIp, gateway, subnet);
  delay(1000);

  IPAddress ip = Ethernet.localIP();

  char line1[17];
  char line2[17];

  snprintf(line1, sizeof(line1), "IP:%d.%d", ip[0], ip[1]);
  snprintf(line2, sizeof(line2), "%d.%d LINK ON", ip[2], ip[3]);

  lcdPrint2(line1, line2);
}

void loop() {
  static unsigned long lastCheck = 0;

  if (millis() - lastCheck >= 1000) {
    lastCheck = millis();

    lcd.setCursor(8, 1);

    if (isLinkOn()) {
      lcd.print("LINK ON ");
    } else {
      lcd.print("LINK OFF");
    }
  }
}