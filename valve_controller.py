class ValveController:
    """
    Controls the steam valve for the Biochar Toilet reaction vessel.

    The valve is initially closed. When the pressure reaches the target (e.g., 15 PSI)
    and the temperature reaches the target (e.g., 121 C), the valve opens to "flash"
    the steam, sterilizing and destroying the sample.
    """
    def __init__(self, target_pressure=15.0, target_temperature=121.0):
        """
        Initializes the ValveController.

        Args:
            target_pressure (float): The pressure threshold to trigger opening (default 15.0 PSI).
            target_temperature (float): The temperature threshold to trigger opening (default 121.0 C).
        """
        self.target_pressure = target_pressure
        self.target_temperature = target_temperature
        self.valve_open = False

    def update(self, pressure, temperature):
        """
        Updates the valve state based on current sensor readings.

        Args:
            pressure (float): Current pressure reading.
            temperature (float): Current temperature reading.
        """
        if not self.valve_open:
            if pressure >= self.target_pressure and temperature >= self.target_temperature:
                self.valve_open = True

    def is_open(self):
        """
        Returns the current state of the valve.

        Returns:
            bool: True if the valve is open, False otherwise.
        """
        return self.valve_open

    def reset(self):
        """
        Resets the valve to the closed state.
        """
        self.valve_open = False
