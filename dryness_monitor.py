import collections

class DrynessMonitor:
    def __init__(self, min_temp_rate=0.5, max_pressure_rate=0.1, history_len=10):
        """
        Initializes the DrynessMonitor.

        Args:
            min_temp_rate (float): Minimum rate of temperature rise (deg/sec) to consider as heating up.
            max_pressure_rate (float): Maximum rate of pressure rise (unit/sec) to consider as "not rising" (i.e. dry).
            history_len (int): Number of past readings to keep for calculating rates.
        """
        self.min_temp_rate = min_temp_rate
        self.max_pressure_rate = max_pressure_rate
        self.history = collections.deque(maxlen=history_len)

    def add_reading(self, temperature, pressure, timestamp):
        """
        Adds a new reading to the monitor.

        Args:
            temperature (float): Current temperature reading.
            pressure (float): Current pressure reading.
            timestamp (float): Current timestamp (in seconds).
        """
        self.history.append({
            'temp': temperature,
            'pressure': pressure,
            'time': timestamp
        })

    def is_dry(self):
        """
        Determines if the sample is dry based on the history of readings.

        Condition: Temperature is rising (> min_temp_rate) AND
                   Pressure is not rising significantly (< max_pressure_rate).

        Returns:
            bool: True if the sample is considered dry, False otherwise.
        """
        if len(self.history) < 2:
            return False

        # Calculate rates based on the first and last point in the history window
        start = self.history[0]
        end = self.history[-1]

        time_diff = end['time'] - start['time']

        if time_diff <= 0:
            return False

        temp_rate = (end['temp'] - start['temp']) / time_diff
        pressure_rate = (end['pressure'] - start['pressure']) / time_diff

        return temp_rate > self.min_temp_rate and pressure_rate < self.max_pressure_rate
