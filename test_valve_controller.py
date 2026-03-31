import unittest
from valve_controller import ValveController

class TestValveController(unittest.TestCase):
    def test_initial_state(self):
        controller = ValveController()
        self.assertFalse(controller.is_open())

    def test_heating_phase(self):
        controller = ValveController(target_pressure=15.0, target_temperature=121.0)
        # Pressure and Temp below target
        controller.update(pressure=10.0, temperature=100.0)
        self.assertFalse(controller.is_open())

        # Pressure high, Temp low
        controller.update(pressure=16.0, temperature=100.0)
        self.assertFalse(controller.is_open())

        # Pressure low, Temp high
        controller.update(pressure=10.0, temperature=125.0)
        self.assertFalse(controller.is_open())

    def test_threshold_crossing(self):
        controller = ValveController(target_pressure=15.0, target_temperature=121.0)

        # Start below threshold
        controller.update(pressure=14.0, temperature=120.0)
        self.assertFalse(controller.is_open())

        # Transition to exactly threshold
        controller.update(pressure=15.0, temperature=121.0)
        self.assertTrue(controller.is_open())

        # Reset and test crossing from below to strictly above
        controller.reset()
        controller.update(pressure=14.9, temperature=120.9)
        self.assertFalse(controller.is_open())

        controller.update(pressure=15.5, temperature=122.0)
        self.assertTrue(controller.is_open())

    def test_latching_behavior(self):
        controller = ValveController(target_pressure=15.0, target_temperature=121.0)
        # Trigger opening
        controller.update(pressure=15.0, temperature=121.0)
        self.assertTrue(controller.is_open())

        # Pressure drops (e.g., after valve opens)
        controller.update(pressure=0.0, temperature=100.0)
        self.assertTrue(controller.is_open())

    def test_reset(self):
        controller = ValveController()
        controller.update(pressure=15.0, temperature=121.0)
        self.assertTrue(controller.is_open())
        controller.reset()
        self.assertFalse(controller.is_open())

if __name__ == '__main__':
    unittest.main()
