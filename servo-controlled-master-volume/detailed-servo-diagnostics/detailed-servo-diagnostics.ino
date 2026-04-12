#include <Servo.h> // Include the Servo library

// --- Pin Definitions ---
const int POT_PIN = A0;  // Analog pin for potentiometer
const int SERVO_PIN = 9; // Digital pin for servo control

// --- Potentiometer Configuration ---
// IMPORTANT: You might need to adjust these values
// after observing the 'Smoothed Pot:' output in the Serial Monitor.
// These should be the actual usable electrical range of your potentiometer,
// where the servo *just starts* and *just stops* moving.
const int CUSTOM_POT_MIN = 0;   // Example: Change to 25 if your pot starts outputting meaningful values at 25
const int CUSTOM_POT_MAX = 1023; // Example: Change to 1000 if your pot stops outputting meaningful values at 1000

// --- Servo Configuration (using your calibrated values) ---
// These are the values you determined: 845us to 2160us
const int SERVO_MIN_PULSE_US = 845;  // Your calibrated minimum pulse width
const int SERVO_MAX_PULSE_US = 2160; // Your calibrated maximum pulse width

// This is the angle range you visually observed your servo moving through
// (e.g., 0 degrees at min pulse, 225 degrees at max pulse)
const int SERVO_ACTUAL_MIN_ANGLE = 0;
const int SERVO_ACTUAL_MAX_ANGLE = 225; // Your observed total range (180 + 45)

// --- Other Constants ---
const float ARDUINO_ANALOG_VOLTAGE = 5.0; // Arduino's analog reference voltage (usually 5.0V for Uno/Nano)
const int ANALOG_MAX_VALUE = 1023;       // Maximum value from analogRead()

// For potentiometer smoothing (a simple rolling average)
const int POT_READINGS_TO_AVERAGE = 10;
int potReadings[POT_READINGS_TO_AVERAGE];
int readIndex = 0;
long totalPotValue = 0;
int smoothedPotValue = 0;

Servo myServo; // Create a Servo object

void setup() {
  Serial.begin(9600); // Initialize serial communication
  Serial.println("--- Potentiometer to Servo Live Monitor ---");
  Serial.println("Configuring servo with pulse range: " + String(SERVO_MIN_PULSE_US) + "us to " + String(SERVO_MAX_PULSE_US) + "us");
  Serial.println("Mapping potentiometer (" + String(CUSTOM_POT_MIN) + "-" + String(CUSTOM_POT_MAX) + ") to servo angles (" + String(SERVO_ACTUAL_MIN_ANGLE) + "-" + String(SERVO_ACTUAL_MAX_ANGLE) + " deg)");
  Serial.println("----------------------------------------");

  // Attach the servo using your calibrated minimum and maximum pulse widths.
  // This tells the Servo library the true electrical limits of your servo.
  myServo.attach(SERVO_PIN, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);

  // Initialize the smoothing array with initial readings
  for (int i = 0; i < POT_READINGS_TO_AVERAGE; i++) {
    potReadings[i] = analogRead(POT_PIN);
    totalPotValue += potReadings[i];
  }
  smoothedPotValue = totalPotValue / POT_READINGS_TO_AVERAGE;

  // Set servo to an initial position based on the current pot value
  // This helps avoid jumps at startup
  int initialMappedAngle = map(smoothedPotValue, CUSTOM_POT_MIN, CUSTOM_POT_MAX, SERVO_ACTUAL_MIN_ANGLE, SERVO_ACTUAL_MAX_ANGLE);
  initialMappedAngle = constrain(initialMappedAngle, SERVO_ACTUAL_MIN_ANGLE, SERVO_ACTUAL_MAX_ANGLE);
  long initialPulseWidth = map(initialMappedAngle, SERVO_ACTUAL_MIN_ANGLE, SERVO_ACTUAL_MAX_ANGLE, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
  initialPulseWidth = constrain(initialPulseWidth, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
  myServo.writeMicroseconds(initialPulseWidth);
}

void loop() {
  // --- 1. Read Potentiometer and Apply Smoothing ---
  int rawPotValue = analogRead(POT_PIN); // Read the raw analog value

  // Update the rolling average
  totalPotValue = totalPotValue - potReadings[readIndex]; // Subtract the oldest reading
  potReadings[readIndex] = rawPotValue;                    // Store the new reading
  totalPotValue = totalPotValue + potReadings[readIndex]; // Add the new reading to the total
  readIndex = (readIndex + 1) % POT_READINGS_TO_AVERAGE;   // Move to the next index
  smoothedPotValue = totalPotValue / POT_READINGS_TO_AVERAGE; // Calculate the new average

  // --- 2. Calculate Incoming Voltage at A0 ---
  float incomingVoltage = (float)rawPotValue / ANALOG_MAX_VALUE * ARDUINO_ANALOG_VOLTAGE;

  // --- 3. Map Smoothed Potentiometer Value to Servo Angle ---
  // This maps the potentiometer's usable range to the servo's actual observed angle range.
  int mappedAngle = map(smoothedPotValue, CUSTOM_POT_MIN, CUSTOM_POT_MAX, SERVO_ACTUAL_MIN_ANGLE, SERVO_ACTUAL_MAX_ANGLE);

  // Ensure the mapped angle does not exceed the servo's defined physical limits
  mappedAngle = constrain(mappedAngle, SERVO_ACTUAL_MIN_ANGLE, SERVO_ACTUAL_MAX_ANGLE);

  // --- 4. Convert Mapped Angle to Commanded Pulse Width ---
  // Crucially, this maps our desired angle (0-225) to the *calibrated* pulse width range (845-2160us).
  // We use map() again because Servo.write() internally might still constrain to 0-180 degrees.
  long commandedPulseWidth = map(mappedAngle, SERVO_ACTUAL_MIN_ANGLE, SERVO_ACTUAL_MAX_ANGLE, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);

  // Ensure the pulse width stays within your calibrated min/max to prevent erratic behavior
  commandedPulseWidth = constrain(commandedPulseWidth, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);

  // --- 5. Command the Servo ---
  myServo.writeMicroseconds(commandedPulseWidth);

  // --- Log All Values to Serial Monitor ---
  Serial.print("Raw Pot: ");
  Serial.print(rawPotValue);
  Serial.print(" | Smoothed Pot: ");
  Serial.print(smoothedPotValue);
  Serial.print(" | A0 Voltage: ");
  Serial.print(incomingVoltage, 2); // Print voltage with 2 decimal places
  Serial.print("V | Mapped Angle: ");
  Serial.print(mappedAngle);
  Serial.print(" deg | Commanded Pulse: ");
  Serial.print(commandedPulseWidth);
  Serial.println(" us");

  delay(50); // Small delay to make logs readable and for system stability
}