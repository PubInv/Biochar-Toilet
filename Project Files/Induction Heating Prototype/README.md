# Phase 2: Dual Induction Heating Prototype

**ESP32-H2 Microcontroller Firmware & Hardware Architecture for Dual Induction Pyrolysis System**

---

## 📋 System Overview

The Phase 2 Dual Induction Heating Prototype upgrades the biochar reactor architecture from resistive heating to a dual-induction heating assembly:

1. **Main Vessel Induction Coil:** Hand-formed hollow copper coil isolated with high-temperature Kapton tape, surrounding the inner vessel inside the modified Hawkins 12L pressure cooker. Driven by an 1800W ZVS DC induction heater via SSR 1 (GPIO4).
2. **Catalytic Converter Induction Heater:** Dedicated secondary induction heater surrounding the external catalytic converter zone. Driven via SSR 2 (GPIO5) to maintain continuous high temperature (800°C+) during processing for syngas cracking and odor mitigation.

---

## 🔧 System Specifications

- **Microcontroller:** Espressif ESP32-H2-DEVKITM-1-N4 (RISC-V, 96MHz)
- **Power Architecture:** 48V LiFePO4 battery bank / Mean Well RSP-2400-48 DC PSU with ZVS induction drivers
- **Pressure Sensing:** 0–15 PSI range (0–5V analog output, mapped to ADC1_CH2 / GPIO3 via 0-3.3V divider)
- **Valve Control:** Solenoid Valve 1 (MOSFET-driven, GPIO10) for pressure relief and steam swing drying
- **Recirculating Pump:** Optional high-current MOSFET driver on GPIO11 (Solenoid 2 channel) for heat exchanger loop cooling
- **Flush Button:** Manual push switch on GPIO12 (Active LOW with internal pull-up) to initiate biochar cycle (30-min auto-timeout) and reset emergency trip via 3s long-press
- **Status LED:** Onboard WS2812 RGB LED on GPIO8 (Green=IDLE/Ready, Orange=Active Cycle, Blue=Dry Latched/Complete, Flashing Red=Emergency Trip)
- **Thermocouple Interface:** MAX31855 K-type breakout via SPI (GPIO0 CLK, GPIO1 CS, GPIO2 MISO)

---

## 📂 Firmware Layout (`Project Files/Induction Heating Prototype/Firmware`)

```
Firmware/
├── CMakeLists.txt         # Root ESP-IDF CMake configuration (project: biochar_induction_firmware)
├── build_firmware.sh      # Linux/macOS build script (auto-fetches ESP-IDF v5.2.1 if needed)
├── build_firmware.bat     # Windows build script
├── .gitignore             # Excludes build output
└── main/
    ├── CMakeLists.txt     # Main component CMake configuration
    └── main.c             # System control loop, ADC calibration, dual heater logic & safety checks
```

---

## 🚀 Building & Flashing Firmware

Navigate to `Project Files/Induction Heating Prototype/Firmware` and run:

**Linux / macOS:**
```bash
./build_firmware.sh
```

**Windows:**
```bat
build_firmware.bat
```

The build scripts clear cached build artifacts, set the target to `esp32h2`, and build the binary via `idf.py build`.

### Passing Additional Targets (Flash & Monitor)

You can pass extra `idf.py` commands (such as `flash`, `monitor`, or `flash monitor`) directly as arguments to the build script:

**Linux / macOS:**
```bash
./build_firmware.sh flash monitor
```

**Windows:**
```bat
build_firmware.bat flash monitor
```
