<div align="center">

# 🔩 Embedded Systems Portfolio

### Arham Amir — Electronics Engineering @ NED University, Karachi

[![Arduino](https://img.shields.io/badge/Arduino-IDE_2.x-00979D?style=for-the-badge&logo=arduino)](https://www.arduino.cc/)
[![ESP32](https://img.shields.io/badge/ESP32-IoT-red?style=for-the-badge)](https://www.espressif.com/)
[![PlatformIO](https://img.shields.io/badge/PlatformIO-Embedded-orange?style=for-the-badge)](https://platformio.org/)
[![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)](LICENSE)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-Arham_Amir-0077B5?style=for-the-badge&logo=linkedin)](https://linkedin.com/in/arham-amir)

> Firmware, schematics, and documentation for real-world embedded systems projects.
> Built on **Arduino Nano**, **Arduino UNO**, and **ESP32** — covering power electronics, robotics, wireless control, and IoT signal processing.

</div>

---

## 📁 Repository Structure

```
embedded-projects-repo/
│
├── 📂 01_AC_Energy_Analyzer_Nano/
├── 📂 02_Auto_Power_Factor_Correction_Nano/
├── 📂 03_Noise_Cancellation_Tinnitus_Alert_ESP32/
├── 📂 04_Function_Generator_MCP_UNO/
├── 📂 05_Smart_Breaker_Nano/
├── 📂 06_Self_Balancing_Robot_UNO/
├── 📂 07_Bluetooth_Car_UNO/
├── 📂 08_Fire_Robot_UNO/
├── 📂 09_Gesture_Control_Car_UNO/
├── 📂 10_Obstacle_Avoiding_Robot_UNO/
├── 📂 11_RC_Car_Nano/
├── 📂 12_7CH_RC_Transmitter_Receiver_Nano/
│
└── README.md
```

Each project folder contains: `src/` · `schematics/` · `docs/`

---

## 🗂️ Project Index

| # | Project | Platform | Category |
|---|---------|----------|----------|
| 01 | [AC Energy Analyzer](#1-ac-energy-analyzer) | Arduino Nano | Power Electronics |
| 02 | [Automatic Power Factor Correction](#2-automatic-power-factor-correction-system) | Arduino Nano | Power Electronics |
| 03 | [Noise Cancellation & Tinnitus Alert](#3-noise-cancellation-tinnitus--alert-system) | ESP32 | Audio / IoT |
| 04 | [Function Generator (MCP4725)](#4-function-generator-mcp4725-dac) | Arduino UNO | Test & Measurement |
| 05 | [Smart Breaker](#5-smart-breaker) | Arduino Nano | Power Electronics |
| 06 | [Self Balancing Robot](#6-self-balancing-robot) | Arduino UNO | Robotics |
| 07 | [Bluetooth Car](#7-bluetooth-car) | Arduino UNO | Robotics / Wireless |
| 08 | [Fire Robot](#8-fire-fighting-robot) | Arduino UNO | Robotics / Safety |
| 09 | [Gesture Control Car](#9-gesture-control-car) | Arduino UNO | Robotics / HMI |
| 10 | [Obstacle Avoiding Robot](#10-obstacle-avoiding-robot) | Arduino UNO | Robotics / Autonomous |
| 11 | [RC Car](#11-rc-car) | Arduino Nano | Robotics / Wireless |
| 12 | [7-Channel RC Transmitter & Receiver](#12-7-channel-rc-transmitter--receiver) | Arduino Nano | Wireless / RF |

---

## 🚀 Projects

---

### 1. AC Energy Analyzer

> **Real-time AC parameter monitoring — Vrms, Irms, power, power factor, and energy on an OLED display.**

| MCU | Voltage Sense | Current Sense | Display |
|-----|--------------|---------------|---------|
| Arduino Nano | ZMPT101B | ACS712 | SSD1306 OLED |

- Measures voltage, current, active power (W), apparent power (VA), power factor, and cumulative energy (kWh)
- Calibrated sensor front-end for accurate real-world readings
- Multi-page OLED UI with serial logging support

📂 [`01_AC_Energy_Analyzer_Nano/`](./01_AC_Energy_Analyzer_Nano/)

---

### 2. Automatic Power Factor Correction System

> **Closed-loop power factor correction — switches capacitor banks automatically to bring PF to unity.**

| MCU | Voltage Sense | Switching | Load |
|-----|--------------|-----------|------|
| Arduino Nano | ZMPT101B + ACS712 | Relay / TRIAC bank | Inductive AC loads |

- Detects phase angle between voltage and current in real time
- Automatically engages capacitor stages to cancel reactive power
- OLED display showing live PF reading and correction status
- ⚠️ Mains voltage — full electrical safety precautions required

📂 [`02_Auto_Power_Factor_Correction_Nano/`](./02_Auto_Power_Factor_Correction_Nano/)

---

### 3. Noise Cancellation, Tinnitus & Alert System

> **Multi-function ESP32 audio processing — ambient noise monitoring, tinnitus masking tones, and threshold alerts.**

| MCU | Microphone | Audio Output | Display |
|-----|-----------|--------------|---------|
| ESP32 DevKit V1 | MAX4466 / MAX9814 | MAX98357A I2S + Speaker | SSD1306 OLED |

- Three selectable modes: noise monitor, tinnitus masking, or combined
- Generates white/pink noise masking tones via I2S DAC
- Triggers LED and buzzer alert when ambient noise exceeds configurable threshold

📂 [`03_Noise_Cancellation_Tinnitus_Alert_ESP32/`](./03_Noise_Cancellation_Tinnitus_Alert_ESP32/)

---

### 4. Function Generator (MCP4725 DAC)

> **Benchtop-style waveform generator — sine, square, triangle, sawtooth via 12-bit I2C DAC.**

| MCU | DAC | Waveforms | Frequency Control |
|-----|-----|-----------|------------------|
| Arduino UNO | MCP4725 (12-bit) | Sine · Square · Triangle · Sawtooth | Rotary encoder + serial |

- Adjustable frequency and amplitude via rotary encoder
- Waveform type selectable by push button
- 16x2 LCD displays active mode and frequency
- Optional op-amp output buffer for signal integrity

📂 [`04_Function_Generator_MCP_UNO/`](./04_Function_Generator_MCP_UNO/)

---

### 5. Smart Breaker

> **Intelligent electronic circuit breaker — monitors current, trips on overload, manual reset via button.**

| MCU | Current Sense | Switching | Indicators |
|-----|--------------|-----------|------------|
| Arduino Nano | ACS712 | 5V Relay | OLED + RGB LED + Buzzer |

- Continuously monitors AC current against a configurable trip threshold
- Auto-disconnects load on overload with audible and visual alarm
- Latching trip logic with manual push-button reset
- OLED displays live current reading and breaker status

📂 [`05_Smart_Breaker_Nano/`](./05_Smart_Breaker_Nano/)

---

### 6. Self Balancing Robot

> **Two-wheeled inverted pendulum robot — MPU-6050 IMU with real-time PID control loop.**

| MCU | IMU | Motor Driver | Control |
|-----|-----|-------------|---------|
| Arduino UNO | MPU-6050 | L298N | PID (Kp, Ki, Kd tunable) |

- Reads gyroscope + accelerometer data via I2C at high frequency
- PID algorithm adjusts motor PWM in real time to maintain balance
- Tune `Kp`, `Ki`, `Kd` constants in firmware to match chassis weight
- Start values: `Kp=20`, `Ki=0`, `Kd=0.8`

📂 [`06_Self_Balancing_Robot_UNO/`](./06_Self_Balancing_Robot_UNO/)

---

### 7. Bluetooth Car

> **4-wheeled smartphone-controlled robot over Bluetooth — forward, reverse, left, right, speed control.**

| MCU | BT Module | Motor Driver | App |
|-----|-----------|-------------|-----|
| Arduino UNO | HC-05 / HC-06 | L298N | Arduino Bluetooth Controller |

- Serial Bluetooth command parsing — `F`, `B`, `L`, `R`, `S`
- Speed control via PWM mapped to app slider values
- Note: disconnect HC-05 TX/RX before uploading firmware

📂 [`07_Bluetooth_Car_UNO/`](./07_Bluetooth_Car_UNO/)

---

### 8. Fire Fighting Robot

> **Autonomous fire detection and suppression — 3-sensor array for directional navigation toward flame.**

| MCU | Sensors | Extinguisher | Driver |
|-----|---------|-------------|--------|
| Arduino UNO | 3× IR Flame Sensors | DC Fan / Pump via relay | L298N |

- Left / center / right flame sensor array for directional fire homing
- Robot drives toward the strongest flame signal and activates extinguisher
- Stops and patrols when no fire detected

📂 [`08_Fire_Robot_UNO/`](./08_Fire_Robot_UNO/)

---

### 9. Gesture Control Car

> **Hand-tilt controlled robot — MPU-6050 glove transmitter sends gesture data wirelessly via NRF24L01.**

| Units | IMU | Wireless | Motor Driver |
|-------|-----|----------|-------------|
| 2× Arduino UNO (TX + RX) | MPU-6050 | NRF24L01 2.4 GHz | L298N |

- Tilt forward → drive forward · tilt back → reverse · tilt left/right → turn
- Flat held → stop
- Full wireless link — no USB tether between glove and car

📂 [`09_Gesture_Control_Car_UNO/`](./09_Gesture_Control_Car_UNO/)

---

### 10. Obstacle Avoiding Robot

> **Fully autonomous navigation — ultrasonic sensor on servo scans environment and routes around obstacles.**

| MCU | Sensor | Servo | Driver |
|-----|--------|-------|--------|
| Arduino UNO | HC-SR04 Ultrasonic | SG90 (pan scan) | L298N |

- Scans left and right when obstacle detected within 20 cm
- Turns toward the side with greater clearance
- Continuous autonomous operation — no remote required

📂 [`10_Obstacle_Avoiding_Robot_UNO/`](./10_Obstacle_Avoiding_Robot_UNO/)

---

### 11. RC Car (NRF24L01)

> **Compact dual-Nano radio car — dual-axis joystick transmitter, 2.4 GHz NRF24L01 link.**

| Units | Wireless | Control | Driver |
|-------|----------|---------|--------|
| 2× Arduino Nano (TX + RX) | NRF24L01 | Dual-axis joystick (KY-023) | L298N |

- Custom transmitter and receiver — no commercial RC hardware needed
- Joystick X/Y axes mapped to steering and throttle PWM
- Add 100µF bypass cap on NRF24L01 VCC/GND for stable RF link

📂 [`11_RC_Car_Nano/`](./11_RC_Car_Nano/)

---

### 12. 7-Channel RC Transmitter & Receiver

> **Full DIY 7-channel RC system from scratch — 6 proportional channels + 1 digital aux, built on Arduino Nano.**

| Units | Wireless | Channels | Compatible With |
|-------|----------|----------|----------------|
| 2× Arduino Nano (TX + RX) | NRF24L01+PA+LNA | CH1–CH6 proportional + CH7 digital | Drones · Aircraft · Ground vehicles |

- 4 joystick axes + 2 potentiometers + 1 toggle switch
- Servo/ESC outputs on CH1–CH7 (D2–D8 on receiver)
- OLED transmitter status display · calibratable endpoints
- Use NRF24L01+PA+LNA for extended range — requires stable 3.3V supply

📂 [`12_7CH_RC_Transmitter_Receiver_Nano/`](./12_7CH_RC_Transmitter_Receiver_Nano/)

---

## 🛠 Tools & Stack

| Category | Details |
|----------|---------|
| Firmware IDE | Arduino IDE 2.x · PlatformIO |
| Simulation | Proteus · Wokwi |
| Microcontrollers | Arduino Nano (ATmega328P) · Arduino UNO · ESP32 DevKit V1 |
| Wireless | NRF24L01 2.4 GHz · HC-05 / HC-06 Bluetooth |
| Key ICs | MPU-6050 · ACS712 · ZMPT101B · MCP4725 · L298N · HC-SR04 |
| Protocols | I2C · SPI · UART · PWM · 2.4 GHz RF |

---

## 📬 Contact

| | |
|---|---|
| 📧 Email | arhamamir117@gmail.com |
| 💼 LinkedIn | [linkedin.com/in/arham-amir](https://linkedin.com/in/arham-amir) |
| 🐙 GitHub | [github.com/arham-amir](https://github.com/arham-amir) |
| 📍 Location | Karachi, Pakistan |

---

<div align="center">

*All projects are original builds. Source code, schematics, and wiring diagrams included in each project folder.*

⭐ Star this repo if you find it useful!

</div>
