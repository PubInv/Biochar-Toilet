# Conductive Heating Prototype — Phase 1

**Biochar Toilet Project | Phase 1 Documentation** |
**Contributor:** Hardhik Pyla ([@hardhik03](https://github.com/hardhik03)) |
**Period:** November 2025 – May 2026 |
**Status:** Complete

---

## Overview

Phase 1 of the Biochar Toilet project was a proof-of-concept prototype to validate the core system architecture before advancing to higher-temperature charring methods. The goal was to build the smallest, safest system possible — one that could demonstrate:

1. Reliable pressure and temperature sensing inside a sealed reaction vessel
2. Automatic dryness detection using temperature–pressure divergence logic
3. Autonomous heater shutoff via SSR when the dry state is detected
4. Flash decompression: opening a solenoid valve under pressure to flash-boil trapped water into steam (providing mechanical sterility)

The reaction vessel was a standard pressure cooker. The heating source was a hot plate controlled by a solid-state relay (SSR). Sensing and control were handled by an ESP32-based embedded system, later migrated to a custom-designed control PCB.

---

## System Architecture

```
[Pressure Cooker] ──── [K-type Thermocouple] ───► [ESP32-H2 Control Board]
      │                                                      │
      ├──── [Analog Pressure Sensor] ───────────────────────►│
      │                                                      │
      └──── [Solenoid Valve 1] ◄──── [MOSFET Driver] ◄──────┤
                                                             │
[Hot Plate] ◄──── [SSR 1] ◄──── [NPN BJT Driver] ◄──────────┘
```

**Dryness Detection Logic:**
When pressure drops below ~1 PSI and stays there for 15 seconds (after an initial active cycle above 2 PSI), the system latches the "dry" flag and disables the heater SSR. This is based on the observed behavior that temperature rises sharply while pressure stays low once all moisture has been driven off.

**Valve Control Logic:**
The solenoid valve opens when pressure reaches 8 PSI and closes at 7 PSI (hysteresis band), releasing steam to the condenser/containment vessel.

**Temperature Control:**
A proportional controller (Kp = 0.03) targets 200°C with a 2-second PWM window. An exponential moving average (α = 0.10) smooths thermocouple readings.

---

## Hardware

| Component | Description |
|---|---|
| Reaction vessel | Standard pressure cooker |
| Microcontroller | ESP32 (breadboard) → ESP32-H2 DEVKITM-1-N4 (custom PCB) |
| Temperature sensor | K-type thermocouple + MAX31855 breakout (SPI) |
| Pressure sensor | Analog 0–100 PSI, read via ADC with voltage divider (0–5V → 0–3.3V) |
| Solenoid valve | 12V solenoid, driven by IRLB3813PBF N-channel MOSFET |
| Heater control | Solid-state relay (SSR 1), NPN BJT driver — active HIGH |
| Hot plate | External AC hot plate, switched via SSR |
| Custom PCB | Bio-Char Control Board v0.1.0 (KiCad 9, 2-layer FR4, ~$48 BOM) |

---

## Pin Configuration

The table below reflects the **working firmware pin assignments** as validated during PCB bring-up. These differ from the original KiCad schematic assignments — see the [Pin Change Notes](#pin-change-notes) section below.

| GPIO | Function | Direction | Notes |
|------|----------|-----------|-------|
| GPIO0 | MAX31855 CLK | Output | SPI clock |
| GPIO1 | MAX31855 CS | Output | SPI chip select |
| GPIO2 | MAX31855 DO | Input | SPI MISO — read-only, no MOSI needed |
| GPIO3 | Pressure Sensor ADC | Input | ADC1_CH3, 0–100 PSI via voltage divider |
| GPIO4 | SSR 1 / Heater | Output | **Active HIGH** — drives NPN BJT base to complete SSR circuit |
| GPIO5 | SSR 2 | Output | Reserved — not currently active (see firmware comments) |
| GPIO10 | Solenoid Valve 1 | Output | MOSFET (IRLB3813PBF) driven |
| GPIO11 | Solenoid Valve 2 | Output | Reserved — not currently active (see firmware comments) |

### Pin Change Notes

During PCB bring-up, the SSR control pins had to be remapped due to a conflict with the ESP32-H2's internal crystal peripheral:

- **GPIO13 and GPIO14** are internally tied to the ESP32-H2's 32 kHz RTC crystal oscillator. The original schematic routed SSR1 and SSR2 control lines to these pins. On Test 1 (05/06/2026), both SSR outputs were unresponsive — the GPIOs could not be driven as outputs while the crystal is active.
- As a Test 1 workaround, SSR1 and SSR2 were manually driven from **GPIO4 and GPIO5** (open pins on the board), confirming the BJT and SSR circuitry was otherwise functional.
- On 05/14/2026, the GPIO13 and GPIO14 traces were physically cut and jumper wires were soldered from GPIO4 → SSR1 and GPIO5 → SSR2 pads. This hardware rework makes the board match the firmware pin assignments permanently.
- The corrected routing will be incorporated into the next PCB design revision.

### Heater/SSR Logic Change

The original Arduino firmware used **active LOW** logic for the heater (GPIO4 LOW = SSR ON, sinking to GND via the SSR's internal optocoupler). The PCB design introduced an NPN BJT between the ESP32 GPIO and the SSR input, which inverts the signal:

```
GPIO4 HIGH → BJT base HIGH → collector pulls SSR input → SSR ON
GPIO4 LOW  → BJT OFF       → SSR input floating        → SSR OFF
```

The ESP-IDF firmware (`Firmware/main/main.c`) uses **active HIGH** accordingly. The `set_heater()` function drives GPIO4 HIGH to turn the heater on and LOW to turn it off.

---

## Known Errors and Issues

| # | Issue | Status | Notes |
|---|-------|--------|-------|
| 1 | ADC non-linearity at low raw values (<350) | Mitigated | Readings below ~0.5 PSI fluctuate. DRY_PRESSURE_MAX threshold (1.0 PSI) provides sufficient headroom for reliable detection. |
| 2 | MAX31855 returns NaN on first read after power-on | Handled | ~100ms settling time required. EMA initialisation stays NaN until a valid reading arrives — no false triggers. |
| 3 | GPIO3 shared with JTAG MTDI | Known | Disconnect JTAG debugger before running in production, or remap ADC to GPIO4/5 if debugging under sustained load. |
| 4 | GPIO13/GPIO14 RTC crystal conflict — SSR pins non-functional | Resolved (05/14/2026) | Original schematic routed SSR1/SSR2 to GPIO13/GPIO14, which are internally tied to the 32 kHz RTC crystal on the ESP32-H2. Confirmed on Test 1 (05/06): both SSR outputs were unresponsive. Test 1 workaround: SSR driven manually from GPIO4/GPIO5. Permanent fix: traces to GPIO13/GPIO14 cut and jumper wires soldered to GPIO4/GPIO5 pads. Next PCB revision will correct the routing. |
| 5 | D2/D5 diode voltage drop — solenoid safety switches not opening | Resolved (05/14/2026) | 1N4148 diodes on the solenoid valve safety switch circuits had a forward voltage drop high enough to prevent the gate logic from pulling low cleanly, blocking valve operation. Bypassing the diodes confirmed the MOSFET/solenoid path was functional. D2 and D5 replaced with 1N4001 diodes; valve operation re-verified successfully. |
| 6 | Bread charring — dryness detection did not trigger | Investigated | Bread had insufficient initial moisture to produce a distinct pressure signature during drying. Detection heuristic works correctly for water-heavy samples. |

---

## Experiment Timeline

See [`Progress Reports.md`](https://github.com/PubInv/Biochar-Toilet/blob/main/Project%20Files/Progress%20Reports.md) for full dated entries. Summary:

| Date | Milestone |
|---|---|
| Nov 21, 2025 | Pressure sensor schematic designed |
| Dec 5, 2025 | Initial pressure sensor physical calibration and error testing |
| Dec 11, 2025 | Dryness detection breadboard prototype — valve, pressure, dryness logic validated |
| Jan 16, 2026 | Microcontroller + solenoid valve + SSR hardware integration |
| Feb 13, 2026 | Bread charring test with auto-shutoff — end-to-end system validated |
| Mar 5–18, 2026 | Custom Control Board PCB design (v0.1.0) — Gerbers generated |
| May 6, 2026 | PCB Board Test 1 — PSI and dry detection verified; GPIO13/14 crystal conflict and D2/D5 diode drop identified |
| May 14, 2026 | Hardware rework — GPIO13/14 traces cut, SSR lines jumpered to GPIO4/5; D2/D5 replaced with 1N4001; valve operation re-verified |

---

## Key Results

**Dryness Detection:** Successfully confirmed. When pressure falls below 1 PSI for 15 sustained seconds following an active cycle, the system correctly identifies the dry state and disables the heater.

**Flash Decompression:** Validated. Opening the solenoid valve under pressure causes water to flash to steam. However, at ~15 PSI, the valve restriction prevented instantaneous pressure drop — steam flowed continuously rather than flashing off in a single burst.

**Bread Charring Test (Feb 13, 2026):** A loaf of bread was heated inside the sealed pressure cooker at ~110°C for approximately 2 hours and 40 minutes. The loaf was visibly charred. The dryness detection algorithm did not trigger in this run (see Known Errors #5). Soot accumulation on the chamber lid was observed, raising particulate management as a future design concern.

**Key Finding — Safety Limit:** Full charring temperatures cannot be safely reached inside the pressure cooker vessel. Lawrence Kincheloe proposed an inner charring cup (heated by a silicon nitride igniter) inside the pressure cooker, keeping the outer vessel cool. This informed the transition to Phase 2 (induction heating).

---

## Firmware

Two firmware versions exist for this prototype:

| File | Platform | Notes |
|------|----------|-------|
| `Firmware/main/main.c` | ESP-IDF v5.x | Current — active HIGH heater logic, reserved pins commented |
| *(Arduino reference)* | Arduino IDE | Original breadboard firmware; superseded by ESP-IDF version |

The ESP-IDF firmware (`Firmware/main/main.c`) implements all three control loops simultaneously:
- Pressure-based valve hysteresis (8 PSI on, 7 PSI off)
- Dry boiler detection (sustained low-pressure latch after active cycle)
- Proportional temperature control with EMA smoothing and 2-second PWM window

To build and flash:
```bash
idf.py set-target esp32h2
idf.py build
idf.py -p PORT flash monitor
```

Root-level Python scripts (`dryness_monitor.py`, `valve_controller.py`, and their tests) were earlier-stage software prototypes for the same logic.

---

## PCB Design

The Bio-Char Control Board v0.1.0 is documented in detail in the main project PCB Design folder. Key specs:

- **MCU:** ESP32-H2-DEVKITM-1-N4 (RISC-V 96MHz, WiFi 802.11n, BLE 5.2)
- **Power:** 12V input → R-78E5.0-1.0 switching regulator (5V) + 2A PTC fuse + reverse-polarity protection
- **Outputs:** 2× IRLB3813PBF MOSFET solenoid drivers, 2× NPN BJT-driven SSR outputs (active HIGH)
- **Sensors:** MAX31855 SPI thermocouple, analog pressure ADC with voltage divider
- **BOM cost:** ~$48 (Mouser, March 2026)
- **Manufacturing files:** Gerbers and drill files included

---

## Conclusions and Lessons Learned

1. Temperature–pressure divergence is a reliable proxy for sample dryness and works well as an autonomous shutoff trigger.
2. Valve restriction geometry matters significantly for flash decompression — a lower-restriction valve is needed for true instantaneous pressure drop.
3. Low-moisture samples (like dry bread) do not produce a clear pressure signature during drying, requiring either a different detection heuristic or moisture pre-conditioning.
4. Soot and particulate management inside the reaction vessel needs to be designed for from the start in future iterations.
5. Charring temperatures (>400°C) cannot be safely achieved in the pressure cooker configuration — an inner heated vessel is necessary.
6. The custom PCB successfully consolidates all sensing and control onto a single board, validating the embedded architecture for future phases.
7. Crystal-reserved GPIOs on the ESP32-H2 (GPIO13, GPIO14 for the 32 kHz RTC crystal) must be identified and avoided during schematic design — they appear usable in simulation but are unresponsive as outputs on hardware while the crystal is active.
8. 1N4148 diodes are inadequate for solenoid valve safety switch circuits at the currents involved — the forward voltage drop prevents clean gate logic switching. 1N4001 is the correct replacement.

---

## Files in This Folder

```
Conductive Heating Prototype/
├── README.md                        ← This file
└── Firmware/
    ├── CMakeLists.txt               ← ESP-IDF top-level build file
    └── main/
        ├── CMakeLists.txt           ← Component registration
        └── main.c                   ← ESP-IDF integrated control firmware
```

Hardware files (schematics, PCB design, test videos) are in the corresponding subfolders of the main project directory and cross-referenced above.
