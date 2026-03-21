# Bio-Char Control Board v0.3.0

**ESP32-H2 Based Industrial Control System for Bio-Char Production**

---

## 📋 Overview

The Bio-Char Control Board is an industrial-grade PCB designed to control and monitor bio-char production processes. It features WiFi/Bluetooth connectivity, multi-voltage power management, high-power output control, and sensor interfaces.

### Key Features
- **ESP32-H2 Microcontroller** - WiFi 802.11n + Bluetooth 5.2
- **Multi-Voltage Power System** - 12V input, regulated 5V and 3.3V rails
- **High-Power Output Control** - 2× Solenoid valves, 2× SSR outputs
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
- **Solenoid Valves:** 2× IRLB3813PBF N-channel MOSFETs (30V, 100A, 2.3mΩ)
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

## 📦 Bill of Materials (BOM)

**Complete BOM available:** [`Bio_Char_BOM_Mouser_FINAL.xlsx`](Bio_Char_BOM_Mouser_FINAL.xlsx)

### BOM Summary
- **Total Component Types:** 18
- **Total Components:** 40 items
- **Total Cost:** $48.12 (verified Mouser pricing as of March 2026)
- **Supplier:** Mouser Electronics

### Key Components

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

## 🔌 Pin Assignments

### ESP32-H2 GPIO Mapping

| GPIO | Function | Connection | Notes |
|------|----------|------------|-------|
| GPIO0 | Solenoid 1 Control | Q2 base (via R1) | Drive Q1 MOSFET |
| GPIO1 | Solenoid 2 Control | Q5 base (via R7) | Drive Q3 MOSFET |
| GPIO2 | SSR1 Control | Direct output | Via current limit R3 |
| GPIO3 | SSR2 Control | Direct output | Via current limit R9 |
| GPIO4 | ADC Input | Pressure sensor | Via voltage divider R14/R15 |
| GPIO5 | MAX31855 CS | SPI Chip Select | Thermocouple interface |
| GPIO6 | MAX31855 CLK | SPI Clock | Thermocouple interface |
| GPIO7 | MAX31855 DO | SPI Data Out | Thermocouple interface |
| TBD | Manual Override 1 | Switch SW1 | Optional solenoid 1 |
| TBD | Manual Override 2 | Switch SW2 | Optional solenoid 2 |

---

## ⚡ Power Distribution

### Power Tree
```
12V Input (J1)
  │
  ├─[F1: Polyfuse 2A]─┬─[D1: 1N4001RLG]─┬── +12V Rail
  │                   │                  │   (Solenoids, SSRs)
  │                   │                  │
  │                   │                  └── [U3: R-78E5.0-1.0 Input]
  │                   │                         │
  │                   └─[C6: 47µF 25V]          │
  │                                             │
  │                                             ├── 5V Rail
  │                                             │   ├── ESP32-H2 VCC
  │                                             │   └── MAX31855 VIN
  │                                             │
  │                                             └─[C7: 47µF 16V]
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
├── Gerber Files/                      # Manufacturing Files
│   ├── F_Cu.gbr                       # Top Copper
│   ├── B_Cu.gbr                       # Bottom Copper
│   ├── F_Mask.gbr                     # Top Soldermask
│   ├── B_Mask.gbr                     # Bottom Soldermask
│   ├── F_Silkscreen.gbr               # Top Silkscreen
│   ├── B_Silkscreen.gbr               # Bottom Silkscreen
│   ├── Edge_Cuts.gbr                  # Board Outline
│   └── ...                            # Additional layers
└── Drill Files/                       # Drill Files
    └── ...                            # NC drill files
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
- Comprehensive documentation

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
