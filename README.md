# SEMB Light Following Robot

This repository contains the firmware for a light-following robot developed as the final assignment for **SEMB (Embedded Systems)** (MEEC-TEC).

The robot platform is a **Waveshare AlphaBot 2** (2WD differential drive): https://www.waveshare.com/wiki/AlphaBot2

The firmware runs on an **ESP32-S2 Super Mini** (configured in PlatformIO as `esp32-s2-saola-1`, see `platformio.ini`) and uses:
- An **LDR ring/array** to estimate the strongest light direction
- An **ultrasonic distance sensor** to detect obstacles
- **ESP-NOW** to receive commands and send telemetry (the remote/controller will be added to this repo later)

Repo layout:
- `LDR_ROBOT/` -> PlatformIO project (firmware)

## Materials (hardware)

- AlphaBot 2 robot base (chassis + motors + motor driver)
- ESP32-S2 Super Mini (main MCU for sensing + decision + control)
- Arduino board (used only for power distribution: 3.3 V and 5 V)
- DC buck/boost converter (powered from the battery, feeds the ESP32)
- 8x LDR modules (small perfboards: 1x LDR + 1x resistor voltage divider)
- Ultrasonic distance sensor
- Generic 8-channel level shifter (3.3 V <-> 5 V)
- Battery pack (typically 6-8 V) and jumper wires

## Power and interfacing (important)

- The **Arduino board is not used for processing/control**. It is only used to provide:
	- 3.3 V and 5 V rails to the AlphaBot 2 shield
	- a stable 5 V reference for the **high-voltage side** of the level shifter
- All robot **logic and processing** (sensing, mode management, motor commands, ESP-NOW) runs on the **ESP32-S2**.
- The ESP32-S2 is powered from a **generic DC buck/boost converter**, fed by the **battery (6-8 V)**.
- The ESP32-S2 uses 3.3 V GPIOs; the AlphaBot 2 shield uses **5 V logic**, so motor-control signals go through a **level shifter**.
- Signals are carried with jumper wires from the ESP32-side wiring to the AlphaBot 2 shield headers.
- Ensure all grounds are common: battery GND, buck/boost GND, ESP32 GND, Arduino GND, AlphaBot GND.

## ESP32 wiring / pin map

Motor-related signals go ESP32 (3.3 V) -> level shifter -> AlphaBot 2 shield (5 V).


### Motor control (via level shifter to AlphaBot shield)

| Signal | ESP32 GPIO | Notes |
|---|---:|---|
| `PWMA` (left PWM) | 3 | PWM channel `CH_A` |
| `AIN1` (left IN1) | 11 | Direction |
| `AIN2` (left IN2) | 5 | Direction |
| `PWMB` (right PWM) | 13 | PWM channel `CH_B` |
| `BIN1` (right IN1) | 14 | Direction |
| `BIN2` (right IN2) | 12 | Direction |

### Ultrasonic sensor

| Signal | ESP32 GPIO |
|---|---:|
| TRIG | 33 |
| ECHO | 39 |

### LDR sensing (ADC inputs)

The LDRs are read as analog inputs. Index `0..7` maps to the following GPIOs:

| LDR index | ESP32 GPIO |
|---:|---:|
| 0 | 8 |
| 1 | 6 |
| 2 | 10 |
| 3 | 4 |
| 4 | 17 |
| 5 | 18 |
| 6 | 7 |
| 7 | 9 |

## Task overview (what each task does)

- **Ultrasonic Task**: reads the ultrasonic sensor, updates distance + `obstacleDetected` in the RTDB.
- **Motor Control Task**: applies the current motor commands to the AlphaBot motors (with a safety stop outside AUTO/MANUAL modes).
- **System Mode Manager Task**: state machine that consumes received commands and transitions between INIT/STOPPED/CALIBRATION/AUTO/MANUAL.
- **ESPNow RX Task**: event-driven receiver; transfers incoming ESP-NOW packets into the RTDB (commands + joystick values).
- **LDR Sensing Task**: samples the 8 LDR ADC channels into `ldrRaw[]`.
- **Calibration Task**: when in CALIBRATION mode, averages multiple LDR samples to compute `ldrOffset[]` and returns to STOPPED.
- **Light Processing Task**: subtracts offsets, finds the strongest illuminated direction (or "none" below threshold).
- **AutoCtrl Task**: converts the light direction into left/right motor commands (light-following behavior).
- **ManualCtrl Task**: converts joystick input into differential-drive motor commands (manual driving).
- **Telemetry TX Task**: periodically sends telemetry (mode, sensors, timings, CPU utilization) via ESP-NOW.

## RTDB, commands, and telemetry

### What is the RTDB?

The **RTDB (Real-Time Database)** is a shared in-memory struct (`RobotRTDB_t`) used as the single source of truth for the robot state.
All tasks read/write robot state through this struct and protect access with `rtdb_lock()` / `rtdb_unlock()`.

It contains, for example: current mode, sensor readings (ultrasonic + LDRs), processed perception (light direction), motor commands, last received command, and timing/CPU utilization statistics.

### ESP-NOW commands received

The robot receives `CommandPacket_t` packets over ESP-NOW. Supported command types:
- `CMD_CONNECT` (0): handshake / leave INIT
- `CMD_START_STOP_TOGGLE` (1): toggle STOPPED <-> AUTO
- `CMD_CALIBRATE` (2): enter CALIBRATION mode
- `CMD_MANUAL_TOGGLE` (3): toggle AUTO <-> MANUAL
- `CMD_JOYSTICK_DATA` (4): carries joystick `joyX`/`joyY` for manual driving

### Telemetry sent

The robot periodically sends a `TelemetryPacket_t` over ESP-NOW with:
- `currentMode`
- Ultrasonic: `ultrasonicDistance` (cm), `obstacleDetected`
- LDRs: `ldrRaw[8]`, `ldrOffset[8]`, and `lightDirection`
- Timing (us): last and max observed execution times for each task
- `cpu_util_percent` computed over a ~1 second window

## Software

- PlatformIO (VS Code)
- Arduino framework for ESP32 (`espressif32`)
- FreeRTOS tasks (created with `xTaskCreate`)
- ESP-NOW for wireless packets

## How to build / upload

1. Open the `LDR_ROBOT/` folder in VS Code.
2. Install PlatformIO and let it set up the environment.
3. Build:
	- PlatformIO: `PlatformIO: Build`
4. Upload:
	- PlatformIO: `PlatformIO: Upload`
5. Serial monitor (115200):
	- PlatformIO: `PlatformIO: Monitor`

The PlatformIO environment is `env:esp32s2` (board: `esp32-s2-saola-1`).

## Task scheduling

Task -> Priority -> Period

| Task | Priority | Period |
|---|---:|---|
| Ultrasonic Task | 5 (Highest) | 20 ms |
| Motor Control Task | 4 (High) | 30 ms |
| System Mode Manager Task | 3 (Normal) | 50 ms |
| ESPNow RX Task | 3 (Normal) | Event-driven (no periodicity) |
| AutoCtrl Task | 3 (Normal) | 50 ms |
| ManualCtrl Task | 3 (Normal) | 50 ms |
| LDR Sensing Task | 2 (Low) | 75 ms |
| Calibration Task | 2 (Low) | 75 ms |
| Light Processing Task | 2 (Low) | 75 ms |
| Telemetry TX Task | 1 (Very Low) | 150 ms |

## Worst-case scenario (measured execution time)

| Task | Worst-case execution time |
|---|---:|
| Ultrasonic Task | 3489 us |
| Motor Control Task | 809 us |
| System Mode Manager Task | 205 us |
| ESPNow RX Task | 1213 us |
| AutoCtrl Task | 746 us |
| ManualCtrl Task | 725 us |
| LDR Sensing Task | 3853 us |
| Calibration Task | 713 us |
| Light Processing Task | 705 us |
| Telemetry TX Task | 625 us |

## Demo video

YouTube: **TBD** (link will be added here)

## Notes / upcoming

- A separate **remote/controller** will be added to this repository in a future update.

