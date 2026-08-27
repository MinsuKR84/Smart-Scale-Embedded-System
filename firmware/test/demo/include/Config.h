#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <IPAddress.h>

#define DEMO_MODE 4
// DEMO_MODE 1
// = 1. Custom PCB Board Bring-up 및 전체 하드웨어 초기 점검

// DEMO_MODE 2
// = 2. Custom PCB 기반 듀얼 HX711 보정 및 실시간 무게 측정

// DEMO_MODE 3
// = 3. TARE 및 AutoZero 영점 보정 동작 시연

// DEMO_MODE 4
// = 4. SEND 기반 Raspberry Pi 분석 요청 및 결과 LCD 표시
// + 5. Ethernet 실패 시 Wi-Fi Fallback 통신 시연

#define MOCK_AI_RESULT 0
#define USE_WIFI_FALLBACK 1

#define LCD_WEIGHT_ADDR 0x3F
#define LCD_STATUS_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

#define HX711_READ_SAMPLES 1
#define HX711_STABLE_SAMPLES 6
#define STABLE_THRESHOLD_G 2.0f
#define DEMO_TARE_LIVE_SAMPLES 2
#define DEMO_TARE_AVG_SAMPLES 25
#define DEMO_TARE_UI_UPDATE_MS 30UL

#define DEMO_CAL_WEIGHT_G 155.0f
#define DEMO_CAL_SAMPLES 25
#define DEMO_CAL_MIN_NET 100L

#define AUTO_ZERO_BAND_G 3
#define AUTO_ZERO_NO_MOVE_MS 10000UL
#define AUTO_ZERO_MOVE_THRESHOLD_G 2
#define AUTO_ZERO_MIN_INTERVAL_MS 15000UL
#define AUTO_ZERO_STABLE_CHECK_MS 1000UL
#define AUTO_ZERO_UI_RETURN_MS 800UL

#define DEFAULT_FACTOR_L 0.005406f
#define DEFAULT_FACTOR_R 0.004932f
#define DEFAULT_A_COEF 1.0f
#define DEFAULT_B_COEF 1.0f

#define RPI_SERVER_IP IPAddress(192, 168, 0, 6)
#define RPI_SERVER_PORT 5000

#define W5500_LOCAL_IP IPAddress(192, 168, 0, 120)
#define W5500_DNS_IP IPAddress(8, 8, 8, 8)
#define W5500_GATEWAY IPAddress(192, 168, 0, 1)
#define W5500_SUBNET IPAddress(255, 255, 255, 0)

#define WIFI_BAUD 115200
#define WIFI_SSID "AIscale"
#define WIFI_PASS "shinhan1234"

#endif
