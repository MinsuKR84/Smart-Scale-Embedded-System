// WizFi360 AT/Wi-Fi 접속 확인
#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LED_R 7
#define LED_G 6
#define LED_B 5

// AT 응답 테스트에서 성공한 baud로 설정하세요.
// 보통 115200 또는 9600입니다.
#define WIFI_BAUD 115200

const char* WIFI_SSID = "MY_HOME_WIFI";
const char* WIFI_PASS = "MY_HOME_PASS";

// LCD 주소 확인 필요: 0x27 또는 0x3F
LiquidCrystal_I2C lcd(0x27, 16, 2);

void lcdPrint2(const String& line1, const String& line2) {
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print(line1.substring(0, 16));

  lcd.setCursor(0, 1);
  lcd.print(line2.substring(0, 16));
}

void clearWifiBuffer() {
  while (Serial.available()) {
    Serial.read();
  }
}

String readWifiResponse(unsigned long timeoutMs) {
  String res = "";
  unsigned long start = millis();

  while (millis() - start < timeoutMs) {
    while (Serial.available()) {
      char c = Serial.read();
      res += c;

      // ATmega328P RAM 보호
      if (res.length() > 220) {
        return res;
      }
    }
  }

  return res;
}

String sendAT(const String& cmd, unsigned long timeoutMs) {
  clearWifiBuffer();

  Serial.print(cmd);
  Serial.print("\r\n");

  return readWifiResponse(timeoutMs);
}

bool responseOK(const String& res) {
  return res.indexOf("OK") >= 0 || res.indexOf("WIFI GOT IP") >= 0;
}

// AT+CIFSR 응답에서 STAIP IP만 추출
String extractStaIp(const String& res) {
  int staIndex = res.indexOf("STAIP");

  if (staIndex < 0) {
    return "";
  }

  int firstQuote = res.indexOf('"', staIndex);
  if (firstQuote < 0) {
    return "";
  }

  int secondQuote = res.indexOf('"', firstQuote + 1);
  if (secondQuote < 0) {
    return "";
  }

  return res.substring(firstQuote + 1, secondQuote);
}

// IP를 16x2 LCD에 맞게 출력
void lcdPrintIp(const String& ip) {
  lcd.clear();

  if (ip.length() == 0) {
    lcd.setCursor(0, 0);
    lcd.print("IP PARSE FAIL");
    lcd.setCursor(0, 1);
    lcd.print("Check CIFSR");
    return;
  }

  // IP가 16글자 이하이면 한 줄에 표시
  if (ip.length() <= 16) {
    lcd.setCursor(0, 0);
    lcd.print("JOIN OK");
    lcd.setCursor(0, 1);
    lcd.print(ip);
  }
  // IP가 길면 두 줄로 분할 표시
  else {
    lcd.setCursor(0, 0);
    lcd.print(ip.substring(0, 16));
    lcd.setCursor(0, 1);
    lcd.print(ip.substring(16, 32));
  }
}

void setup() {
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  digitalWrite(LED_R, LOW);
  digitalWrite(LED_G, LOW);
  digitalWrite(LED_B, LOW);

  Wire.begin();
  lcd.init();
  lcd.backlight();

  lcdPrint2("WizFi360 TEST", "UART Init...");
  Serial.begin(WIFI_BAUD);
  delay(1500);

  lcdPrint2("WIFI TEST", "AT...");
  String res = sendAT("AT", 2000);

  if (res.indexOf("OK") < 0) {
    digitalWrite(LED_R, HIGH);
    lcdPrint2("AT FAIL", "Check UART");
    while (1) {
      delay(1000);
    }
  }

  digitalWrite(LED_G, HIGH);
  lcdPrint2("AT OK", "Set STA Mode");
  delay(700);

  sendAT("AT+CWMODE=1", 3000);

  lcdPrint2("JOIN AP", "Wait...");
  delay(700);

  String joinCmd = String("AT+CWJAP=\"") + WIFI_SSID + "\",\"" + WIFI_PASS + "\"";
  res = sendAT(joinCmd, 15000);

  if (responseOK(res)) {
    digitalWrite(LED_G, HIGH);
    digitalWrite(LED_R, LOW);

    lcdPrint2("WIFI JOIN OK", "Get IP...");
    delay(1000);

    res = sendAT("AT+CIFSR", 4000);

    String ip = extractStaIp(res);
    lcdPrintIp(ip);
  } else {
    digitalWrite(LED_R, HIGH);
    digitalWrite(LED_G, LOW);

    lcdPrint2("JOIN FAIL", res.substring(0, 16));
  }
}

void loop() {
}

