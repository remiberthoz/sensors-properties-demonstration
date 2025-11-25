"""
Live absorbance sensor visualization
====================================

This Python program reads serial data from an Arduino running the *Simulated
absorbance sensors* firmware and plots each sensor's output in real time.

The Arduino sends, at regular intervals, a sequence of bytes representing
absorbance values from simulated sensors (and optionally a "ground truth"
calibrated sensor).

Each sequence is received, decoded, and displayed as a live-updating plot
using Matplotlib.

See `main.ino` or `README.md` for details on the data format, calibration,
interest, and more details.

Required Python packages:
    - matplotlib
    - numpy
    - pyserial
"""

import sys
import serial
import time

import numpy as np
import matplotlib.pyplot as plt
import matplotlib.widgets as widgets
import matplotlib.animation as anim
import matplotlib
import cycler

matplotlib.rcParams['axes.prop_cycle'] = cycler.cycler('color', ['#377eb8', '#ff7f00', '#4daf4a', '#f781bf', '#5e4c5a'])


""" ---------------------------------------------------------------------------
    Serial communication configuration.
    ---------------------------------------------------------------------------
""" 

""" Serial port path (depends on operating system).
Typical values:
    - Windows: "COM4"
    - Linux: "/dev/tty0"
"""
SERIAL_PORT = "/dev/ttyACM0"

""" Baud rate. Must match the value defined in the Arduino program."""
BAUD_RATE = 9600

""" Serial communication channel handle."""
ser = serial.Serial(SERIAL_PORT, BAUD_RATE)


""" ---------------------------------------------------------------------------
    Serial communication functions.
    ---------------------------------------------------------------------------
""" 

def grab_data():
    """ Reads one full line of sensor data from the Arduino via serial.

    Expected data format (per sampling interval):
        [sensor_1] [sensor_2] ... [sensor_N] [n_sensors] '\n'

    - Each sensor value is a byte (0-255) representing absorbance.
      0 corresponds to no absorbance, 255 to MAX_A (see Arduino code).
    - The last byte ('n_sensors') indicates how many sensor values were sent.
    - The first byte may or may not be the calibrated value, depending on how
      the Arduino program is configured.

    The function reads until a newline ('\n') is found. It checks data
    consistency by verifying that the number of received sensor bytes matches
    n_sensors. If not, it retries until a valid line is found.

    Returns:
        list[int]: Sensor values (0-255), excluding the trailing count byte.

    N_sensors indicates how many sensor values were sent.

    TODO: It may happen that the python program lags behind the Arduino,
    since the serial communication buffer is not flushed.
    """

    consecutive_mismatches = 0

    while True:

        line = ser.readline()[:-1]  # Read until '\n', remove '\n'

        if len(line) < 1:
            # Only '\n' received
            consecutive_mismatches += 1
            continue

        n_sensors = line[-1]
        if n_sensors < 1:
            # Invalid number of sensors (should not happen)
            consecutive_mismatches += 1
            continue

        if len(line) != n_sensors + 1:
            # Mismatch: missed or extra bytes, likely due to timing
            consecutive_mismatches += 1
            continue

        # All good, return sensor values
        return line[:-1]


""" ---------------------------------------------------------------------------
    Initialization.
    ---------------------------------------------------------------------------
"""

""" Number of sensors. Determined from a valid transmission. """
N_SENSORS = len(grab_data())

""" Data storage array. Acts as a rolling buffer (FIFO).
Each row represents one sensor ; 512 samples are stored per sensor.
"""
values = np.zeros(shape=(512, N_SENSORS), dtype=float)

""" Elapsed time index. """
t = 0

""" Sampling interval (ms). Must match the value in Arduino program. """
DT = 25

""" Time axis (x-axis of the graphs), values in milliseconds. """
T_AXIS = DT * np.arange(values.shape[0])


""" ---------------------------------------------------------------------------
    Matplotlib figure setup.
    ---------------------------------------------------------------------------

    Includes:
    - A main plot showing the output of all sensors over time.
    - A "Pause" button to temporarily stop live updates.
"""

""" Pause state flag. """
PAUSED = False

def toggle_pause(_):
    """Toggle the paused state.
    Triggered when the 'Pause' button is clicked).
    """
    global PAUSED
    PAUSED = not PAUSED

# Create main figure and axis
fig, ax = plt.subplots(1, figsize=(16, 8))

# Add small axis inset for the Pause button
ax_pause = fig.add_axes([0.80, 0.01, 0.05, 0.05])
btn_pause = widgets.Button(ax_pause, "Pause")
btn_pause.on_clicked(toggle_pause)

# Initialize one line per sensor
lines = ax.plot(T_AXIS/1000, values,
    ls='none', marker='.',
    label=[f"Sensor {i}" for i in range(1, N_SENSORS+1)]
    )

# Configure plot appearance
ax.set_ylim(0, 5)
ax.grid()
ax.legend()
ax.set_ylabel("Valeur de sortie du capteur (V)")
ax.set_xlabel("Temps (s)")


""" ---------------------------------------------------------------------------
    Animation function (main loop)
    ---------------------------------------------------------------------------
"""

def animate(i):
    """ Update the live plot at each animation frame.

    - Reads new data from the serial port.
    - Scales byte values (0-255) to volts (0-5V).
    - Updates the rolling data buffer and redraws the graph.

    Parameters:
        i (int): Animation frame index (provided by matplotlib).

    Returns:
        list[Line2D]: updated plot lines (required by matplotlib).
    """

    # If buffer is full, roll it left (discard oldest values)
    if i >= values.shape[0]:
        i = values.shape[0] - 1
        values[:] = np.roll(values, -1, axis=0)

    # Read the newest data from serial and scale to volts
    values[i] = [c/256*5 for c in grab_data()]
    
    # Skip updates if paused
    if PAUSED:
        return lines

    # Update each sensor's graph
    for l, line in enumerate(lines):
        line.set_ydata(values[:, l])

    return lines

# Start animation
ani = anim.FuncAnimation(fig, animate, None, interval=0, blit=True)
plt.show()