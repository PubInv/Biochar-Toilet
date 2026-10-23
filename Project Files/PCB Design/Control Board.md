# Bio-Char Control Board v0.3.0

**ESP32-H2 Based Industrial Control System for Bio-Char Production**

---

## 📋 Overview

The Bio-Char Control Board is an industrial-grade PCB designed to control and monitor bio-char production processes. It features WiFi/Bluetooth connectivity, multi-voltage power management, high-power output control, and sensor interfaces.

### Key Features
- **ESP32-H2 Microcontroller** - WiFi 802.11n + Bluetooth 5.2
- **Multi-Voltage Power System** - 12V input, regulated 5V and 3.3V rails
- **High-Power Output Control** - 2× Solenoid valves / High Current MOSFET outputs, 2× SSR outputs
- **Sensor Interfaces** - K-type thermocouple, analog pressure sensor
- **Manual Override** - Physical switches for safety
- **Professional Design** - 2-layer PCB, through-hole components for easy assembly

---

## 🔧 Hardware Specifications

### Power Supply
- **Input:** 12V DC (via Phoenix terminal block)
- **5V Rail:** R-78E5.0-1.0 switching regulator (1A, 95% efficiency)
- **3.3V Rail:** ESP32-H2 internal LDO
- **Protection:** 2A PTC resettable fuse, reverse polarity protection

### Microcontroller
- **IC:** Espressif ESP32-H2-DEVKITM-1-N4
- **Connectivity:** WiFi 802.11b/g/n (2.4GHz), BLE 5.2, Thread/Zigbee capable
- **Flash:** 4MB
- **RAM:** 272KB SRAM
- **Processor:** RISC-V 96MHz

### Output Control
- **Solenoid Valves / High Current Outputs:** 2× IRLB3813PBF N-channel MOSFETs (30V, 100A, 2.3mΩ)
- **SSR Control:** 2× NPN transistor drivers (2N3904)
- **Protection:** Flyback diodes on all inductive loads

### Sensor Interfaces
- **Thermocouple:** MAX31855 breakout module (K-type, -200°C to +700°C, SPI)
- **Pressure Sensor:** Analog input with voltage divider (0-5V → 0-3.3V)

### Connectors
- **Phoenix Contact Terminals:** 5.08mm pitch, 90° horizontal
  - 1× 3-position (pressure sensor)
  - 5× 2-position (power input, solenoids, SSRs)
- **Pin Headers:** 2.54mm pitch
  - 1× 8-pin (general I/O)
  - 1× 6-pin (MAX31855 interface)

---

## 📦 Bill of Materials (BOM) & System Variants

### Control Board PCB Assembly BOM

**Complete BOM available:** [`Bio_Char_BOM_Mouser_FINAL.xlsx`](Bio_Char_BOM_Mouser_FINAL.xlsx)

#### BOM Summary
- **Total Component Types:** 18
- **Total Components:** 40 items
- **Total Cost:** $48.12 (verified Mouser pricing as of March 2026)
- **Supplier:** Mouser Electronics

#### Key PCB Components

| Category | Component | Part Number | Qty | Unit Price |
|----------|-----------|-------------|-----|------------|
| **Microcontroller** | ESP32-H2-DEVKITM-1-N4 | 356-ESP32H2DKITM1-N4 | 1 | $9.38 |
| **Power** | R-78E5.0-1.0 Regulator | 919-R-78E5.0-1.0 | 1 | $3.97 |
| **Power** | Polyfuse 2A | 530-0ZRN0200FF1E | 1 | $0.42 |
| **MOSFETs** | IRLB3813PBF | 942-IRLB3813PBF | 2 | $2.16 |
| **Transistors** | 2N3904BU | 512-2N3904BU | 2 | $0.28 |
| **Diodes** | 1N4001RLG | 863-1N4001RLG | 2 | $0.13 |
| **Diodes** | 1N4148 | 512-1N4148 | 2 | $0.10 |
| **Diodes** | 1N5819RLG | 863-1N5819RLG | 1 | $0.39 |
| **Resistors** | 1kΩ 1/4W | 603-CFR-25JR-521K | 6 | $0.18 |
| **Resistors** | 100kΩ 1/4W | 603-CFR-25JB-52-100K | 4 | $0.18 |
| **Resistors** | 10kΩ 1/4W | 603-CFR-25JB-52-10K | 2 | $0.10 |
| **Resistors** | 22kΩ 1/4W | 603-CFR-25JB-52-22K | 2 | $0.10 |
| **Capacitors** | 100nF Ceramic | 80-C322C104K5R5TA | 3 | $0.87 |
| **Capacitors** | 47µF 25V Electrolytic | 598-476CKH025M | 2 | $0.21 |
| **Terminals** | Phoenix 2-pos 5.08mm | 651-1715721 | 5 | $1.30 |
| **Terminals** | Phoenix 3-pos 5.08mm | 651-1715734 | 1 | $1.66 |
| **Headers** | Pin Headers 2.54mm | 649-1012937990801BLF | 2 | $0.14 |
| **Modules** | MAX31855 Breakout | 485-269 | 1 | $14.95 |

**Note:** MOSFET upgrade from IRLB8721PBF to IRLB3813PBF provides better performance (lower Rds(on), higher current rating).

---

### System Peripheral BOM: Conductive vs. Induction Variants

The control board connects to peripheral hardware specific to each prototype generation:

#### Phase 1: Conductive Heating System Peripherals
| Subsystem | Component | Specifications | Connection |
|-----------|-----------|----------------|------------|
| **Heater** | Single Resistive Hot Plate | 120V / 1000W Joule element | SSR 1 Output (GPIO4) |
| **Pressure Sensor** | 0–100 PSI Transducer | 0.5–4.5V output | ADC Input (GPIO3) via divider |
| **Valves** | 1× Solenoid Valve | U.S. Solid 1" SS 12V DC | Solenoid 1 MOSFET (GPIO10) |
| **Temperature** | K-Type Thermocouple | Single sheath in vessel wall | MAX31855 SPI (GPIO0/1/2) |

#### Phase 2: Dual Induction Heating System Peripherals
| Subsystem | Component | Specifications | Connection |
|-----------|-----------|----------------|------------|
| **Main Heater** | 1800W ZVS Induction Unit | 12V–48V DC, hollow copper vessel coil | SSR 1 Output (GPIO4) |
| **Catalyst Heater** | ZVS Induction Unit | 12V–48V DC, catalytic converter zone | SSR 2 Output (GPIO5) |
| **Pressure Sensor** | 0–15 PSI Transducer | 0–5V output (high resolution) | ADC Input (GPIO3) via divider |
| **Valves** | 1× Solenoid Valve | U.S. Solid 1" SS 12V DC | Solenoid 1 MOSFET (GPIO10) |
| **Fluid Cooling** | Recirculating Pump | 110V / 12V DC water circulator | Solenoid 2 MOSFET (GPIO11) |
| **Temperature** | K-Type Thermocouple | Single sheath in vessel side wall | MAX31855 SPI (GPIO0/1/2) |

---

## 📐 PCB Specifications

### Physical Dimensions
- **Board Size:** TBD (verify in KiCad)
- **Layers:** 2 (Top + Bottom copper)
- **Thickness:** 1.6mm standard FR4
- **Copper Weight:** 1oz (35µm)

### Design Rules
- **Minimum Track Width:** 0.25mm (signal), 2.0mm (power)
- **Minimum Clearance:** 0.25mm
- **Via Size:** 0.6mm drill, 1.0mm pad (standard)
- **Surface Finish:** HASL or ENIG recommended

### Manufacturing Files
All necessary manufacturing files are included in this repository:
- **Gerber Files/** - Complete gerber set for PCB fabrication
- **Drill Files/** - NC drill files for hole placement

---

## 🔌 Pin Assignments & Firmware Variants

### 1. Original Conductive Heating Firmware Variant (`Project Files/Conductive Heating Prototype/Firmware`)

| GPIO | Function | Hardware Connection | Notes |
|------|----------|---------------------|-------|
| GPIO0 | MAX31855 CLK | SPI Clock | Thermocouple SPI |
| GPIO1 | MAX31855 CS | SPI Chip Select | Thermocouple SPI |
| GPIO2 | MAX31855 DO | SPI Data Out (MISO) | Thermocouple SPI |
| GPIO3 | ADC Pressure | ADC1_CH2 Input | 0–100 PSI Sensor via voltage divider |
| GPIO4 | SSR 1 Control | Q2 base → SSR 1 | Main Resistance Hot Plate |
| GPIO5 | SSR 2 Control | Q5 base → SSR 2 | Reserved / Secondary |
| GPIO10 | Solenoid 1 Control | Q1 MOSFET Driver | Main Vent Solenoid Valve 1 |
| GPIO11 | Solenoid 2 Control | Q3 MOSFET Driver | Reserved / Valve 2 |

### 2. Dual Induction Heating Firmware Variant (`Project Files/Induction Heating Prototype/Firmware`)

| GPIO | Function | Hardware Connection | Notes |
|------|----------|---------------------|-------|
| GPIO0 | MAX31855 CLK | SPI Clock | Thermocouple SPI |
| GPIO1 | MAX31855 CS | SPI Chip Select | Thermocouple SPI |
| GPIO2 | MAX31855 DO | SPI Data Out (MISO) | Thermocouple SPI |
| GPIO3 | ADC Pressure | ADC1_CH2 Input | 0–15 PSI (0–5V) Sensor via voltage divider |
| GPIO4 | Main Coil Heater | Q2 base → SSR 1 | Main Vessel Induction Coil (PWM controlled) |
| GPIO5 | Catalyst Heater | Q5 base → SSR 2 | Catalytic Converter Induction Heater (Active during cycle) |
| GPIO8 | Status LED Driver | WS2812 RGB LED | Onboard RGB LED indicator (Green=IDLE, Orange=ACTIVE, Blue=DRY, Red Flashing=TRIP) |
| GPIO10 | Solenoid 1 Control | Q1 MOSFET Driver | Main Vent Solenoid Valve 1 |
| GPIO11 | Recirculating Pump | Q3 MOSFET Driver | Water/Fluid Recirculating Pump (Feature flagged) |
| GPIO12 | Flush Button Input | Manual Push Switch | Active LOW input (internal pull-up) — short-press starts cycle, 3s long-press resets emergency trip |

*Note: GPIO8/9 are internally tied to the ESP32-H2 32 MHz crystal oscillator and cannot be used for general I/O. GPIO14/15 are reserved for the 32 kHz RTC crystal.*

---

## ⚡ Power Distribution

### Power Tree
```
12V Input (J1)
  │
  ├─[F1: Polyfuse 2A]─┬─[D1: 1N4001RLG]─┬── +12V Rail
  │                   │                  │   (Solenoids, Pumps, SSR drivers)
  │                   │                  │
  │                   │                  └── [U3: R-78E5.0-1.0 Input] ──> 5V Rail
  │                   └─[C6: 47µF 25V]
  │
  └── ESP32 LDO ───> 3.3V Rail (Internal peripherals)
```

### Current Budget
- **12V Rail:** ~3-4A max (solenoids + regulator input)
- **5V Rail:** ~1A max (ESP32 + MAX31855)
- **3.3V Rail:** ~500mA max (ESP32 internal)

---

## 🛠️ Assembly Instructions

### Component Placement Order
1. **Passive Components First:** Resistors, diodes, capacitors
2. **IC Sockets (if used):** For U1, U3 (optional)
3. **Power Components:** F1, D1, U3, Q1, Q3
4. **Signal Components:** Q2, Q5, remaining passives
5. **Connectors:** Phoenix terminals, pin headers
6. **Modules Last:** ESP32-H2, MAX31855 (after testing power rails)

### Soldering Tips
- Use temperature-controlled soldering iron (350°C)
- Start with low-profile components
- Double-check polarized components (diodes, electrolytic caps)
- Test power rails before installing ESP32 and MAX31855

### Testing Procedure
1. **Visual Inspection:** Check for solder bridges, cold joints
2. **Power Test:** Apply 12V, verify 5V output (no load)
3. **Current Test:** Measure idle current (<50mA expected)
4. **ESP32 Test:** Upload basic blink sketch
5. **Sensor Test:** Read MAX31855 and pressure sensor
6. **Output Test:** Test MOSFET switching with load

---

## 📂 Repository Structure

```
PCB Design/
├── Bio_Char_BOM_Mouser_FINAL.xlsx    # Bill of Materials
├── Control Board.kicad_sch            # Schematic
├── Control Board.kicad_pcb            # PCB Layout
├── Control Board.kicad_pro            # Project File
├── Control Board.kicad_prl            # Local Settings
├── Control Board.md                   # This file
├── PCB_v0.4.0_Induction_Revision_Plan.md # Hardware Roadmap for v0.4.0
├── Gerber Files/                      # Manufacturing Files
│   ├── F_Cu.gbr                       # Top Copper
│   ├── B_Cu.gbr                       # Bottom Copper
│   └── ...                            # Additional layers
└── Drill Files/                       # NC Drill Files
```

---

## 🚀 Getting Started

### Prerequisites
- KiCad 9.0 or later
- Basic understanding of electronics
- Soldering equipment
- 12V DC power supply

### Ordering PCBs
1. Download gerber files from `Gerber Files/` folder
2. Upload to PCB manufacturer (JLCPCB, PCBWay, OSH Park, etc.)
3. Recommended settings:
   - Layers: 2
   - Thickness: 1.6mm
   - Copper: 1oz
   - Surface Finish: HASL or ENIG
   - Soldermask: Green (or your preference)

### Ordering Components
1. Open `Bio_Char_BOM_Mouser_FINAL.xlsx`
2. All Mouser part numbers are verified and ready to order
3. Total cost: ~$48 (as of March 2026)
4. Order from: [Mouser Electronics](https://www.mouser.com)

---

## 🔍 Design Decisions

### Why ESP32-H2?
- Native WiFi and Bluetooth connectivity
- Thread/Zigbee support for future smart home integration
- Adequate GPIO for all control and sensing needs
- Low power consumption
- Active community support

### Why Through-Hole Components?
- Easier hand assembly for prototypes
- Better for educational purposes
- More robust mechanical connections
- Easier to debug and repair
- Suitable for industrial environments

### Why IRLB3813PBF MOSFETs?
- Upgrade from original IRLB8721PBF
- Lower Rds(on) (2.3mΩ vs 8.7mΩ) = less heat
- Higher current rating (100A vs 62A) = more headroom
- Logic-level gate (works with 3.3V ESP32)
- Better availability

---

## ⚠️ Safety Considerations

### Electrical Safety
- **Input Fuse:** PTC resettable fuse provides overcurrent protection
- **Reverse Polarity:** D1 protects against reverse voltage
- **Flyback Protection:** Diodes on all inductive loads
- **Isolation:** Keep 12V and 3.3V circuits separated

### Operating Conditions
- **Temperature Range:** 0°C to 60°C ambient
- **Humidity:** <85% RH (non-condensing)
- **Ventilation:** Ensure adequate airflow around regulator

### Installation
- Use proper gauge wire for 12V input (18 AWG minimum)
- Secure all screw terminals tightly
- Mount board in protective enclosure
- Keep away from moisture and extreme temperatures

---

## 📝 Version History

### v0.3.0 (March 2026) - Current
- Complete PCB layout with routing
- Verified BOM with Mouser pricing
- Manufacturing files (Gerbers + Drills) generated
- MOSFET upgrade to IRLB3813PBF
- Dual-variant documentation (Conductive vs Induction)

### v0.2.0
- Schematic refinement
- Component selection and footprint assignment

### v0.1.0
- Initial schematic design
- Basic architecture

---

## 👤 Author

**hardhik03**
- GitHub: [@hardhik03](https://github.com/hardhik03)

---

**Last Updated:** March 18, 2026
**KiCad Version:** 9.0.7
**Board Revision:** v0.3.0
