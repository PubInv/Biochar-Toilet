# Project Tasks

Based on `README.md` and `DESIGN_IDEAS.md`.

- [x] Implement Valve Controller logic (to address the "small bug controlling the valve").
- [ ] Implement Heater Controller (mentioned as a future step).
- [ ] Design reaction vessel (ongoing).
- [ ] Implement emergency pop-off valve safety logic.

### Testing and Benchmarking Improvements
- [ ] Fix lazy test `test_cooling_or_steady_temp` in `test_dryness_monitor.py` which only tests slow temperature rise instead of actual cooling (temperature dropping).
- [ ] Fix lazy test `test_threshold_crossing` in `test_valve_controller.py` which only tests the boundary value exactly, instead of a true threshold crossing (transition from below to above threshold).
- [ ] Add tests simulating noisy and high-frequency sensor data for `dryness_monitor.py` (rate calculation is highly sensitive to window size).
- [ ] Develop tests and benchmarks for low-moisture samples (like the bread charring experiment) where the dryness algorithm failed to trigger due to lack of distinct pressure change.
