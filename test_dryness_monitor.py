import unittest
from dryness_monitor import DrynessMonitor

class TestDrynessMonitor(unittest.TestCase):
    def test_initial_state(self):
        monitor = DrynessMonitor()
        self.assertFalse(monitor.is_dry(), "Should not be dry initially without data")

    def test_wet_state(self):
        # Wet state: Both T and P rise (simulating heating water in closed vessel)
        monitor = DrynessMonitor(min_temp_rate=0.5, max_pressure_rate=0.1)

        # Add data points where both T and P increase
        # T increases 1 deg/sec, P increases 0.5 unit/sec
        # This should result in is_dry() == False because P rate (0.5) > max_pressure_rate (0.1)
        for i in range(5):
            monitor.add_reading(temperature=100 + i*1.0, pressure=10 + i*0.5, timestamp=i)

        self.assertFalse(monitor.is_dry(), "Should not be dry when pressure is rising significantly")

    def test_dry_state(self):
        # Dry state: T rises, P stays relatively constant
        monitor = DrynessMonitor(min_temp_rate=0.5, max_pressure_rate=0.1)

        # Add data points where T increases but P is stable
        # T increases 1 deg/sec, P increases 0.05 unit/sec
        # This should result in is_dry() == True because:
        # T rate (1.0) > min_temp_rate (0.5) AND P rate (0.05) < max_pressure_rate (0.1)
        for i in range(5):
            monitor.add_reading(temperature=150 + i*1.0, pressure=20 + i*0.05, timestamp=i)

        self.assertTrue(monitor.is_dry(), "Should be dry when temp rises and pressure is stable")

    def test_cooling_or_steady_temp(self):
        # Temp not rising enough
        monitor = DrynessMonitor(min_temp_rate=0.5, max_pressure_rate=0.1)

        # T increases 0.1 deg/sec (below threshold), P stable
        for i in range(5):
            monitor.add_reading(temperature=100 + i*0.1, pressure=10, timestamp=i)

        self.assertFalse(monitor.is_dry(), "Should not be dry if temp is not rising fast enough")

    def test_pressure_drop(self):
        # Pressure dropping (maybe leak or cooling), Temp rising
        monitor = DrynessMonitor(min_temp_rate=0.5, max_pressure_rate=0.1)

        # T increases 1.0 deg/sec, P decreases
        for i in range(5):
            monitor.add_reading(temperature=100 + i*1.0, pressure=10 - i*0.1, timestamp=i)

        # P rate is negative, which is < max_pressure_rate. T rate is high. Should be dry (or at least satisfy condition).
        # In physical reality, P dropping while T rising might mean valve open or leak, effectively "dry" wrt steam generation pressure build-up.
        self.assertTrue(monitor.is_dry(), "Should be dry if pressure is dropping while temp rises")

if __name__ == '__main__':
    unittest.main()
