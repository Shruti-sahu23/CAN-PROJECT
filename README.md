<div align="center">

# 🚗 CAN-Bus Automotive Engine Monitoring & Distributed Vehicle Control System

### **A Multi-ECU Distributed Embedded Architecture on NXP LPC2129 ARM7TDMI-S**

[![Author](https://img.shields.io/badge/Author-Shruti%20Sahu-7c3aed?style=for-the-badge&logo=github)](https://github.com/)
[![Microcontroller](https://img.shields.io/badge/MCU-NXP%20LPC2129%20(ARM7)-0284c7?style=for-the-badge&logo=arm)](https://www.nxp.com/)
[![Protocol](https://img.shields.io/badge/Bus-CAN%202.0B%20%40%20250%20kbps-d97706?style=for-the-badge)](https://en.wikipedia.org/wiki/CAN_bus)
[![Firmware](https://img.shields.io/badge/Firmware-Embedded%20C%20(ISO%20C99)-16a34a?style=for-the-badge&logo=c)](https://en.wikipedia.org/wiki/Embedded_C)
[![Toolchain](https://img.shields.io/badge/Toolchain-Keil%20µVision%20%7C%20Proteus-9333ea?style=for-the-badge)](https://www.keil.com/)

<br>

<p align="center">
  <b>Real-Time Engine Telemetry</b> • <b>8-Level Power Window Position Indicator</b> • <b>Jitter-Free IR Radar</b> • <b>Active CAN Node Supervision & Auto-Recovery</b>
</p>

</div>

---

## 📑 Executive Overview

This repository contains the complete firmware, hardware design specifications, and engineering documentation for an **automotive-grade distributed electronic control system**. Built on three **NXP LPC2129 ARM7TDMI-S** microcontrollers, the system communicates over a high-speed **ISO 11898 CAN 2.0B bus at 250 kbps**.

Unlike simplistic single-board prototypes, this project implements a **true 3-node distributed vehicle network** featuring real-time telemetry streaming, multi-stage digital sensor filtering, boundary-protected position actuation, and **active software watchdog supervision** that detects physical node disconnections and recovers automatically without system freezes.

---

## 🏛️ System Architecture & Block Diagram

```text
                           =======================================================
                           ||   DIFFERENTIAL CAN BUS (CAN_H / CAN_L @ 250 kbps)  ||
                           =======================================================
                            |  [120Ω Term]         |                 |  [120Ω Term]
                     [MCP2551 Transceiver]  [MCP2551 Transceiver]  [MCP2551 Transceiver]
                            |                      |                 |
                     P0.24(TD1)/P0.25(RD1)  P0.24(TD1)/P0.25(RD1)  P0.24(TD1)/P0.25(RD1)
                            |                      |                 |
                   +--------+--------+    +--------+--------+    +---+-------------+
                   |  NODE 1: MASTER |    |  NODE 2: WINDOW |    |  NODE 3: RADAR  |
                   | TELEMETRY ECU   |    |  POSITION ECU   |    |  PROXIMITY ECU  |
                   |  (LPC2129 ARM7) |    |  (LPC2129 ARM7) |    |  (LPC2129 ARM7) |
                   +--------+--------+    +--------+--------+    +---+-------------+
                            |                      |                 |
           +----------------+---------------+      |                 |
           |                |               |      |                 |
      [20×4 LCD]       [DS18B20]       [3 Buttons] |            [Sharp GP2D12]
      Dashboard       Temperature      Inputs      |            Analog IR Sensor
     (P0.0–P0.10)       (P0.22)     (P0.14/15/16)  |            (P0.27 / AD0.0)
                                                   |                 |
                                          [8-LED Position Bar]  [3-Stage DSP Filter]
                                             Glass Levels 0-8     Median + EMA + Hyst
                                               (P0.0 – P0.7)
```

---

## 🧰 Hardware Bill of Materials (Exact Components)

| # | Hardware Component | Qty | Primary Purpose & Function | Physical Node Mapping |
| :-: | :--- | :-: | :--- | :--- |
| **1** | **NXP LPC2129** | 3 | 32-bit ARM7TDMI-S Microcontrollers @ 60 MHz | Master ECU, Window ECU, Radar ECU |
| **2** | **Microchip MCP2551** | 3 | High-Speed CAN Transceivers (ISO 11898) | All 3 Nodes (`P0.24 TD1`, `P0.25 RD1`) |
| **3** | **Maxim DS18B20** | 1 | Digital 1-Wire Engine Temperature Sensor | Main Telemetry ECU (`P0.22` with 4.7kΩ pull-up) |
| **4** | **Sharp GP2D12** | 1 | Analog Infrared Distance / Proximity Sensor | Reverse Radar ECU (`P0.27 / AD0.0`) |
| **5** | **20×4 Alphanumeric LCD** | 1 | Instrument Cluster Digital Telemetry Display | Main Telemetry ECU (`P0.0–P0.10`) |
| **6** | **Position Indicator LEDs** | 8 | Window Glass Position Indicators (0 to 8 Levels) | Window Control ECU (`P0.0–P0.7` Active LOW) |
| **7** | **Push Button Switches** | 3 | Driver Inputs: Win UP, Win DOWN, Reverse Mode | Main Telemetry ECU (`P0.16`, `P0.14`, `P0.15`) |
| **8** | **CAN Differential Bus** | 1 | Shielded Twisted Pair + Two 120Ω Terminations | Network Backbone @ 250 kbps |

---

## ⚙️ Distributed ECU Module Specifications

### 🚗 ECU 1: Master Instrument Cluster & Telemetry Supervisor
* **Central Dashboard**: Drives the **20×4 LCD** via 8-bit parallel bus (`P0.0–P0.7` Data, `P0.8` EN, `P0.9` RS, `P0.10` RW) with custom CGRAM icons (`°`, `▲`, `▼`).
* **1-Wire Temperature Acquisition**: Bit-banged 1-Wire master driver on `P0.22` with $0.0625^\circ\text{C}$ resolution and line fault detection.
* **Driver Input Handling**: Hardware-debounced switch polling on `P0.16` (UP), `P0.14` (DOWN), and `P0.15` (Reverse).
* **Software Watchdog Supervision**: Monitors CAN communication response times ($500\text{ ms}$ for Window, $800\text{ ms}$ for Radar) and triggers automatic fallback recovery.

### 🪟 ECU 2: Body Control & Window Position Indicator
* **8-Level Discrete Position Tracking**: Directly controls 8 physical LEDs across `P0.0 – P0.7` representing discrete levels ($0$ = 0% glass to $8$ = 100% closed glass).
* **Single Tap vs. Continuous Hold**:
  * *Single Tap*: Increments or decrements **exactly 1 LED** per tap.
  * *Continuous Hold*: Automatically steps level-by-level (every $250\text{ ms}$) while held down.
* **Boundary Safeguards**: Ignores invalid movement requests at Level 8 (Fully Closed) and Level 0 (Fully Open).
* **Bidirectional Feedback**: Transmits confirmation frame `0x201` with exact level ($0..8$) and percentage ($0..100\%$).

### 🚧 ECU 3: ADAS Reverse Proximity Radar
* **10-Bit ADC Sampling**: Samples analog output of the Sharp GP2D12 sensor on `P0.27 (AD0.0)` with a dedicated ADC clock divider (`CLKDIV = 14` $\rightarrow 4.0\text{ MHz} \le 4.5\text{ MHz}$).
* **3-Stage Jitter-Free DSP Pipeline**:
  1. **20-Sample Median & Trimmed Mean Filter**: Samples 20 consecutive readings, sorts them, and computes the trimmed average of the middle 8 values to reject noise spikes.
  2. **Exponential Moving Average (EMA)**: Low-pass filter with smoothing factor $\alpha = 0.15$.
  3. **$\pm 2.0\text{ cm}$ Schmitt-Trigger Deadband Hysteresis**: Completely stops single-centimeter LCD toggling and flickering.
* **Zone Classification**: Categorizes distance into **Safe ($\ge 80\text{ cm}$)**, **Warning ($40-79\text{ cm}$)**, **Danger ($20-39\text{ cm}$)**, and **Critical Stop ($< 20\text{ cm}$)**.

---

## 📡 CAN 2.0B Communication Protocol Matrix

All ECUs communicate using 11-bit standard CAN identifiers over a $250\text{ kbps}$ bus (`C1BTR = 0x001C001D` @ $60\text{ MHz}$ PCLK):

| CAN ID | Source ECU | Destination ECU | DLC | Data Byte 0 | Data Byte 1 | Description & Timeout Handling |
| :---: | :---: | :---: | :---: | :---: | :---: | :--- |
| **`0x101`** | Main Node | Window Node | 1 | `0x01`=UP / `0x02`=DOWN | — | Window Step Command ($500\text{ ms}$ timeout watchdog) |
| **`0x102`** | Main Node | Reverse Node | 1 | `0x01`=ON / `0x00`=OFF | — | Reverse Radar Mode Toggle Frame |
| **`0x103`** | Reverse Node | Main Node | 2 | Distance ($10..120\text{ cm}$) | Zone ($1..4$) | Proximity Telemetry Stream ($800\text{ ms}$ signal loss watchdog) |
| **`0x201`** | Window Node | Main Node | 2 | Level ($0..8$) | Percentage ($0..100\%$) | 8-LED Position Status Acknowledgement |

---

## ⭐ Active Node Supervision & Fault Recovery Workflow

```text
  Driver Action (Button Press)
              │
              ▼
   Transmit CAN Frame (0x101 / 0x102)
              │
              ▼
   Wait for Node Response Frame
              │
     ┌────────┴────────┐
     │                 │
[Response Received] [Timeout / Signal Lost]
     │                 │
  Update Display       Display Dedicated 20×4 LCD
  & 8-LED Actuators    Error Warning Screen (1.8s)
                       │
                       ▼
                 Flag "! WARN: ECU OFFLINE"
                 & Safely Restore Telemetry Dash
```

---

## 🖥️ 20×4 LCD Telemetry & Error Screen Gallery

```text
┌────────────────────┐  ┌────────────────────┐
│====================│  │--- VEHICLE DASH ---│
│  SMART VEHICLE ECU │  │TEMP  : 32.5°C [ OK ]│
│ ENGINE & SAFETY OS │  │STATUS: ALL ECUS OK │
│====================│  │WIN: 50%    REV: OFF│
└────────────────────┘  └────────────────────┘
   1. Boot Screen           2. Normal Dashboard

┌────────────────────┐  ┌────────────────────┐
│! CAN BUS WARNING ! │  │! CAN BUS WARNING ! │
│NODE: WINDOW MODULE │  │NODE: REVERSE RADAR │
│ERROR: NO RESPONSE  │  │ERROR: NO RESPONSE  │
│STATUS: OFFLINE /ERR│  │STATUS: OFFLINE /ERR│
└────────────────────┘  └────────────────────┘
  3. Window Node Error     4. Reverse Node Error

┌────────────────────┐  ┌────────────────────┐
│=== POWER WINDOW ===│  │<<  REVERSE RADAR >>│
│MOTION: ROLLING UP ▲│  │DIST  : 110 cm[SAFE]│
│GLASS: [||||....]50%│  │ZONE  : GREEN / OK  │
│STATUS: LEVEL 4 OF 8│  │BAR   :[||||||||||||]│
└────────────────────┘  └────────────────────┘
 5. Window Level 4 (50%)  6. Reverse Radar (Safe)
```

---

## 📁 Repository Structure

```text
CAN-Based-Engine-Monitoring-and-Vehicle-Control-System/
├── Main_Node/                         # Master Telemetry & Instrument Cluster ECU
│   ├── MAIN_NODE.c                    # Main Loop & Central System Coordination
│   ├── dashboard.c / .h               # 20×4 LCD Telemetry & Error Screen Graphics
│   ├── project_functions.c / .h       # CAN Actions & Timeout Supervision Watchdog
│   ├── LCD_functions.c / LCD.h        # HD44780 8-Bit Driver & Custom CGRAM Glyphs
│   ├── DS18B20.c / .h                 # 1-Wire Temperature Driver with Fault Check
│   ├── EINT.c / .h                    # Debounced Push Button Input Driver
│   ├── CAN.c / .h                     # LPC2129 CAN1 Controller Hardware Driver
│   ├── CAN_defines.h                  # Protocol IDs, Command Codes & Zone IDs
│   ├── delays.c / .h                  # Calibrated Microsecond/Millisecond Delays
│   ├── types.h                        # Standard Fixed-Width Typedefs
│   └── Startup.s                      # ARM7 Vector Table & Startup Assembly
│
├── Window_glass_control_Node/         # 8-LED Window Position Indicator ECU
│   ├── MAIN_WINDOW_NODE.c             # Event-Driven CAN Message Listener Loop
│   ├── window_control.c / .h          # 8-Level Discrete LED Stepping & Limits (P0.0–P0.7)
│   ├── CAN.c / .h                     # Hardware CAN1 Driver
│   ├── CAN_Defines.h                  # CAN Protocol Constants
│   ├── delays.c / .h                  # Delay Routines
│   ├── types.h                        # Standard Typedefs
│   └── Startup.s                      # ARM7 Startup Assembly
│
├── Reverse_alert_node/                # ADAS Reverse Proximity Radar ECU
│   ├── MAIN_REVERSE_ALERT_NODE.c      # Reverse Mode Telemetry Streaming Loop
│   ├── distance_sensor.c / .h         # 20-Sample Trimmed Mean, EMA & Deadband Filter
│   ├── adc.c / .h                     # 10-Bit ADC Driver (P0.27 / AD0.0)
│   ├── adc_defines.h                  # ADC Bitmask & Clock Divider Configuration
│   ├── CAN.c / .h                     # Hardware CAN1 Driver
│   ├── CAN_Defines.h                  # CAN Protocol Constants
│   ├── delays.c / .h                  # Delay Routines
│   ├── types.h                        # Standard Typedefs
│   └── Startup.s                      # ARM7 Startup Assembly
│
├── Documentation/                     # Engineering Documentation & Schematics
│   ├── Images/                        # System Diagrams & Vector Block Diagrams
│   │   └── system_block_diagram.svg   # Vector Network Block Diagram
│   ├── PINOUT_AND_WIRING.md           # LPC2129 Pinout & Circuit Wiring Tables
│   ├── SYSTEM_DOCUMENTATION.md        # Technical System Specifications
│   ├── LCD_SCREENS_GALLERY.md         # 20×4 LCD Screen Gallery & Transitions
│   └── documentation.html             # HTML Source for Headless PDF Generation
│
├── Project_Documentation.pdf          # 5-Page Comprehensive Engineering PDF
└── README.md                          # Repository Documentation
```

---

## ▶️ Build & Simulation Instructions

### 1. Keil µVision Compilation
1. Open Keil µVision and load the project for each node.
2. Ensure Target Device is configured as **NXP LPC2129** with a **`12.0 MHz`** crystal (CCLK = 60 MHz via on-chip PLL).
3. Under **Project Options $\rightarrow$ Output**, verify **"Create HEX File"** is enabled.
4. Press **`F7`** (Rebuild All) to compile clean `.hex` binaries.

### 2. Proteus Circuit Simulation
1. Place 3× `LPC2129` microcontrollers and 3× `MCP2551` transceivers in Proteus 8.
2. Wire the differential `CANH` and `CANL` bus with two $120\Omega$ termination resistors.
3. Wire the peripherals:
   - **Main Node**: 20×4 LCD to `P0.0–P0.10`, DS18B20 to `P0.22`, Buttons to `P0.14, P0.15, P0.16`.
   - **Window Node**: 8 LEDs to `P0.0–P0.7` (Active LOW).
   - **Reverse Node**: Sharp GP2D12 analog output to `P0.27 (AD0.0)`.
4. Load each node's `.hex` file and press **Play** to run simulation.

---

## 👩‍💻 Author

* **Shruti Sahu**
* **Project**: CAN-Based Engine Monitoring and Vehicle Control System
* **Specialization**: Embedded Systems & Automotive Electronics (ARM7 / CAN Protocol)
