# Simulated absorbance sensors

This program simulates a series of light intensity sensors based on a single
physical sensor. It is designed primarily for educational purposes to teach
biology or chemistry students about sensor properties. Simulated sensors have
different properties (linearity, sensitivity, range, response time, noise).
The program visualizes the behavior of these simulated sensors in real-time,
allowing for a hands-on, inexpensive way to discover these concepts.


## Problems to fix / todo list...

- The calibration procedure is not OK, it should somehow take the value of
  MAX_A into account ; and likely the complete scaling (with to_byte).

- Lower the Baud rate to make the transmission more robust with poor-quality
  USB cables.

- Try to output sensor values on DAC such that the students can work on an
  oscilloscope. Check if the DAC communication rate is good enough to
  simulate real analog.

- Program Arduino IO to read jumper status, and potentially allow students to
  change easily configuration.

## TECHNICAL REQUIREMENTS

### Hardware

The program is designed to run on an Arduino Uno board (or compatible) fitted
whith a companion shield that holds a light source, a light sensor, and a
sample holder for spectrophotometry cuvettes.

The light source and the sensor are positioned approximately 1 cm apart,
separated by the cuvette holder, which is used to adjust light intensities on
the sensor.

Refer to the schematic for connections and for part numbers that I used (also
see below). The cuvette holder was 3d printed, the 3d model is available.

<img width="1496" height="840" alt="shield_schematic" src="https://github.com/user-attachments/assets/14cae6d0-0188-45b4-8bf7-49f29e9427bc" />

<img width="1064" height="722" alt="shield_layout" src="https://github.com/user-attachments/assets/6a7c240b-edb2-42dd-8647-925a03d93173" />

<img width="514" height="412" alt="shield_cuvette_holder" src="https://github.com/user-attachments/assets/3ce30682-7737-4427-b172-6110486a68bb" />

#### Details on parts and wavelength choices

I decided to work with infrared light (935 +/- 25) nm to limit interference
from modern indoor lighting sources. I used a IR LED, OP140D with a 470R
resitor ; and a wavelength-matched OP550A IR NPN phototransitor kept in
linear regime using a 1k resistor on the emitter side.

Samples absorbing in infrared can easily be prepared with aqueous solutions
of copper sulfate. To avoid spills in the electronic lab, I mixed solutions
of copper sulfate with a preparation of 1 % agarose. It gelifies and stays in
the cuvettes.

Hydrated copper sulfate and agarose gel both absorb at 935 nm. The absorbance
of these prepared samples can be measured with a reference spectrophotometer.

### Software

This Arduino program works in tandem with a companion python program, which
runs on a computer to:

- Read data from the Arduino, through serial communication.
- Display a live graph of the sensors readings.


## Objective

This setup aims to teach students about sensor properties such as:

- Linearity
- Sensitivity
- Range (low and high saturations)
- Response time
- Noise

Students will visualize the response of several simulated sensors in
real-time. These sensors outputs are derived from the output of the physical
sensor, giving a sense of reality to the simulation (simulated sensors will
respond to change of absorbance standards, ambient light, light path
obstruction, ...).

This exercise allows students to explore how different sensors react to the
same physical change, producing different electrical outputs. Students can
create output-input graphs, measure response time, and compare sensors.

Instructors can adjust the sensors properties by change in code, rather than
by buying and mounting various sensors.


## Simulated sensors are *absorbance sensors*

In the context of biology and chemistry courses, students are typically
introduced to the concept of absorbance, often via spectrophotometry. Since
this concept is well known, and thus more intuitive than light intensity,
the sensors outputs a quantity linked to absorbance rather than light 
intensities (even though it really measures light intensities). This makes
it similar to how spectrophotometers usually work.


## Calibration

The setup can be calibrated by the instructors using samples of known
absorbance. Adjustements in the code can be made such that the Arduino
program can determine true absorbance internally, before deriving the output
values of simulated sensors.

Indeed, the sensor used on the Arduino shield is not necessarely linear
against light intensity itself.


## Prerequisites for students

Before using this system, students should:

- Understand the concept of absorbance as a property of a sample.
- Know that electronic sensors measure an electrical quantity (e.g., voltage,
  current), which is then mapped to physical parameters like absorbance.

These two facts explain why the output values are all different for a single
sample.

Students do not need to know that there is only one physical sensor in the
system. The setup can be presented as if multiple sensors are being used,
each with different response characteristics.


## Students focus on sensor characteristics

The main teaching goal is for students to understand how different sensors
behave, not how absorbance relates to light intensity. The system abstracts
this relationship by presenting data as voltage outputs (mapped to real
absorbance values via calibration). Students can focus on sensor
characteristics. This knowledge can later be transferred to any type of
sensor.
