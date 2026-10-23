#!/usr/bin/env python3
import sys
import serial
import serial.tools.list_ports
import time
import math
import csv
import re
from datetime import datetime

# We expect serial lines like:
# P=12.34psi | Valve=CLOSED | Pump=OFF | MainHeater=ON | CatHeater=ON | TRIP=NO | DRY=NO | CycleActive=YES | Duty=100%
# T_ema=123.4°C

def find_serial_ports():
    ports = serial.tools.list_ports.comports()
    return [port.device for port in ports]

def select_serial_port():
    ports = find_serial_ports()
    if not ports:
        print("No serial ports found. Please connect the device and try again.")
        sys.exit(1)

    print("\nAvailable serial ports:")
    for i, port in enumerate(ports):
        print(f"[{i}] {port}")

    while True:
        try:
            choice = input("\nSelect a port by number: ")
            index = int(choice)
            if 0 <= index < len(ports):
                return ports[index]
            else:
                print("Invalid choice, please select a valid number.")
        except ValueError:
            print("Please enter a valid number.")

def estimate_cycle_time(mass_g):
    # Base configuration: 88g = 2 hours (120 minutes)
    # The user noted it should be exponential (larger mass = disproportionately longer dwell)
    # A simple exponential model: Time = k * mass^n
    # Let's say n = 1.5, we can solve for k.
    # 120 = k * (88)^1.5  -> k = 120 / (88)^1.5 = 120 / 825.8 = 0.1453
    # Time(mins) = 0.1453 * (mass_g)^1.5

    # Just to be safe, if mass is tiny, return something reasonable
    if mass_g <= 0:
        return 0

    k = 120.0 / math.pow(88.0, 1.5)
    estimated_minutes = k * math.pow(mass_g, 1.5)
    return estimated_minutes

def parse_line_for_pressure(line):
    # Extract "P=12.34psi"
    match = re.search(r'P=([\d\.]+)psi', line)
    if match:
        return float(match.group(1))
    return None

def parse_line_for_temp(line):
    # Extract "T_ema=123.4°C"
    match = re.search(r'T_ema=([\d\.]+)', line)
    if match:
        return float(match.group(1))
    return None

def main():
    print("=== Pyrolysis Cycle Estimator and Data Logger ===")

    # Ask for mass
    try:
        mass_input = input("Enter the input sample mass of water in grams (e.g. 88): ")
        mass_g = float(mass_input)
    except ValueError:
        print("Invalid mass entered. Exiting.")
        sys.exit(1)

    # Calculate estimated cycle time
    estimated_mins = estimate_cycle_time(mass_g)
    print(f"\nEstimated Dwell Time for {mass_g}g: {estimated_mins:.2f} minutes ({estimated_mins/60.0:.2f} hours)\n")

    # Select port
    port_name = select_serial_port()
    baud_rate = 115200 # Standard ESP-IDF baud

    print(f"\nConnecting to {port_name} at {baud_rate} baud...")

    # Prepare CSV logger
    timestamp_str = datetime.now().strftime("%Y%m%d_%H%M%S")
    csv_filename = f"pyrolysis_log_{timestamp_str}.csv"

    print(f"Logging data to: {csv_filename}")
    print("Press Ctrl+C to stop logging.\n")

    try:
        ser = serial.Serial(port_name, baud_rate, timeout=1.0)
    except Exception as e:
        print(f"Failed to open serial port {port_name}: {e}")
        sys.exit(1)

    # Open CSV file
    with open(csv_filename, mode='w', newline='') as csv_file:
        writer = csv.writer(csv_file)
        writer.writerow(["Timestamp", "Elapsed_Time_Mins", "Mass_g", "Estimated_Total_Time_Mins", "Pressure_PSI", "Temp_C", "Pyrolysis_State_Estimate_%"])

        start_time = time.time()
        last_pressure = None

        try:
            while True:
                line = ser.readline().decode('utf-8', errors='replace').strip()
                if not line:
                    continue

                print(f"[Serial] {line}")

                # Try to parse Pressure and Temperature. They might be on different lines.
                p = parse_line_for_pressure(line)
                if p is not None:
                    last_pressure = p

                t = parse_line_for_temp(line)

                if t is not None and last_pressure is not None:
                    # We have both a recent pressure and a new temp reading. Log them.
                    current_time = time.time()
                    elapsed_mins = (current_time - start_time) / 60.0

                    # Rough estimate of progress
                    progress_pct = (elapsed_mins / estimated_mins) * 100.0 if estimated_mins > 0 else 0
                    if progress_pct > 100.0:
                        progress_pct = 100.0

                    writer.writerow([
                        datetime.now().isoformat(),
                        f"{elapsed_mins:.2f}",
                        f"{mass_g:.2f}",
                        f"{estimated_mins:.2f}",
                        f"{last_pressure:.2f}",
                        f"{t:.2f}",
                        f"{progress_pct:.1f}"
                    ])
                    csv_file.flush() # Ensure it writes to disk

                    # Reset so we wait for the next full pair
                    last_pressure = None

        except KeyboardInterrupt:
            print("\nLogging stopped by user.")
        finally:
            ser.close()
            print(f"Data saved to {csv_filename}")

if __name__ == "__main__":
    main()
