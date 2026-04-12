#include <Servo.h>  // Include the Servo library
#include <EEPROM.h> // Include the EEPROM library for non-volatile storage

// --- Pin Definitions ---
const int POT_PIN = A0;   // Analog pin for potentiometer
const int SERVO_PIN = 9;  // Digital pin for servo control
const int BUTTON_PIN = 2; // Digital pin for the push button (using INPUT_PULLUP)

// Single LED Pin - IMPORTANT: Connect a resistor (e.g., 220-330 Ohm) in series!
const int LED_PIN = 8; // Using digital pin 8 for the single LED
const int MODE_SWITCH_PIN = 4; // Digital pin for the mode toggle switch (using INPUT_PULLUP)

// --- Constants for Mode and Button ---
const long LONG_PRESS_DURATION = 2000; // 2 seconds for a long press

// --- YOUR POTENTIOMETER'S USABLE ELECTRICAL RANGE ---
// Based on your last successful test, these should map correctly now.
// If you still experience dead zones, re-run the previous test to find exact values.
const int CUSTOM_POT_MIN = 0;
const int CUSTOM_POT_MAX = 1023;

// --- YOUR CALIBRATED WIDE-ANGLE SERVO RANGES ---
// These are the values you determined from calibration:
const int SERVO_MIN_PULSE_US = 845;   // Your calibrated minimum pulse width
const int SERVO_MAX_PULSE_US = 2160;  // Your calibrated maximum pulse width
const int SERVO_ACTUAL_MIN_ANGLE = 0;   // Assuming your lowest pulse width maps to 0 degrees
const int SERVO_ACTUAL_MAX_ANGLE = 225; // Your observed total movement (180 + 45 = 225 degrees)


// --- LED Blinking Pattern Durations (milliseconds) ---
// For SET_LOW_MODE: Short ON, Long OFF
const long LOW_MODE_ON_TIME = 100;
const long LOW_MODE_OFF_TIME = 900;

// For SET_HIGH_MODE: Short ON, Shorter OFF
const long HIGH_MODE_ON_TIME = 100;
const long HIGH_MODE_OFF_TIME = 400;

// For STORE_FEEDBACK: Very rapid flashing
const long STORE_FEEDBACK_TOGGLE_TIME = 50; // ON and OFF duration for rapid flash
const unsigned long STORE_FEEDBACK_DURATION = 500; // Total duration of the rapid flash


// --- PotentiometerReader Class (REMOVED - NO SMOOTHING) ---
// This class is no longer used as per your request to remove smoothing.


// --- 1. ServoController Class ---
// This class handles controlling the servo based on mapped angles and uses writeMicroseconds.
class ServoController {
public:
  // _minAngle and _maxAngle here define the *desired output angle range* for the servo (e.g., 0-225)
  ServoController(int pin, int minAngle, int maxAngle)
    : _pin(pin), _minAngle(minAngle), _maxAngle(maxAngle) {}
    
  void attach(int initialPotValue, int potMin, int potMax) {
    // Attach the servo using the calibrated pulse widths.
    // This tells the Servo library the true electrical limits of your servo.
    _servo.attach(_pin, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
    // Set initial position based on current pot value
    writeMappedAngle(initialPotValue, potMin, potMax);
  }

  void writeMappedAngle(int potValue, int potMin, int potMax) {
    // Map the potentiometer's input range to the servo's desired angle range
    int targetAngle = map(potValue, potMin, potMax, _minAngle, _maxAngle);
    // Constrain the target angle to ensure it stays within the defined physical limits
    targetAngle = constrain(targetAngle, _minAngle, _maxAngle);
    
    // Convert the target angle to a pulse width using the calibrated pulse range.
    // This is crucial for wide-angle servos, as Servo.write(angle) might internally
    // constrain to 0-180 degrees. By doing this mapping explicitly, we ensure
    // 0-225 degrees truly maps to 845-2160us.
    long commandedPulseWidth = map(targetAngle, _minAngle, _maxAngle, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
    
    // Constrain the pulse width to the known valid range (just in case of mapping edge cases)
    commandedPulseWidth = constrain(commandedPulseWidth, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
    
    _servo.writeMicroseconds(commandedPulseWidth); // Command the servo directly with microseconds
    _currentAngle = targetAngle; // Store the current angle for getCurrentAngle()
  }

  void writeAngle(int angle) {
    // Constrain the input angle to the servo's defined physical range
    int constrainedAngle = constrain(angle, _minAngle, _maxAngle);
    
    // Convert the constrained angle to a pulse width
    long commandedPulseWidth = map(constrainedAngle, _minAngle, _maxAngle, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
    commandedPulseWidth = constrain(commandedPulseWidth, SERVO_MIN_PULSE_US, SERVO_MAX_PULSE_US);
    
    _servo.writeMicroseconds(commandedPulseWidth); // Command the servo directly
    _currentAngle = constrainedAngle; // Store the current angle
  }

  int getCurrentAngle() const { return _currentAngle; }
private:
  Servo _servo;
  int _pin;
  int _minAngle;
  int _maxAngle;
  int _currentAngle; // Stores the last commanded angle
};

// --- 2. PushButton Class ---
// Handles debouncing and detecting short/long presses for the button.
class PushButton {
public:
  PushButton(int pin, long longPressDuration)
    : _pin(pin), _longPressDuration(longPressDuration),
      _lastButtonState(HIGH), _buttonPressStartTime(0), _longPressActive(false),
      _shortPressTriggered(false)
  {
    pinMode(_pin, INPUT_PULLUP); // Use internal pull-up resistor
  }
  void update() {
    int currentButtonState = digitalRead(_pin);

    // Detect change in button state
    if (currentButtonState != _lastButtonState) {
      _lastButtonStateChangeTime = millis();
    }

    // Debounce logic
    if ((millis() - _lastButtonStateChangeTime) > _debounceDelay) {
      if (currentButtonState != _debouncedButtonState) {
        _debouncedButtonState = currentButtonState;

        if (_debouncedButtonState == LOW) { // Button just pressed down
          _buttonPressStartTime = millis();
          _longPressActive = false;
          _shortPressTriggered = false; // Reset short press flag
        } else { // Button just released
          unsigned long pressDuration = millis() - _buttonPressStartTime;
          if (pressDuration >= _debounceDelay && pressDuration < _longPressDuration) {
            _shortPressTriggered = true; // Mark short press if duration is within range
          }
          _longPressActive = false; // Reset long press flag
        }
      }
    }
    _lastButtonState = currentButtonState;

    // Check for ongoing long press (button held down past duration)
    if (_debouncedButtonState == LOW &&
        (millis() - _buttonPressStartTime >= _longPressDuration) &&
        !_longPressActive) {
      _longPressActive = true; // Mark long press as active
    }
  }

  // Returns true once if a long press was just completed
  bool isLongPressTriggered() {
    if (_longPressActive) {
      _longPressActive = false; // Consume the event
      return true;
    }
    return false;
  }

  // Returns true once if a short press was just completed
  bool isShortPressTriggered() {
    if (_shortPressTriggered) {
      _shortPressTriggered = false; // Consume the event
      return true;
    }
    return false;
  }
private:
  int _pin;
  long _longPressDuration;
  int _lastButtonState;
  unsigned long _buttonPressStartTime;
  bool _longPressActive;
  bool _shortPressTriggered;
  int _debouncedButtonState = HIGH;
  unsigned long _lastButtonStateChangeTime = 0;
  const long _debounceDelay = 50; // Standard debounce delay
};

// --- 3. PositionStore Class ---
// Handles saving and loading servo positions to/from EEPROM.
class PositionStore {
public:
  PositionStore(int magicAddr, int lowAddr, int highAddr)
    : _magicAddr(magicAddr), _lowAddr(lowAddr), _highAddr(highAddr),
      _lowPosition(0), _highPosition(1) {}
      
  void loadPositions() {
    int storedMagicNumber = 0;
    // Read a magic number to check if EEPROM has been initialized
    EEPROM.get(_magicAddr, storedMagicNumber); 
    if (storedMagicNumber == MAGIC_NUMBER) {
      // If magic number matches, load stored positions
      EEPROM.get(_lowAddr, _lowPosition);
      EEPROM.get(_highAddr, _highPosition);
      Serial.print("Loaded from EEPROM: Low = ");
      Serial.print(_lowPosition);
      Serial.print(", High = ");
      Serial.println(_highPosition);
    } else {
      // If not initialized, set default values and write magic number
      Serial.println("EEPROM uninitialized or corrupted. Setting default values.");
      _lowPosition = DEFAULT_LOW_POSITION;
      _highPosition = DEFAULT_HIGH_POSITION;
      EEPROM.put(_magicAddr, MAGIC_NUMBER);
      EEPROM.put(_lowAddr, _lowPosition);
      EEPROM.put(_highAddr, _highPosition);
      Serial.print("Initialized EEPROM: Low = ");
      Serial.print(_lowPosition);
      Serial.print(", High = ");
      Serial.println(_highPosition);
    }
  }

  void storePosition(int currentAngle, bool isLowSet) {
    bool updated = false;
    // If default values are still in place, set both Low and High to current angle
    if (_lowPosition == DEFAULT_LOW_POSITION && _highPosition == DEFAULT_HIGH_POSITION) {
      _lowPosition = currentAngle;
      _highPosition = currentAngle;
      Serial.println("Initial user values set (Low and High now same).");
      updated = true;
    }
    else {
        if (isLowSet) { // Store as Low position
            _lowPosition = currentAngle;
            Serial.println("New Low value stored.");
            updated = true;
        } else { // Store as High position
            _highPosition = currentAngle;
            Serial.println("New High value stored.");
            updated = true;
        }
    }
    
    // Auto-correct order if Low becomes greater than High
    if (_lowPosition > _highPosition) {
        int temp = _lowPosition;
        _lowPosition = _highPosition;
        _highPosition = temp;
        Serial.println("Auto-corrected: Low and High values swapped to maintain order.");
        updated = true;
    }

    if (updated) {
      // Save updated positions to EEPROM
      EEPROM.put(_lowAddr, _lowPosition);
      EEPROM.put(_highAddr, _highPosition);
      // Ensure magic number is also written if it somehow got corrupted
      int storedMagicNumber = 0;
      EEPROM.get(_magicAddr, storedMagicNumber);
      if (storedMagicNumber != MAGIC_NUMBER) {
          EEPROM.put(_magicAddr, MAGIC_NUMBER);
      }
      Serial.print("Current Stored Range: Low = ");
      Serial.print(_lowPosition);
      Serial.print(", High = ");
      Serial.println(_highPosition);
    } else {
      Serial.println("No change to Low/High boundaries. Not writing to EEPROM.");
    }
  }

  int getLowPosition() const { return _lowPosition; }
  int getHighPosition() const { return _highPosition; }
private:
  const int _magicAddr;
  const int _lowAddr;
  const int _highAddr;
  int _lowPosition;
  int _highPosition;
  static const int MAGIC_NUMBER = 0xABCD; // A unique number to identify initialized EEPROM
  static const int DEFAULT_LOW_POSITION = 0; // Default state for uninitialized EEPROM
  static const int DEFAULT_HIGH_POSITION = 1; // Default state for uninitialized EEPROM
};

// --- FORMAL DEFINITION FOR STATIC CONST MEMBER ---
const int PositionStore::MAGIC_NUMBER;

// --- 4. LedIndicator Class ---
// Manages the blinking patterns and state of a single LED.
class LedIndicator {
public:
  LedIndicator(int pin)
    : _ledPin(pin), _blinkingActive(false), _onDuration(0), _offDuration(0),
      _lastToggleTime(0), _ledState(LOW), _feedbackBlinkActive(false), _feedbackStartTime(0) {
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW); // Ensure LED is off initially
  }

  // Turns the LED completely off
  void turnOff() {
    _blinkingActive = false;
    _feedbackBlinkActive = false; // Stop any ongoing feedback blink
    digitalWrite(_ledPin, LOW);
  }

  // Sets a continuous blinking pattern for modes (e.g., slow blink for SET_LOW_MODE)
  void setBlinkPattern(long onDuration, long offDuration) {
    _blinkingActive = true;
    _feedbackBlinkActive = false; // Stop any ongoing feedback blink
    _onDuration = onDuration;
    _offDuration = offDuration;
    _lastToggleTime = millis(); // Reset timer to start new pattern immediately
    _ledState = HIGH;           // Start new pattern with LED ON
    digitalWrite(_ledPin, HIGH);
  }

  // Starts a rapid, short-duration blink for feedback (e.g., after storing a position)
  void startFeedbackBlink() {
    _blinkingActive = false;    // Stop any mode blinking
    _feedbackBlinkActive = true;
    _feedbackStartTime = millis(); // Record when the feedback blink started
    _lastToggleTime = millis();    // Initialize for immediate first toggle
    _ledState = HIGH;              // Force first state ON
    digitalWrite(_ledPin, HIGH);
  }

  // This function must be called repeatedly in loop() to update the LED's state
  void update() {
    if (_feedbackBlinkActive) {
      // Manage the rapid flashing for the defined feedback duration
      if (millis() - _feedbackStartTime >= STORE_FEEDBACK_DURATION) {
        _feedbackBlinkActive = false; // Stop feedback blink after duration
        digitalWrite(_ledPin, LOW);   // Ensure LED is OFF after feedback
      } else {
        // Continue rapid flashing
        if (millis() - _lastToggleTime >= STORE_FEEDBACK_TOGGLE_TIME) {
          _ledState = !_ledState; // Toggle LED state (ON to OFF, or OFF to ON)
          digitalWrite(_ledPin, _ledState ? HIGH : LOW);
          _lastToggleTime = millis(); // Update last toggle time
        }
      }
    } else if (_blinkingActive) {
      // Manage the current mode's regular blink pattern
      if (_ledState == HIGH) { // If LED is currently ON
        if (millis() - _lastToggleTime >= _onDuration) {
          digitalWrite(_ledPin, LOW); // Turn OFF
          _ledState = LOW;
          _lastToggleTime = millis();
        }
      } else { // If LED is currently OFF
        if (millis() - _lastToggleTime >= _offDuration) {
          digitalWrite(_ledPin, HIGH); // Turn ON
          _ledState = HIGH;
          _lastToggleTime = millis();
        }
      }
    } else {
      // If no blinking is active (e.g., in NORMAL_MODE or ALTERNATIVE_HOLD_MODE), ensure LED is off
      digitalWrite(_ledPin, LOW);
    }
  }

private:
  int _ledPin;
  bool _blinkingActive;     // True if a regular mode blink pattern is active
  long _onDuration;         // ON time for regular blink pattern
  long _offDuration;        // OFF time for regular blink pattern
  unsigned long _lastToggleTime; // Last time the LED state was changed
  bool _ledState;           // Current state of the LED (HIGH/LOW)

  bool _feedbackBlinkActive;    // True if feedback blink is active
  unsigned long _feedbackStartTime; // When the feedback blink started
};


// --- Global Objects ---
// Define EEPROM addresses for clarity
const int EEPROM_MAGIC_ADDR = 0;
const int EEPROM_LOW_ADDR = sizeof(int);
const int EEPROM_HIGH_ADDR = sizeof(int) * 2;

// Initialize ServoController with the ACTUAL physical angle range
ServoController servoCtrl(SERVO_PIN, SERVO_ACTUAL_MIN_ANGLE, SERVO_ACTUAL_MAX_ANGLE);
PushButton storeButton(BUTTON_PIN, LONG_PRESS_DURATION);
PositionStore posStore(EEPROM_MAGIC_ADDR, EEPROM_LOW_ADDR, EEPROM_HIGH_ADDR);
LedIndicator ledIndicator(LED_PIN);

// --- Variables for Toggle Switch Debouncing ---
int lastModeSwitchState = HIGH;      // Last read state of the toggle switch
unsigned long lastModeSwitchChangeTime = 0; // Last time the mode switch state changed
const long MODE_SWITCH_DEBOUNCE_DELAY = 50; // Debounce time for the toggle switch


// --- System Mode Enumerations (for manual control sub-modes) ---
enum SystemMode {
  NORMAL_MODE,   // Servo controlled by pot, LED OFF
  SET_LOW_MODE,  // LED blinking (short ON, long OFF), long press stores low
  SET_HIGH_MODE, // LED blinking (short ON, shorter OFF), long press stores high
};
SystemMode currentManualMode = NORMAL_MODE; // Start in normal manual control mode


// --- Main Control Mode Enumeration (for switch between pot control and hold) ---
enum MainControlMode {
  MANUAL_POT_CONTROL_MODE, // The original potentiometer-controlled mode
  ALTERNATIVE_HOLD_MODE    // The new mode where servo holds position
};
MainControlMode currentMainMode = MANUAL_POT_CONTROL_MODE; // Start in potentiometer control mode


// --- Arduino Setup Function ---
void setup() {
  Serial.begin(9600);
  Serial.println("--- System Booting ---");

  // Setup the mode switch pin
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  lastModeSwitchState = digitalRead(MODE_SWITCH_PIN); // Read initial state

  // Load stored positions from EEPROM (handles initialization if blank)
  posStore.loadPositions();

  // Get initial raw potentiometer value for servo attachment (NO SMOOTHING)
  int initialRawPotValue = analogRead(POT_PIN);

  // Attach servo and set initial position based on current pot value
  // Uses CUSTOM_POT_MIN/MAX (0-1023) as confirmed.
  servoCtrl.attach(initialRawPotValue, CUSTOM_POT_MIN, CUSTOM_POT_MAX);

  // Initialize LED to off for Normal Mode (initial state)
  ledIndicator.turnOff();

  Serial.println("--- System Ready ---");
  Serial.println("Servo control is direct from Potentiometer (NO SMOOTHING).");
}

// --- Arduino Loop Function ---
void loop() {
  // Always update store button state
  storeButton.update();

  // --- Handle Main Mode Toggle Switch ---
  int currentModeSwitchState = digitalRead(MODE_SWITCH_PIN);

  // Debounce the toggle switch and detect state change
  if (currentModeSwitchState != lastModeSwitchState) {
    lastModeSwitchChangeTime = millis();
  }

  if ((millis() - lastModeSwitchChangeTime) > MODE_SWITCH_DEBOUNCE_DELAY) {
    // Check if debounced state is actually different from current main mode
    if (currentModeSwitchState == LOW && currentMainMode == MANUAL_POT_CONTROL_MODE) {
      currentMainMode = ALTERNATIVE_HOLD_MODE;
      ledIndicator.turnOff(); // Ensure LED is off in alternative mode
      Serial.println("Switched to ALTERNATIVE_HOLD_MODE. Servo will hold its current position.");
      // Capture the current servo angle to hold it
      servoCtrl.writeAngle(servoCtrl.getCurrentAngle());
    } else if (currentModeSwitchState == HIGH && currentMainMode == ALTERNATIVE_HOLD_MODE) {
      currentMainMode = MANUAL_POT_CONTROL_MODE;
      ledIndicator.turnOff(); // Ensure LED is off when returning to normal pot mode
      Serial.println("Switched to MANUAL_POT_CONTROL_MODE.");
      // Re-sync servo to current potentiometer position immediately (NO SMOOTHING)
      int rawPotValue = analogRead(POT_PIN);
      servoCtrl.writeMappedAngle(rawPotValue, CUSTOM_POT_MIN, CUSTOM_POT_MAX);
      Serial.println("Servo is now controlled by Potentiometer (Manual Mode).");
    }
  }
  lastModeSwitchState = currentModeSwitchState; // Update last state for next iteration


  // --- Main Control Mode Logic ---
  switch (currentMainMode) {
    case MANUAL_POT_CONTROL_MODE:
      // Read raw potentiometer value directly (NO SMOOTHING)
      int rawPotValue = analogRead(POT_PIN);
      
      // Map pot range to the servo's actual angle range
      servoCtrl.writeMappedAngle(rawPotValue, CUSTOM_POT_MIN, CUSTOM_POT_MAX);

      // --- Mode State Machine (Your original SystemMode logic for setting limits) ---
      switch (currentManualMode) {
        case NORMAL_MODE:
          // In Normal Mode, short press transitions to SET_LOW_MODE
          if (storeButton.isShortPressTriggered()) {
            currentManualMode = SET_LOW_MODE;
            ledIndicator.setBlinkPattern(LOW_MODE_ON_TIME, LOW_MODE_OFF_TIME); // Low Mode blink pattern
            // LOGGING ADDED: Indicate entering SET_LOW_MODE
            Serial.println("\n--- Entering SET_LOW_MODE: Adjust pot to desired LOW angle, then LONG PRESS to save. ---");
          }
          // Long press in Normal Mode does nothing
          if (storeButton.isLongPressTriggered()) {
            Serial.println("Long press in Manual NORMAL_MODE has no action.");
          }
          break;

        case SET_LOW_MODE:
          // Short press transitions to SET_HIGH_MODE
          if (storeButton.isShortPressTriggered()) {
            currentManualMode = SET_HIGH_MODE;
            ledIndicator.setBlinkPattern(HIGH_MODE_ON_TIME, HIGH_MODE_OFF_TIME); // High Mode blink pattern
            // LOGGING ADDED: Indicate entering SET_HIGH_MODE
            Serial.println("\n--- Entering SET_HIGH_MODE: Adjust pot to desired HIGH angle, then LONG PRESS to save. ---");
          }
          // Long press stores the current angle as Low
          if (storeButton.isLongPressTriggered()) {
            // LOGGING ADDED: Show angle being stored
            Serial.print("Attempting to store LOW angle: ");
            Serial.print(servoCtrl.getCurrentAngle());
            Serial.println(" degrees.");
            posStore.storePosition(servoCtrl.getCurrentAngle(), true); // 'true' indicates setting Low
            ledIndicator.startFeedbackBlink(); // Start rapid blink feedback
            currentManualMode = NORMAL_MODE; // Return to normal mode after storing
            Serial.println("Manual Sub-Mode: NORMAL (LED: Off, after rapid blink)");
          }
          break;

        case SET_HIGH_MODE:
          // Short press transitions back to NORMAL_MODE
          if (storeButton.isShortPressTriggered()) {
            currentManualMode = NORMAL_MODE;
            ledIndicator.turnOff(); // Turn LED off when back to normal
            Serial.println("\n--- Exiting SET_HIGH_MODE: Back to NORMAL_MODE (LED: Off). ---");
          }
          // Long press stores the current angle as High
          if (storeButton.isLongPressTriggered()) {
            // LOGGING ADDED: Show angle being stored
            Serial.print("Attempting to store HIGH angle: ");
            Serial.print(servoCtrl.getCurrentAngle());
            Serial.println(" degrees.");
            posStore.storePosition(servoCtrl.getCurrentAngle(), false); // 'false' indicates setting High
            ledIndicator.startFeedbackBlink(); // Start rapid blink feedback
            currentManualMode = NORMAL_MODE; // Return to normal mode after storing
            Serial.println("Manual Sub-Mode: NORMAL (LED: Off, after rapid blink)");
          }
          break;
      }
      break; // End of MANUAL_POT_CONTROL_MODE

    case ALTERNATIVE_HOLD_MODE:
      // In this mode, the servo simply holds its last commanded position.
      // The potentiometer and the 'storeButton' have no effect on the servo.
      // The LED is explicitly turned off when entering this mode and stays off.
      // No continuous servo updates from pot needed here.
      break; // End of ALTERNATIVE_HOLD_MODE
  }

  // Always update the LED (for blinking or solid state, and turning off blink)
  ledIndicator.update();

  // Small delay for smooth operation and readability
  delay(10);
}