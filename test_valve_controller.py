import unittest
from valve_controller import ValveController

class TestValveController(unittest.TestCase):
    def test_initial_state(self):
        controller = ValveController()
        self.assertFalse(controller.is_open())
        self.assertFalse(controller.is_emergency_tripped())
        self.assertEqual(controller.dynamic_target_pressure, 8.0)
        self.assertFalse(controller.dry_latched)

    def test_venting_lower_bound(self):
        # Initial bounds: target 8.0, lower bound 6.0
        controller = ValveController(target_pressure=8.0, target_temperature=121.0, min_seal_pressure=4.0)

        # Trigger release cycle first
        controller.update(pressure=8.0, temperature=121.0, current_time_ms=0)
        self.assertTrue(controller.release_active)

        # Pressure drops to 6.0, which is the vent_lower_bound (target - 2.0)
        # It should end the release state here, not go all the way down to 4.0
        controller.update(pressure=6.0, temperature=121.0, current_time_ms=500)
        self.assertFalse(controller.is_open())
        self.assertFalse(controller.release_active)

    def test_dynamic_target_drop_and_dry_latch(self):
        controller = ValveController(target_pressure=7.0, target_temperature=121.0, min_seal_pressure=4.0, repressurize_timeout_ms=5000)

        # Temp is below 100, timer should not start
        controller.update(pressure=5.0, temperature=99.0, current_time_ms=0)
        self.assertFalse(controller.is_repressurizing)

        # Temp hits 100, timer starts
        controller.update(pressure=5.0, temperature=100.0, current_time_ms=1000)
        self.assertTrue(controller.is_repressurizing)
        self.assertEqual(controller.repressurize_timer_start, 1000)
        self.assertEqual(controller.dynamic_target_pressure, 7.0)

        # If pressure is higher than target, timer stops
        controller.update(pressure=7.5, temperature=100.0, current_time_ms=2000)
        self.assertFalse(controller.is_repressurizing)

        # Pressure drops below target, timer restarts
        controller.update(pressure=5.0, temperature=100.0, current_time_ms=3000)
        self.assertTrue(controller.is_repressurizing)
        self.assertEqual(controller.repressurize_timer_start, 3000)

        # 5 seconds later (timeout hit), target should drop from 7 to 6
        controller.update(pressure=5.0, temperature=100.0, current_time_ms=8000)
        self.assertEqual(controller.dynamic_target_pressure, 6.0)
        self.assertEqual(controller.vent_lower_bound, 4.0)

        # Another 5 seconds later, target drops from 6 to 5
        controller.update(pressure=4.0, temperature=100.0, current_time_ms=13000)
        self.assertEqual(controller.dynamic_target_pressure, 5.0)
        self.assertEqual(controller.vent_lower_bound, 4.0)

        # Another 5 seconds later, target drops from 5 to 4
        controller.update(pressure=3.0, temperature=100.0, current_time_ms=18000)
        self.assertEqual(controller.dynamic_target_pressure, 4.0)
        self.assertEqual(controller.vent_lower_bound, 4.0)

        self.assertFalse(controller.dry_latched)

        # Now at minimum. Another 5 seconds means dry latched
        controller.update(pressure=3.0, temperature=100.0, current_time_ms=23000)
        self.assertTrue(controller.dry_latched)

    def test_single_shot_duration_and_cooldown(self):
        # Window is 2000ms, cooldown is 2000ms
        controller = ValveController(target_pressure=8.0, target_temperature=121.0, min_seal_pressure=4.0, window_ms=2000, cooldown_ms=2000)

        # 1. Trigger release cycle at pressure = 8.0 (High pressure).
        # range = 8.0 - 6.0 = 2.0
        # t = (8.0 - 6.0) / 2.0 = 1.0 -> duration = 2000ms
        controller.update(pressure=8.0, temperature=121.0, current_time_ms=0)
        self.assertTrue(controller.is_open())
        self.assertTrue(controller.release_active)

        # 2. Still open at 1000ms
        controller.update(pressure=8.0, temperature=121.0, current_time_ms=1000)
        self.assertTrue(controller.is_open())

        # 3. Burst finishes at 2000ms, valve should close and enter cooldown
        controller.update(pressure=8.0, temperature=121.0, current_time_ms=2000)
        self.assertFalse(controller.is_open())
        self.assertEqual(controller.cooldown_start_time, 2000)

    def test_emergency_overpressure_trip(self):
        controller = ValveController(target_pressure=8.0, min_seal_pressure=4.0, max_safe_pressure=18.0, max_safe_temperature=250.0)

        # Pressure exceeds emergency safe limit (18.0 PSI)
        controller.update(pressure=18.5, temperature=80.0, current_time_ms=0)
        self.assertTrue(controller.is_open())
        self.assertTrue(controller.is_emergency_tripped())

    def test_emergency_overtemp_trip(self):
        controller = ValveController(target_pressure=8.0, min_seal_pressure=4.0, max_safe_pressure=18.0, max_safe_temperature=250.0)

        # Temperature exceeds emergency safe limit (250.0 °C)
        controller.update(pressure=10.0, temperature=255.0, current_time_ms=0)
        self.assertTrue(controller.is_open())
        self.assertTrue(controller.is_emergency_tripped())

    def test_emergency_latching_behavior(self):
        controller = ValveController(target_pressure=8.0, target_temperature=121.0, min_seal_pressure=4.0, max_safe_pressure=18.0, max_safe_temperature=250.0)

        # Trip emergency
        controller.update(pressure=19.0, temperature=121.0, current_time_ms=0)
        self.assertTrue(controller.is_open())

        # Pressure drops after emergency trip, valve should stay latched open
        controller.update(pressure=3.0, temperature=100.0, current_time_ms=100)
        self.assertTrue(controller.is_open())

if __name__ == '__main__':
    unittest.main()
