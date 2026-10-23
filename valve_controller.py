import math
import time

class ValveController:
    """
    Controls the steam valve for the Biochar Toilet reaction vessel with safety interlocks.
    Uses calculated single-shot duration with cooldown for logarithmic pressure release.
    Also implements dynamic target pressure dropping and dryness detection based on repressurization timeouts.
    """
    def __init__(self, target_pressure=8.0, target_temperature=121.0, min_seal_pressure=4.0, max_safe_pressure=18.0, max_safe_temperature=250.0, window_ms=2000, cooldown_ms=2000, repressurize_timeout_ms=300000):
        self.initial_target_pressure = target_pressure
        self.target_temperature = target_temperature
        self.min_seal_pressure = min_seal_pressure
        self.max_safe_pressure = max_safe_pressure
        self.max_safe_temperature = max_safe_temperature

        self.valve_open = False
        self.release_active = False
        self.emergency_tripped = False

        self.dynamic_target_pressure = target_pressure
        self.dry_latched = False

        self.window_ms = window_ms
        self.cooldown_ms = cooldown_ms
        self.burst_start_time = 0
        self.burst_duration = 0
        self.cooldown_start_time = -self.cooldown_ms

        self.repressurize_timeout_ms = repressurize_timeout_ms
        self.repressurize_timer_start = 0
        self.is_repressurizing = False

        # Calculate bounds
        self._update_bounds()

    def _update_bounds(self):
        self.vent_lower_bound = max(self.min_seal_pressure, self.dynamic_target_pressure - 2.0)

    def get_time_ms(self):
        return time.time() * 1000

    def update(self, pressure, temperature, current_time_ms=None):
        if current_time_ms is None:
            current_time_ms = self.get_time_ms()

        # Emergency Interlock Check
        if pressure >= self.max_safe_pressure or temperature >= self.max_safe_temperature:
            self.emergency_tripped = True
            self.valve_open = True
            return

        if self.emergency_tripped:
            self.valve_open = True
            self.release_active = False
            return

        # ------------------- DRYNESS & DYNAMIC TARGET LOGIC -------------------
        if not self.valve_open and not self.release_active and not self.dry_latched:
            # We are waiting for pressure to build.
            if temperature >= 100.0 and pressure < self.dynamic_target_pressure:
                if not self.is_repressurizing:
                    self.is_repressurizing = True
                    self.repressurize_timer_start = current_time_ms
                else:
                    # Check for timeout
                    if (current_time_ms - self.repressurize_timer_start) >= self.repressurize_timeout_ms:
                        if self.dynamic_target_pressure > self.min_seal_pressure:
                            self.dynamic_target_pressure = max(self.min_seal_pressure, self.dynamic_target_pressure - 1.0)
                            self._update_bounds()
                            print(f"[Valve] Timeout reached. Dropped target pressure to {self.dynamic_target_pressure} PSI")
                            self.repressurize_timer_start = current_time_ms # Reset timer for new target
                        else:
                            # Reached minimum seal pressure and STILL timed out. We are dry.
                            self.dry_latched = True
                            self.is_repressurizing = False
                            print(f"[Valve] DRY LATCHED: Failed to reach minimum seal pressure ({self.min_seal_pressure} PSI)")
            else:
                self.is_repressurizing = False
        else:
            # If valve is open or releasing, we are not repressurizing in the 'stuck' sense
            self.is_repressurizing = False

        # ------------------- VALVE LOGIC -------------------
        if not self.release_active and pressure >= self.dynamic_target_pressure and temperature >= self.target_temperature:
            self.release_active = True
            print(f"[Valve] Target {self.dynamic_target_pressure} PSI reached. Initiating release.")

        if self.release_active:
            # CRITICAL SAFETY FLOOR
            if pressure <= self.vent_lower_bound:
                self.valve_open = False
                self.release_active = False
                print(f"[Valve] Dropped below vent bound {self.vent_lower_bound} PSI. Closing.")
            else:
                if self.valve_open:
                    # We are in an active burst
                    if (current_time_ms - self.burst_start_time) >= self.burst_duration:
                        # Burst finished, close valve and start cooldown
                        self.valve_open = False
                        self.cooldown_start_time = current_time_ms
                else:
                    # Valve is closed. Are we in cooldown?
                    if (current_time_ms - self.cooldown_start_time) >= self.cooldown_ms:
                        # Cooldown finished (or hasn't happened yet), start a new burst
                        clamped_pressure = min(pressure, self.dynamic_target_pressure)
                        range_p = self.dynamic_target_pressure - self.vent_lower_bound

                        if range_p <= 0:
                            duty_cycle = 0.0
                        else:
                            t = (clamped_pressure - self.vent_lower_bound) / range_p
                            log_factor = math.log1p(t * 1.71828) / 1.0
                            duty_cycle = max(0.0, min(1.0, log_factor))

                        self.burst_duration = duty_cycle * self.window_ms
                        self.burst_start_time = current_time_ms
                        self.valve_open = True
        else:
            self.valve_open = False

    def is_open(self):
        return self.valve_open

    def is_emergency_tripped(self):
        return self.emergency_tripped

    def reset(self, current_time_ms=None):
        self.valve_open = False
        self.release_active = False
        self.emergency_tripped = False
        self.burst_start_time = 0
        self.cooldown_start_time = -self.cooldown_ms
        self.dynamic_target_pressure = self.initial_target_pressure
        self.dry_latched = False
        self.is_repressurizing = False
        self._update_bounds()
