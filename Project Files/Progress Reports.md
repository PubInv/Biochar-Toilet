## **[05/14/2026]**

**PCB Control Board – Trace Rework and Hardware Fixes**

Following Test 1 findings, Hardhik performed hardware rework on the Bio-Char Control Board (v0.1.0) to resolve the two issues identified on 05/06:

GPIO13 and GPIO14 traces to the SSR control lines were physically cut on the PCB. Jumper wires were soldered from GPIO4 and GPIO5 directly to the SSR1 and SSR2 pads respectively, bypassing the original routing. This brings the hardware into alignment with the current firmware pin assignments. The trace cut and remap will be incorporated into the next PCB design revision.

Diodes D2 and D5 (1N4148) on the solenoid valve safety switch circuits were desoldered and replaced with 1N4001 diodes. The 1N4148's forward voltage drop under the switch current was high enough to prevent the safety switches from pulling the gate logic low cleanly, keeping the solenoid valves from opening. The 1N4001's lower drop at higher currents resolved this. Valve operation was re-verified after the swap and both solenoids opened and closed correctly.

These changes are hardware-only, no firmware modifications were required.





**PCB Control Board – Initial Testing (Test 1)**

Hardhik conducted the first round of physical tests on the assembled Bio-Char Control Board (v0.1.0). The test session covered four areas of the board's integrated control firmware, but also uncovered two hardware issues that required rework.

PSI readings from the analog pressure sensor were confirmed correct across the expected range. ADC-to-PSI calibration constants (ADC_ZERO = 320, ADC_FULL = 3008) were validated against a reference gauge. The dry-boiler detection logic also worked as expected — after a sustained pressure drop below 1 PSI for 15 seconds following an active cycle, the dryLatched flag triggered and the heater SSR was disabled correctly. A full-system Serial readout at 115200 baud was recorded showing pressure, valve, heater, and temperature simultaneously.

Two hardware faults were found:

**SSR pins non-functional (GPIO13/GPIO14 crystal conflict):** GPIO13 and GPIO14 on the ESP32-H2 are internally tied to the 32 kHz RTC crystal oscillator and cannot be used as general-purpose outputs. The original schematic routed the SSR1 and SSR2 control lines to these pins. As a workaround for Test 1, the SSR lines were driven from GPIO4 and GPIO5 (open pins), confirming the SSR and BJT circuitry itself works correctly once a valid GPIO is used. A permanent trace rework was scheduled.

**Solenoid valve safety switches not opening (D2, D5 diode voltage drop):** Diodes D2 and D5 (1N4148) on the solenoid valve safety switch circuits had a higher-than-expected forward voltage drop under the switch current, which prevented the gate logic from pulling low cleanly and kept the valves from opening. The valves were tested by bypassing the diodes to confirm the MOSFET and solenoid circuits were otherwise functional. Replacement with 1N4001 diodes was scheduled.

Valve hysteresis (8 PSI open, 7 PSI close) was verified using the GPIO4/GPIO5 workaround after identifying the GPIO13/GPIO14 issue.





## **[04/24/2026]**
Lawrence and Peter ran a series of experiments with different amounts of material that we want to use for induction heating. 
The temperature measurements are located in [Induction experimental data.md](https://github.com/PubInv/Biochar-Toilet/blob/main/Induction experimental data.md)
Interestng results were that we didn't hit the curie temperature of the carbon steel, as evident by the thermal readings and the presence of a magnetic field. Based on
color charts for steel and our measurements, we estimate we got up between 900-1000 degrees F for the pure ball bearing. 

Quick Observations
The Magnetite Advantage: At 5 grams, the Magnetite is significantly more efficient at converting the induction field to heat compared to the same mass of Carbon Steel bearings (reaching 740°F vs 130°F in 60 seconds).

Volume vs. Mass: The Carbon Steel only starts to compete in peak temperature (940°F) when the volume is matched to the Magnetite, suggesting density and surface area are playing a massive role here. 
It is believed that this is due to eddie currents, which seem to favor lengh wise induced current when the material is making good electrical contact. 

The "Mixed" Thermal Inertia: The mixture shows a much more gradual and staggered climb, likely due to the Bentonite acting as a thermal buffer or insulator between the active heating elements.
Further adjustments to the mix will need to be determined to balance thermal heat transfer, rapid heating, and options for agitation. 

Peter will drill out the holes for the induction heater next week, and we are continuing to debug issues with the higher power induction coil. 
Right now we suspect the issue may be the spacing on the coils (currently 10-30mm), but it's interesting to note that coil geometry has a large effect on heating potential. 
We may need to accelerate our understanding of coil geometry and options for focusing the magnetic field towards the center of the crucible.





## **[03/27/2026]**
Lawrence and Peter ran an experiment attempting to char cheese puffs. The cheese puffs are about 8mm in diameter, 
and there were about 6 of them in the crucible, moistened with water and coated in mostly magnetite with some kitty litter.
The induction coil system had been running for about 5 minutes when the mixture began smoking. The max temperature attained was 833F before the team put a cap on the 
system at the 9 minute marker (an arbitrary amount of time).

Once the system had cooled to safe handling temperature, the team opened the lid to discover the results were good. All but one of the cheese puffs all charred all 
the way through. This experiment was conducted in the open air (oxygen-rich) so pyrolysis was not as efficient, but there WAS evidence of some pyrolysis happeneing, 
due to the residue on the crucible cap.

The total powered time of this experiment was about 16 minutes. 





## **[03/20/2026]**
Lawrence and Peter had identified and (Lawrence) updated the project BOM with the bulkhead fittings and hardware required for the induction coil 
system to insulate the energized copper from the aluminum pot, as well as to seal the fitting and lock the insulated sleeve in place using a 
furrel nut and insolating washer. They also identified 440 stainless steel shot 2mm as being an excellent starting point for mixing with
the magnetite.

Peter had digitized the bounding box of the induction module (for clearance) and the model of the 1" air line's bulkhead fitting;
he also imported STEP files of the PSU and T-Pipe fitting. The purpose for this is to create proper mounting systems contining the 
components to make a solid, safe, transportable unit for demonstration and experimentation.





## **[03/18/2026]**

**Bio-Char Control Board PCB – v0.1.0 Design Complete**

Hardhik completed the design and manufacturing file generation for the Bio-Char Control Board v0.1.0, an ESP32-H2-based industrial control PCB purpose-built for the bio-char production system. The design work ran from March 5 through March 18, spanning 14 KiCad revision checkpoints.

The board integrates the full sensor and control stack onto a single 2-layer FR4 PCB:
- **Microcontroller:** ESP32-H2-DEVKITM-1-N4 (RISC-V 96MHz, WiFi 802.11n, BLE 5.2, Thread/Zigbee capable)
- **Power management:** 12V input → R-78E5.0-1.0 switching regulator (5V, 95% efficiency) with 2A PTC resettable fuse and reverse-polarity protection diode
- **High-power outputs:** 2× IRLB3813PBF N-channel MOSFETs for solenoid valve control (upgraded from IRLB8721PBF for lower Rds(on) of 2.3mΩ and higher current headroom); 2× NPN transistor-driven SSR outputs
- **Sensors:** MAX31855 K-type thermocouple breakout (SPI, −200°C to +700°C) and analog pressure sensor input with voltage divider (0–5V → 0–3.3V)
- **Safety:** Flyback diodes on all inductive loads, manual override switches for both solenoids, Phoenix Contact screw terminals throughout

The complete BOM was finalized and verified against Mouser Electronics pricing: 40 components across 18 types at a total cost of ~$48 (as of March 2026). Gerber files and NC drill files were generated and are ready for fabrication at JLCPCB, PCBWay, or OSH Park using standard 2-layer, 1.6mm FR4, 1oz copper settings.





## **[03/06/2026]**
Lawrence and Peter were able to fill a crucible with 100% magnetite and a cup of water, then using only the induction coil 
system, was able to evaporate all of the water in less than 8 minutes, starting with the sample at room temperature.
Next steps towards biochar include testing different ratios of kitty litter (bentonite), magnetite, and mild steel shot to not only
evaporate water quickly, but to char an organic sample. 
Data will be collected then analyzed via spreadsheet graph comparison.





## **[02/13/2026]**

**Dryness Detection + Auto Shutoff – Bread Charring Test**

Hardhik ran a combined dryness-detection and automatic shutoff test using bread as the organic sample. The system consisted of the microcontroller connected to a solenoid valve and an SSR-controlled hot plate. Key hardware integrations documented in this session included the SSR wired to the hot plate and the microcontroller-to-valve connection.

The bread was placed in the charring vessel and the system was powered on. The hot plate heated the sample while the pressure sensor monitored internal pressure. The dryness detection logic — which tracks a sustained low-pressure window after an initial active cycle — successfully identified when the material had dried sufficiently and triggered the auto shutoff, cutting power to the hot plate via the SSR.

The charred bread result showed visible and complete charring throughout the sample. This confirmed that the integrated dryness detection and automatic shutoff pipeline works end-to-end: the system can autonomously stop heating once the organic material is processed, without requiring manual intervention.

Video evidence of the test process and the final charred bread result were captured for documentation.





## **[01/16/2026]**

**Dryness Detection + Valve Auto Shutoff – Hardware Integration**

Hardhik integrated the microcontroller, solenoid valve, and SSR into a working prototype for the dryness detection and auto-shutoff system. Prior breadboard work was assembled into a more connected configuration, with the valve and SSR now driven directly by the ESP32 GPIO outputs.

Multiple video recordings were captured documenting the valve actuation behavior, the sensor feedback loop, and overall system operation. The system demonstrated the ability to open and close the solenoid valve based on pressure thresholds, while independently managing the heater SSR through the temperature control loop — operating both control loops concurrently without interference.

This session established the core firmware architecture (pressure-based valve hysteresis + dry detection + proportional temperature control) that later became the basis for the PCB firmware.





## **[12/11/2025]**
**Dryness Detection – Breadboard Prototype**

Hardhik built and tested an initial breadboard prototype for dryness detection. The circuit connected a pressure sensor and solenoid valve to an ESP32 microcontroller to test the concept of detecting when a heating cycle has driven off all moisture from a sample.

Key tests conducted in this session:

- **Dryness Detection:** The system successfully distinguished between an "active" pressurized state and a "dry" low-pressure state, validating the logic of using pressure as a proxy for remaining moisture in the system.
- **Pressure Sensor Check:** The sensor readings were verified on the breadboard against known pressure values.
- **Valve Turning ON / OFF:** The solenoid valve was confirmed to open and close correctly under GPIO control at the expected pressure thresholds.

The breadboard circuit provided confidence in the sensing and actuation approach before moving toward a more permanent hardware integration.





## **[12/05/2025]**
**Pressure Sensor – Initial Physical Testing**

Following the pressure sensor schematic work (Nov 21), Hardhik conducted the first physical pressure sensor tests. Two recordings were captured: one showing successful pressure sensor calibration against a reference, and one documenting a sensor error condition (likely an out-of-range or wiring issue) to characterize failure modes.

The calibration session established the ADC-to-PSI mapping that would later be formalized in firmware (ADC_ZERO ≈ 320, ADC_FULL ≈ 3008 for 0–100 PSI full-scale). Understanding the error behavior of the sensor was an important step for designing reliable dryness detection logic in subsequent firmware iterations.





## **[11/21/2025]**
**Pressure Sensor – Schematic Design**

Hardhik designed the initial pressure sensor schematic for the Phase 1 prototype. The schematic documents the sensor wiring, voltage divider configuration (0–5V → 0–3.3V for ESP32 ADC compatibility), and connector pinout. This served as the hardware reference for subsequent physical testing and was later incorporated directly into the Control Board PCB design.





## **[11/07/2025]**
**Flash System Prototype - Test 1**

On Nov.7th an initial prototype was tested. This consisted of a pressure cooker containing a cup of water,
with a thermocouple added and a hose connected to an electronic valve, leading to a condenser.

This experiment was largely confirmatory of our approach, but we learned certain things:
1. At high power and about 15 psi, the steam does not "flash" off. When the valve is opened, the
steam escapes, but it keeps flowing. Our valve apparently produces so much restriction that the
pressure does not drop quickly (though it does drop, but remains positive).
2. The temperature did rise DRASTICALLY when the water dried up. Our theory that we can detect this by the time that the pressure ceases to rise and the temperature does rise is well established.
3. A small amount of steam leaked through my high-temperature epoxy around the thermocouple.

So in principle, this was a successful test.
