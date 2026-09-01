# Smart Scale Embedded System

ATmega328P-AU 기반 Custom PCB와 Dual Load Cell을 이용하여  
무게 측정, 사용자 인터페이스, Ethernet/Wi-Fi 통신을 구현한 스마트 전자저울 프로젝트입니다.

![Final Smart Scale](docs/images/final_meat_demo.png)

Prototype 단계에서 계량 구조를 먼저 검증한 뒤,  
ATmega328P-AU와 Dual HX711 회로를 적용한 2-Layer Main PCB를 직접 설계·제작하고  
Firmware, Board Bring-up, 계량 실험 및 Raspberry Pi 외부 시스템 연동까지 진행했습니다.


---

## Portfolio

[📄 Embedded HW/FW Portfolio (PDF)](docs/portfolio/portfolio.pdf)

---


## 1. Project Overview & My Contribution

본 프로젝트는 스마트 전자저울과 Raspberry Pi 기반 육류 품질 판별 시스템을 연동한 2인 팀 프로젝트입니다.

### My Scope — Embedded HW / FW

- ATmega328P-AU 기반 2-Layer Main PCB 설계 및 제작
- Dual Load Cell + Dual HX711 측정 회로 구성
- HX711 IC ×2 및 주변회로 PCB 직접 구성
- Tare / Calibration / AutoZero Firmware
- LCD / Button / RGB LED / Buzzer 제어
- WIZ550io Ethernet 통신
- WizFi360io-C Wi-Fi 통신
- Ethernet 응답 실패 시 Wi-Fi 재전송
- PCB Bring-up 및 계량 성능 검증
- 3D 프린팅 외관 설계 및 시스템 조립

### Teammate Scope

- UVC Camera
- Raspberry Pi 4
- AI 기반 육류 품질 판별

### Main Components

| Category | Component |
|---|---|
| MCU | ATmega328P-AU |
| Weight Sensor | 10 kg Load Cell ×2 |
| ADC | HX711 IC ×2 |
| Ethernet | WIZ550io / W5500 |
| Wi-Fi | WizFi360io-C |
| UI | LCD ×2, Button, RGB LED, Buzzer |
| External Processing | Raspberry Pi 4 |


---


## 2. System Architecture

![System Architecture](docs/diagrams/system_architecture.png)

Dual Load Cell의 신호는 각각의 HX711을 통해 ATmega328P-AU로 입력됩니다.

MCU에서는 좌·우 채널을 독립적으로 보정한 뒤 최종 무게를 계산하며,
LCD와 사용자 입력 장치를 제어합니다.

외부 시스템과의 통신은 두 경로로 구성했습니다.

- **SPI → WIZ550io → Ethernet**
- **UART → WizFi360io-C → Wi-Fi**

사용자가 SEND 버튼을 누르면 현재 무게값을 Raspberry Pi 4로 전달하고,
수신한 분석 결과를 LCD에 표시합니다.


---


## 3. Hardware Development

### Prototype → Final Custom PCB

최종 PCB를 바로 제작하지 않고,
ATmega128A와 상용 HX711 모듈을 이용해 Dual Load Cell 측정 구조를 먼저 검증했습니다.

![Prototype Setup](docs/validation/prototype/ATmega128+Breadboard.png)

Prototype에서 검증한 구조를 바탕으로  
최종 시스템에서는 ATmega328P-AU와 두 개의 HX711 IC를 하나의 Main PCB에 통합했습니다.

| Prototype | Final Custom PCB |
|---|---|
| ATmega128A | ATmega328P-AU |
| HX711 Module ×2 | HX711 IC ×2 Direct Integration |
| Breadboard / Jumper Wiring | 2-Layer Custom PCB |
| Measurement Structure Validation | Final Hardware Integration |

최종 PCB에서는 상용 HX711 모듈을 사용하지 않고
HX711 SOP-16 IC와 주변회로를 직접 구성했습니다.

Detailed schematics: [`hardware/schematic`](hardware/schematic)

> Prototype 결과는 측정 구조와 시험 방법을 검토하기 위한 개발 단계 자료이며,
> 최종 성능 결과는 Final PCB Validation을 기준으로 제시합니다.


---


## 4. Firmware & Communication

Firmware는 센서 측정, 보정, 사용자 인터페이스 및 통신 기능을 분리하여 구성했습니다.

| Module | Function |
|---|---|
| `Dual_Scale.h`, `Calib.h` | Dual Load Cell 측정 및 Calibration |
| `Tare.h`, `AutoZero.h` | Tare / AutoZero |
| `LCD.h`, `Button.h` | User Interface |
| `Wiz550.h` | Ethernet |
| `Wizfi360.h` | Wi-Fi |
| `MeatAnalysis.h` | Request / Response 처리 |

두 HX711에서 좌·우 Raw 값을 독립적으로 읽은 뒤
각 채널의 Calibration Factor를 적용하여 무게값을 계산합니다.

### Ethernet / Wi-Fi Fallback

![Communication Flow](docs/diagrams/communication_flow.png)

SEND 입력이 발생하면 Ethernet을 우선 사용합니다.

Ethernet 경로에서 응답을 받지 못하면
시스템 초기화 단계에서 AP Router에 접속해 둔 WizFi360io-C를 이용하여
동일한 요청을 Wi-Fi로 다시 전송합니다.

| Type | Data Format | Processing |
|---|---|---|
| Request | `CAPTURE,W=<weight>` | 현재 무게와 분석 요청 전송 |
| Ethernet Response | `G`, `M`, `C` payload | 결과 파싱 후 LCD 표시 |
| Wi-Fi Response | `+IPD,<len>:<payload>` | `+IPD` 헤더 제거 후 payload 파싱 |


---


## 5. Board Bring-up

Main PCB 제작 후 모든 기능을 한 번에 연결하지 않고
전원부부터 센서, UI, 통신 순서로 단계별 검증을 수행했습니다.

| Step | Verification |
|---|---|
| 1 | VCC-GND Short Check |
| 2 | 5 V / 3.3 V Power Rail |
| 3 | MCU Firmware / GPIO |
| 4 | HX711 #L / #R Raw Response |
| 5 | LCD / Button / RGB LED / Buzzer |
| 6 | Tare / Calibration |
| 7 | WIZ550io SPI / Link / IP |
| 8 | WizFi360io-C AT / AP / IP |

Custom PCB에서 좌·우 HX711의 Raw 데이터가
각 Load Cell의 하중 변화에 따라 독립적으로 반응하는 것을 확인했습니다.

### PCB Bring-up Video

[![PCB Bring-up Test](docs/images/pcb_bringup.png)](https://www.youtube.com/watch?v=BGXn1gEe-zE)



---


## 6. Measurement Validation

Prototype과 Final PCB에서 동일한 종류의 계량 항목을 이용해
측정 특성을 단계적으로 확인했습니다.

시험 항목은 OIML R 76의 비자동 저울 평가 개념을 참고하여 구성했으며,
정식 인증 또는 적합성 시험을 의미하지 않습니다.

| Test | Evaluation |
|---|---|
| Load Error | 기준 하중 대비 표시 오차 |
| Eccentric Loading | 위치 변화에 따른 표시 오차 |
| Repeatability | 동일 하중 반복 측정 특성 |
| Constant-load Variation | 일정 하중 유지 시 표시값 변화 |

### 6.1 Prototype Validation — Development Reference

Prototype 단계에서는 ATmega128A와 상용 HX711 모듈을 이용하여
측정 구조와 실험 방법을 사전 검토했습니다.

현재 `docs/validation/prototype`의 그래프는 개발 단계에서 확보한
대표 측정 결과이며, Final PCB와의 직접적인 정량 성능 비교에는 사용하지 않습니다.

Prototype 자료:

- `docs/validation/prototype/load_Error_result.png`
- `docs/validation/prototype/eccentric_loading_result.png`
- `docs/validation/prototype/repeatability_result.png`
- `docs/validation/prototype/constant_load_result.png`

### 6.2 Final PCB Validation — Primary Result

최종 ATmega328P-AU Main PCB와 기구물을 결합한 상태에서
동일한 계량 항목을 다시 시험했습니다.

#### Load Error

하중 증가에 따른 측정 오차 변화를 확인했습니다.

![Final PCB Load Error](docs/validation/final_pcb/load_Error_result.png)

#### Eccentric Loading

동일한 하중의 적재 위치를 변경하여 위치별 측정 차이를 확인했습니다.

![Final PCB Eccentric Loading](docs/validation/final_pcb/eccentric_loading_result.png)

#### Repeatability

동일 하중을 반복 측정하여 측정값의 Range와 분산을 확인했습니다.

![Final PCB Repeatability](docs/validation/final_pcb/repeatability_result.png)

#### Constant-load Variation

약 5 kg의 하중을 유지한 상태에서 시간에 따른 표시값 변화를 확인했습니다.

![Final PCB Constant Load](docs/validation/final_pcb/constant_load_result.png)

> Final PCB 결과를 본 프로젝트의 최종 계량 검증 결과로 사용합니다.



---


## 7. Engineering Decisions & Troubleshooting

### Dual Load Cell Calibration

**Problem**

좌·우 Load Cell의 Raw 값과 감도 차이로 인해
두 센서에 하나의 동일한 보정값을 적용하기 어려웠습니다.

**Analysis**

좌·우 HX711 값을 각각 확인하여
두 채널의 초기 Raw 값과 하중에 따른 변화량이 서로 다름을 확인했습니다.

**Solution**

각 Load Cell에 독립적인 Offset과 Calibration Factor를 적용한 뒤
보정된 두 채널의 무게값을 합산하도록 Firmware를 구성했습니다.

### Communication Fallback

유선 통신의 연결 상태를 확인하기 쉽다는 점을 고려해
Ethernet을 기본 경로로 사용했습니다.

LAN 연결이 제거되거나 Ethernet 응답을 받지 못한 경우에는
이미 AP에 접속된 WizFi360io-C를 이용하여 동일 요청을 Wi-Fi로 재전송하도록 구성했습니다.


---


## 8. Demo Videos

### 4-Step Functional Demo

무게 측정, Tare/AutoZero, Ethernet 통신 및
Ethernet 실패 후 Wi-Fi 재전송까지 전체 동작 흐름을 확인했습니다.

[![4-Step Functional Demo](docs/images/4-Step_demo.png)](https://www.youtube.com/watch?v=tLRHiVdR0As&feature=youtu.be)


### Final System Demo with Actual Meat

실제 육류를 이용하여 최종 시스템에서
무게 측정 → 분석 요청 → 외부 처리 → 결과 표시까지 전체 흐름을 검증했습니다.

[![Final System Demo](docs/images/final_meat_demo.png)](https://youtu.be/FqbM9Xd1DCc)


---