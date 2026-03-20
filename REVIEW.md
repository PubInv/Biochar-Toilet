# Project Review: Biochar Toilet Prototype

## 1. Project Summary

**Goal:** Develop a low-cost, low-energy Biochar Toilet capable of quickly reducing human waste to sanitary biochar, primarily targeting impoverished communities lacking sewage infrastructure.

**Methodology:**
- **Phase 1 (Current):** A prototype using a pressure cooker to validate the core physical processes: heating, pressurization, "flash" decompression for sterilization and cell destruction, and dryness detection.
- **Process:** The system seals the waste and heats it until a target pressure (e.g., 15 PSI) and temperature (e.g., 121°C) are reached. A valve then opens to "flash" the water into steam, sterilizing the sample. Once the sample is dry (detected when temperature rises without a corresponding pressure increase), it is further heated to charring temperatures.
- **Components:** The current system uses a pressure sensor, a thermocouple, a solid-state relay (SSR) for a hot plate, and a solenoid valve. The future design envisions an induction-heated, internally-fitting oval stainless steel pressure vessel with a magnetite catalyst.

## 2. Code Review (Bugs & Improvements)

The repository contains two main logic components: `valve_controller.py` and `dryness_monitor.py`, along with their respective test suites.

### A. Valve Controller (`valve_controller.py`)
- **Status:** The `README.md` and `TODO.md` mention a "small bug controlling the valve."
- **Analysis:**
  - The `update(pressure, temperature)` method checks if `pressure >= self.target_pressure` AND `temperature >= self.target_temperature` to set `self.valve_open = True`.
  - **Bug/Issue:** The latching logic is implemented correctly (once open, it stays open unless `reset()` is called). However, the logic requires *both* pressure and temperature to meet or exceed targets simultaneously. In a real physical system, if the pressure hits 15 PSI before the temperature reaches 121°C, or vice-versa, the valve might not open when intended. Furthermore, the "flash" decompression relies on rapid pressure drop, but if the valve opens and pressure drops, it remains latched open (as intended for flashing), but there is no logic to *close* the valve again after flashing to build pressure for a subsequent cycle or for charring.
  - **Improvement:** The controller likely needs a more complex state machine (e.g., `HEATING`, `FLASHING`, `DRYING`, `CHARRING`) rather than a simple boolean `valve_open` latch. It needs a mechanism to close the valve after a set time or when pressure drops below a safe threshold.

### B. Dryness Monitor (`dryness_monitor.py`)
- **Status:** Implements logic to detect dryness based on diverging temperature and pressure trends.
- **Analysis:**
  - The `is_dry()` method calculates rates over a sliding window (`history`).
  - **Bug/Issue:** The rate calculation uses `end['time'] - start['time']`. However, if `add_reading` is called rapidly with the same timestamp, it could cause a division by zero. The code currently checks `if time_diff <= 0: return False`, which prevents the crash but silently ignores the readings.
  - **Bug/Issue:** The rate calculation is highly sensitive to the window size (`history_len`). If `history_len=10` and readings are taken every millisecond, the time window is only 10ms, making the rate calculation extremely noisy. The tests simulate readings 1 second apart, but real sensor data might be faster and noisier.
  - **Improvement:** Consider using a moving average or a low-pass filter for temperature and pressure before calculating the derivative. Also, ensure timestamps are strictly monotonically increasing or enforce a minimum time delta between recorded readings.

## 3. Design Review (Mechanics & Safety)

The design concepts outlined in `DESIGN_IDEAS.md` and modeled in `pressure_vessel.scad` show significant thought towards industrial robustness.

### A. Pressure Vessel Architecture
- **"Inside-Fitting" Oval Manway:** This is an excellent, proven design for pressure vessels. It naturally uses internal pressure to enhance the seal and eliminates complex locking mechanisms that could foul with dust. The addition of a single internal hinge for semi-automation is a practical refinement.
- **"Potted" Internal Coil:** Moving the induction coil inside the vessel (cast in high-temp refractory cement) solves the Faraday Cage effect and prevents the steel shell from overheating. This is a critical and sound design choice.

### B. Catalyst & Exhaust System
- **Magnetite Catalyst:** Using magnetite for both CO oxidation and De-NOx (SCR) using the ammonia naturally present in urine is highly innovative.
- **Heated "Reaction Tower":** Placing the catalyst in a vertical tower above the steam cleaning valve, heated by a central cartridge heater, is an efficient way to ensure the exhaust gases reach the necessary reaction temperatures (~350°C+).

### C. Safety Concerns & Considerations
- **Pop-off Valve:** The `TODO.md` correctly identifies the need for an emergency pop-off valve. The `pressure_vessel.scad` includes a "Pressure Relief Valve" on the lid, which is crucial. This must be a mechanical, fail-safe valve, completely independent of the software or microcontroller.
- **High-Amperage Feedthrough:** Routing high-power induction coil connections through a pressurized, wet, and chemically aggressive environment (urine/steam) is a major engineering challenge. The feedthroughs must be hermetically sealed, electrically isolated, and capable of withstanding thermal cycling. The `.scad` model shows side ports for this, overriding earlier ideas of lid routing.
- **Condenser/Containment:** The `README.md` notes that flashing steam must enter a separate containment chamber to cool and liquefy safely. This is vital to prevent scalding operators.

## 4. Next Steps & Recommendations

Based on the review, the immediate next steps should be:

1. **Fix the Valve Controller:** Refactor `valve_controller.py` to handle the full cycle (Heating -> Flashing -> Closing -> Charring). Implement a state machine rather than a simple latch.
2. **Robustify Dryness Monitor:** Add low-pass filtering to `dryness_monitor.py` to handle real-world sensor noise and ensure it handles high-frequency data sampling gracefully.
3. **Implement Heater Controller:** As per `TODO.md`, create the logic to control the solid-state relay (SSR) based on the dryness state (e.g., switch from boiling/flashing power levels to charring power levels).
4. **Mechanical Prototyping:** Build a physical prototype of the "Inside-Fitting" oval manway to test the seal under low pressure, before introducing the complexities of the internal induction coil.
5. **Implement Emergency Pop-off Safety Logic:** While the pop-off valve is mechanical, the software should also have a hard-stop safety threshold that cuts power to all heaters if pressure approaches the pop-off limit.