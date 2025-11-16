#include <Arduino.h>
#include <math.h>

/** Simulated absorbance sensors
 *  ============================
 *
 * This program simulates a series of light intensity sensors based on a single
 * physical sensor. It is designed primarily for educational purposes to teach
 * biology or chemistry students about sensor properties. Simulated sensors have
 * different properties (linearity, sensitivity, range, response time, noise).
 * The program visualizes the behavior of these simulated sensors in real-time,
 * allowing for a hands-on, inexpensive way to discover these concepts.
 *
 * 
 * ## TECHNICAL REQUIREMENTS
 *
 * ### Hardware
 *
 * The program is designed to run on an Arduino Uno board (or compatible) fitted
 * whith a companion shield that holds a light source, a light sensor, and a
 * sample holder for spectrophotometry cuvettes.
 *
 * The light source and the sensor are positioned approximately 1 cm apart,
 * separated by the cuvette holder, which is used to adjust light intensities on
 * the sensor.
 *
 * Refer to the schematic for connections and for part numbers that I used (also
 * see below). The cuvette holder was 3d printed, the 3d model is available.
 *
 * #### Details on parts and wavelength choices
 *
 * I decided to work with infrared light (935 +/- 25) nm to limit interference
 * from modern indoor lighting sources. I used a IR LED, OP140D with a 470R
 * resitor ; and a wavelength-matched OP550A IR NPN phototransitor kept in
 * linear regime using a 1k resistor on the emitter side.
 *
 * Samples absorbing in infrared can easily be prepared with aqueous solutions
 * of copper sulfate. To avoid spills in the electronic lab, I mixed solutions
 * of copper sulfate with a preparation of 1 % agarose. It gelifies and stays in
 * the cuvettes.
 *
 * Hydrated copper sulfate and agarose gel both absorb at 935 nm. The absorbance
 * of these prepared samples can be measured with a reference spectrophotometer.
 *
 * 
 * ### Software
 *
 * This Arduino program works in tandem with a companion python program, which
 * runs on a computer to:
 * 
 * - Read data from the Arduino, through serial communication.
 * - Display a live graph of the sensors readings.
 *
 *
 * ## Objective
 *
 * This setup aims to teach students about sensor properties such as:
 * 
 * - Linearity
 * - Sensitivity
 * - Range (low and high saturations)
 * - Response time
 * - Noise
 *
 * Students will visualize the response of several simulated sensors in
 * real-time. These sensors outputs are derived from the output of the physical
 * sensor, giving a sense of reality to the simulation (simulated sensors will
 * respond to change of absorbance standards, ambient light, light path
 * obstruction, ...).
 * 
 * This exercise allows students to explore how different sensors react to the
 * same physical change, producing different electrical outputs. Students can
 * create output-input graphs, measure response time, and compare sensors.
 * 
 * Instructors can adjust the sensors properties by change in code, rather than
 * by buying and mounting various sensors.
 *
 * 
 * ## Simulated sensors are *absorbance sensors*
 *
 * In the context of biology and chemistry courses, students are typically
 * introduced to the concept of absorbance, often via spectrophotometry. Since
 * this concept is well known, and thus more intuitive than light intensity,
 * the sensors outputs a quantity linked to absorbance rather than light 
 * intensities (even though it really measures light intensities). This makes
 * it similar to how spectrophotometers usually work.
 * 
 *
 * ## Calibration
 * 
 * The setup can be calibrated by the instructors using samples of known
 * absorbance. Adjustements in the code can be made such that the Arduino
 * program can determine true absorbance internally, before deriving the output
 * values of simulated sensors.
 * 
 * Indeed, the sensor used on the Arduino shield is not necessarely linear
 * against light intensity itself.
 * 
 * 
 * ## Prerequisites for students
 * 
 * Before using this system, students should:
 * 
 * - Understand the concept of absorbance as a property of a sample.
 * - Know that electronic sensors measure an electrical quantity (e.g., voltage,
 *   current), which is then mapped to physical parameters like absorbance.
 * 
 * These two facts explain why the output values are all different for a single
 * sample.
 * 
 * Students do not need to know that there is only one physical sensor in the
 * system. The setup can be presented as if multiple sensors are being used,
 * each with different response characteristics.
 *
 * 
 * ## Students focus on sensor characteristics
 *
 * The main teaching goal is for students to understand how different sensors
 * behave, not how absorbance relates to light intensity. The system abstracts
 * this relationship by presenting data as voltage outputs (mapped to real
 * absorbance values via calibration). Students can focus on sensor
 * characteristics. This knowledge can later be transferred to any type of
 * sensor.
 */


/** ---------------------------------------------------------------------------
 *  Pinout
 *  ---------------------------------------------------------------------------
 */

/** Arduino analog pin connected to the physical sensor.
 * Reads a voltage linked to the light intensity incident on the sensor.
 */
constexpr uint8_t PHYSICAL_SENSOR_PIN = A0;

/** Arduino pin connected to light source. */
constexpr uint8_t PHYSICAL_OUTPUT_PIN = 5;


/** ---------------------------------------------------------------------------
 *  Physical sensor reading and calibration
 *  ---------------------------------------------------------------------------
 */

/** Reads the voltage value from the physical sensor.
 * Returns a floating-point value corresponding to the raw ADC reading
 * (integer in the range 0-1023, cast to float for convenience).
 */
float read_physical_sensor() {
    return (float) analogRead(PHYSICAL_SENSOR_PIN);
}

/** Sensor reading in "dark" condition (light source is OFF). */
float physical_dark;

/** Sensor reading in full illumination (light source is ON, blank sample). */
float physical_full;

/** Calibration factors for physical sensor.
 * 
 * The physical sensor may not be perfectly linear: the measured absorbance
 * (A_measured) is not necessarily proportional to the true absorbance
 * (A_standard).
 * 
 * Calibration procedure:
 * 
 * 1. Set POWER_LAW_CORRECTOR_GAMMA = 1 and POWER_LAW_CORRECTOR_C = 1.
 * 2. Set OUTPUT_CALIBRATED_A = true.
 * 3. Record output values from the physical sensor using samples of known
 *    absorbance.
 * 4. Fit a model:  output = c * (A_standard)^g
 * 5. Set:
 *     POWER_LAW_CORRECTOR_GAMMA = 1/g
 *     POWER_LAW_CORRECTOR_C     = 1/c
 * 6. New output values should match real absorbance, when taking scaling into
 *    account (see below).
 */
constexpr float POWER_LAW_CORRECTOR_GAMMA = 0.666;
constexpr float POWER_LAW_CORRECTOR_C = 1;

/** Converts a raw physical sensor reading into calibrated absorbance. */
float physical_to_A(const float physical_value) {
    const float A = pow(
        max(0, log10f(physical_full / physical_value) / POWER_LAW_CORRECTOR_C),
        POWER_LAW_CORRECTOR_GAMMA
    );
    return A;
}


/** ---------------------------------------------------------------------------
 *  Absorbance scaling
 *  ---------------------------------------------------------------------------
 */

/** Maximum expected absorbance value.
 * Used only for scaling sensor outputs to byte values (0-255).
 */
constexpr float MAX_A = 2;

/** Conversion factor from absorbance to byte-scale (0-255). */
constexpr float to_byte = UINT8_MAX / MAX_A;


/** ---------------------------------------------------------------------------
 *  Simulation timing
 *  ---------------------------------------------------------------------------
 */

/** Sampling interval (in milliseconds).
 * 
 * Defines both the data readings and serial transmission rate.
 * This value must match the time axis configuration in the companion python
 * program.
 */
constexpr unsigned long simulation_dt_ms = 25;

/** Maximum number of memory slots (per sensor) for response-time simulation.
 * Used to statically allocate storage for delayed sensor response.
 */
constexpr size_t MAX_MEMORY_SIZE = 50;


/** ---------------------------------------------------------------------------
 *  Utility functions
 *  ---------------------------------------------------------------------------
 */

/** Generates uniform random noise in the range [-sigma, +sigma]. */
float uniform(float sigma) {
    return (((float) rand() / RAND_MAX) - 0.5) * sigma;
}

/** See accumulate_read_physical_sensor(). */
float accumulated_reads_value = 0;
/** See accumulate_read_physical_sensor(). */
int accumulated_reads_count = 0;

/** Reads a sample from the physical sensor, increments the reading to
 * accumulated_reads_value and increments accumulated_reads_count by one.
 *
 * Can be used to read multiplie values and perform averaging. Is intended to be
 * used with get_and_reset_accumulated_reads().
 */
void accumulate_read_physical_sensor() {
    accumulated_reads_value += read_physical_sensor();
    accumulated_reads_count += 1;
}

/** Returns the average of reads accumulated by accumulate_read_physical_sensor(),
 * and resets the accumulation variables.
 */
float get_and_reset_accumulated_reads() {
    const float result = accumulated_reads_value / accumulated_reads_count;
    accumulated_reads_value = 0;
    accumulated_reads_count = 0;
    return result;
}


/** ---------------------------------------------------------------------------
 *  Simulated sensors
 *  ---------------------------------------------------------------------------
 *
 * Represents a simulated absorbance sensor derived from the calibrated physical
 * sensor. Each simulated sensor can have:
 *
 * - A background offset.
 * - A linear sensitivity.
 * - A non-linear power response.
 * - Additive random noise.
 * - A finite response time (via memory-based smoothing).
 *
 * The instantaneous sensor output value is modeled as:
 *
 *     value = max(background, |uniform(sigma)| + pow(real_A, gamma) * alpha)
 *
 * where `real_A` is the calibrated absorbance measured by the physical sensor.
 *
 * The instantaneous output value is then averaged with past values (based on
 * response time) to simulate slower sensor dynamics.
 */
class SimulatedSensor {
    public:
    
    const size_t memory_size;  // Number of memory slots (more = longer response time)
    const float background;    // Minimum output (baseline)
    const float alpha;         // Sensitivity (scaling factor)
    const float gamma;         // Non-linearity exponent
    const float sigma;         // Noise amplitude

    uint8_t memory[MAX_MEMORY_SIZE] = { 0 };  // FIFO buffer for temporal smoothing

    SimulatedSensor(const float response_time_ms,
                    const float background,
                    const float alpha,
                    const float gamma,
                    const float sigma):
        memory_size{min((size_t) round(response_time_ms / simulation_dt_ms), MAX_MEMORY_SIZE)},
        background{background},
        alpha{alpha},
        gamma{gamma},
        sigma{sigma}
        {};

    /** Updates the simulated sensor output for a given real absorbance value. */
    uint8_t update(const float real_A) {

        // Compute base value for this sensor
        float simulated_output = pow(real_A, gamma) * alpha;

        // Apply response-time smoothing
        if (memory_size >= 1) {
            float acc = 0;
            for (size_t i = 0; i < memory_size; i++) {
                acc += (float) memory[i]/to_byte;
            }
            simulated_output = (acc + simulated_output) / (memory_size + 1);
        }

        // Apply background floor
        if (simulated_output < background) {
            simulated_output = background;
        }

        // Add noise
        simulated_output += abs(uniform(sigma));

        // Clamp and convert to byte range (for storage and transmission to computer)
        uint8_t simulated_byte_output = round(max(0, min(simulated_output*to_byte, UINT8_MAX)));

        // Shift memory buffer and store new output
        if (memory_size >= 1) {
            for (size_t i = 0; i < memory_size-1; i++) {
                memory[i] = memory[i+1];
            }
            memory[memory_size-1] = simulated_byte_output;
        }

        return simulated_byte_output;
    }
};


/** ---------------------------------------------------------------------------
 *  Sensors configuration
 *  ---------------------------------------------------------------------------
 * 
 * Each simulated sensor below is derived from the physical measurement, with
 * different response characteristics.
 */
SimulatedSensor sensors[] = {
    // Calibrated sensor, typically hidden from students
    // SimulatedSensor{0.0, 0.0, 1.00, 1.00, 0.00},

    // Sensor more sensitive at low absorbance
    SimulatedSensor{3.0, 0.04, 3.00, 1.00, 0.20},

    // Sensor with slower temporal response
    SimulatedSensor{400.0, 0.02, 0.75, 1.00, 0.03},

    // Sensor with reduced overall sensitivity
    SimulatedSensor{0.0, 0.03, 0.50, 1.00, 0.03},

    // Sensor more sensitive at high absorbance
    SimulatedSensor{100.0, 0.33, 0.75, 2, 0.01},
};

const int n_sensors = sizeof(sensors)/sizeof(sensors[0]);    

/** Controls whether the program outputs the calibrated absorbance value. */
constexpr boolean OUTPUT_CALIBRATED_A = false;


/** ---------------------------------------------------------------------------
 *  Setup and loop
 *  ---------------------------------------------------------------------------
 * 
 * Serial output format:
 *
 * For each sampling interval, the Arduino sends a sequence of bytes:
 *
 *     [calibrated?] [sensor_1] [sensor_2] ... [sensor_N] [n_sensors] '\n'
 *
 * - Each sensor value is a byte (0-255) representing absorbance.
 * - If OUTPUT_CALIBRATED_A = true, the first value is the calibrated
 *   absorbance.
 * - The last byte ('n_sensors') indicates how many sensor values were sent
 *   (including the calibrated one, if enabled) for integrity checking.
 * - Each sequence ends with a newline ('\n') for synchronization.
 *
 * Examples:
 * 
 *   OUTPUT_CALIBRATED_A = true, n_sensors = 4  -> [CAL][S1][S2][S3][S4]\n
 *   OUTPUT_CALIBRATED_A = false, n_sensors = 3 -> [S1][S2][S3]\n
 */

/** Time (in ticks) when the next loop should start to maintain simulation 
 * sampling interval.
 */
unsigned long nextTick;

void setup() {
    pinMode(PHYSICAL_SENSOR_PIN, INPUT);
    pinMode(PHYSICAL_OUTPUT_PIN, OUTPUT);
    randomSeed(analogRead(PHYSICAL_SENSOR_PIN));

    // Adjust calibration to environment (read *dark* and full light)
    digitalWrite(PHYSICAL_OUTPUT_PIN, 0);
    for (int i = 0; i < 256; i++)
        accumulate_read_physical_sensor();
    physical_dark = get_and_reset_accumulated_reads();
    
    // Adjust calibration to environment (read dark and *full* light)
    digitalWrite(PHYSICAL_OUTPUT_PIN, 1);
    for (int i = 0; i < 256; i++)
        accumulate_read_physical_sensor();
    physical_full = get_and_reset_accumulated_reads();

    Serial.begin(115200);
    nextTick = millis();  // First loop will start immediately
}

void loop() {

    // Accumulate sensor readings during sampling period
    // TODO: this effectively reduces response time of all sensors, check if it
    // is a problem
    if (millis() < nextTick) {
        accumulate_read_physical_sensor();
        return;
    }

    nextTick += simulation_dt_ms;

    // Convert averaged raw readings to absorbance
    const float real_A = physical_to_A(get_and_reset_accumulated_reads());

    // Send calibrated absorbance reading, if enabled
    if (OUTPUT_CALIBRATED_A) {
        Serial.write(round(max(0, min(real_A*to_byte, UINT8_MAX))));
    }

    // Simulate and send sensors outputs
    for (int i = 0; i < n_sensors; i++) {
        uint8_t sensor_output = sensors[i].update(real_A);
        Serial.write(sensor_output);
    }

    // Send total number of transmitted values (for integrity checking)
    Serial.write(OUTPUT_CALIBRATED_A + n_sensors);
    // Send a new line (for synchronization)
    Serial.write('\n');
}
