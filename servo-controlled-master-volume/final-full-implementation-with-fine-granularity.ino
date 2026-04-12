#include <Servo.h>  // Include the Servo library
#include <EEPROM.h> // Include the EEPROM library for non-volatile storage
#include <MIDI.h>   // Include the MIDI Library from Forty Seven Effects

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
// For NORMAL_MODE: Very slow blink to indicate active manual control
const long NORMAL_MODE_ON_TIME = 50;
const long NORMAL_MODE_OFF_TIME = 1950; // Total 2 second cycle for Normal Mode

// For SET_LOW_MODE: Short ON, Long OFF
const long LOW_MODE_ON_TIME = 100;
const long LOW_MODE_OFF_TIME = 900;

// For SET_HIGH_MODE: Short ON, Shorter OFF
const long HIGH_MODE_ON_TIME = 100;
const long HIGH_MODE_OFF_TIME = 400;

// For STORE_FEEDBACK: Very rapid flashing after saving
const long STORE_FEEDBACK_TOGGLE_TIME = 50; // ON and OFF duration for rapid flash
const unsigned long STORE_FEEDBACK_DURATION = 500; // Total duration of the rapid flash

// For momentary button press feedback
const long MOMENTARY_FLASH_DURATION = 30; // Very brief flash for button feedback


// --- MIDI Constants ---
const byte MIDI_CHANNEL = 1;    // MIDI Channel to listen on (0-15, but usually 1-16. Library uses 0-15)
const byte MIDI_CC_NUMBER = 7;  // Control Change Number for Main Volume

// --- Forward declaration for LedIndicator (needed because PushButton uses it) ---
class LedIndicator;

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

// --- 4. LedIndicator Class ---
// Manages the blinking patterns and state of a single LED.
class LedIndicator {
public:
  LedIndicator(int pin)
    : _ledPin(pin), _blinkingActive(false), _onDuration(0), _offDuration(0),
      _lastToggleTime(0), _ledState(LOW), _feedbackBlinkActive(false), _feedbackStartTime(0),
      _momentaryFlashActive(false), _momentaryFlashStartTime(0) { // Initialize new members
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW); // Ensure LED is off initially
  }

  // Turns the LED completely off (e.g., for MIDI mode)
  void turnOff() {
    _blinkingActive = false;
    _feedbackBlinkActive = false;
    _momentaryFlashActive = false; // Ensure momentary flash is also off
    digitalWrite(_ledPin, LOW);
  }

  // Sets a continuous blinking pattern for modes (e.g., slow blink for SET_LOW_MODE)
  void setBlinkPattern(long onDuration, long offDuration) {
    _blinkingActive = true;
    _feedbackBlinkActive = false; // Stop any ongoing feedback blink
    _momentaryFlashActive = false; // Stop any ongoing momentary flash
    _onDuration = onDuration;
    _offDuration = offDuration;
    _lastToggleTime = millis(); // Reset timer to start new pattern immediately
    _ledState = HIGH;           // Start new pattern with LED ON
    digitalWrite(_ledPin, HIGH);
  }

  // Starts a rapid, short-duration blink for feedback (e.g., after storing a position)
  void startFeedbackBlink() {
    _blinkingActive = false;    // Stop any mode blinking
    _momentaryFlashActive = false; // Stop any momentary flashes
    _feedbackBlinkActive = true;
    _feedbackStartTime = millis(); // Record when the feedback blink started
    _lastToggleTime = millis();    // Initialize for immediate first toggle
    _ledState = HIGH;              // Force first state ON
    digitalWrite(_ledPin, HIGH);
  }

  // Starts a very brief, high-priority flash for button press confirmation
  void startMomentaryFlash() {
    _momentaryFlashActive = true;
    _momentaryFlashStartTime = millis();
    digitalWrite(_ledPin, HIGH); // Turn on immediately
  }

  // New public method to check if a specific blink pattern is currently active
  bool isCurrentPattern(long onDuration, long offDuration) const {
    return _blinkingActive && (_onDuration == onDuration) && (_offDuration == offDuration);
  }

  // This function must be called repeatedly in loop() to update the LED's state
  void update() {
    // Priority 1: Momentary flash (for button presses)
    if (_momentaryFlashActive) {
      if (millis() - _momentaryFlashStartTime >= MOMENTARY_FLASH_DURATION) {
        digitalWrite(_ledPin, LOW);
        _momentaryFlashActive = false;
        // Reset _lastToggleTime to ensure the underlying mode's blink pattern
        // restarts correctly after the momentary flash ends.
        _lastToggleTime = millis(); 
      }
      return; // Do not process other blinking modes if momentary flash is active
    }

    // Priority 2: Feedback blink (after storing positions)
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
    } 
    // Priority 3: Regular mode blink pattern
    else if (_blinkingActive) {
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
      // If no blinking is active (e.g., in MIDI_CONTROL_MODE after a momentary flash,
      // or if explicitly turned off), ensure LED is off.
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

  bool _momentaryFlashActive;      // True if a momentary flash is active
  unsigned long _momentaryFlashStartTime; // When the momentary flash started
};

// --- 2. PushButton Class ---
// Handles debouncing and detecting short/long presses for the button.
class PushButton {
public:
  // Constructor now takes a reference to the LedIndicator object
  PushButton(int pin, long longPressDuration, LedIndicator& led) 
    : _pin(pin), _longPressDuration(longPressDuration),
      _lastButtonState(HIGH), _buttonPressStartTime(0), _longPressActive(false),
      _shortPressTriggered(false), _led(led) // Initialize the LedIndicator reference
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
      _led.startMomentaryFlash(); // Provide visual feedback for long press
      return true;
    }
    return false;
  }

  // Returns true once if a short press was just completed
  bool isShortPressTriggered() {
    if (_shortPressTriggered) {
      _shortPressTriggered = false; // Consume the event
      _led.startMomentaryFlash(); // Provide visual feedback for short press
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
  LedIndicator& _led; // Reference to the LED indicator object
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
      // Serial.print("Loaded from EEPROM: Low = "); // Removed Serial.print
      // Serial.print(_lowPosition);
      // Serial.print(", High = ");
      // Serial.println(_highPosition);
    } else {
      // If not initialized, set default values and write magic number
      // Serial.println("EEPROM uninitialized or corrupted. Setting default values."); // Removed Serial.print
      _lowPosition = DEFAULT_LOW_POSITION;
      _highPosition = DEFAULT_HIGH_POSITION;
      EEPROM.put(_magicAddr, MAGIC_NUMBER);
      EEPROM.put(_lowAddr, _lowPosition);
      EEPROM.put(_highAddr, _highPosition);
      // Serial.print("Initialized EEPROM: Low = "); // Removed Serial.print
      // Serial.print(_lowPosition);
      // Serial.print(", High = ");
      // Serial.println(_highPosition);
    }
  }

  void storePosition(int currentAngle, bool isLowSet) {
    bool updated = false;
    // If default values are still in place, set both Low and High to current angle
    if (_lowPosition == DEFAULT_LOW_POSITION && _highPosition == DEFAULT_HIGH_POSITION) {
      _lowPosition = currentAngle;
      _highPosition = currentAngle;
      // Serial.println("Initial user values set (Low and High now same)."); // Removed Serial.print
      updated = true;
    }
    else {
        if (isLowSet) { // Store as Low position
            _lowPosition = currentAngle;
            // Serial.println("New Low value stored."); // Removed Serial.print
            updated = true;
        } else { // Store as High position
            _highPosition = currentAngle;
            // Serial.println("New High value stored."); // Removed Serial.print
            updated = true;
        }
    }
    
    // Auto-correct order if Low becomes greater than High
    if (_lowPosition > _highPosition) {
        int temp = _lowPosition;
        _lowPosition = _highPosition;
        _highPosition = temp;
        // Serial.println("Auto-corrected: Low and High values swapped to maintain order."); // Removed Serial.print
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
      // Serial.print("Current Stored Range: Low = "); // Removed Serial.print
      // Serial.print(_lowPosition);
      // Serial.print(", High = ");
      // Serial.println(_highPosition);
    } else {
      // Serial.println("No change to Low/High boundaries. Not writing to EEPROM."); // Removed Serial.print
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


// --- Global Objects ---
// Define EEPROM addresses for clarity
const int EEPROM_MAGIC_ADDR = 0;
const int EEPROM_LOW_ADDR = sizeof(int);
const int EEPROM_HIGH_ADDR = sizeof(int) * 2;

// Initialize ServoController with the ACTUAL physical angle range
ServoController servoCtrl(SERVO_PIN, SERVO_ACTUAL_MIN_ANGLE, SERVO_ACTUAL_MAX_ANGLE);
// LedIndicator needs to be initialized before PushButton because PushButton takes it as a reference
LedIndicator ledIndicator(LED_PIN);
PushButton storeButton(BUTTON_PIN, LONG_PRESS_DURATION, ledIndicator); // Pass ledIndicator reference
PositionStore posStore(EEPROM_MAGIC_ADDR, EEPROM_LOW_ADDR, EEPROM_HIGH_ADDR);


// --- Variables for Toggle Switch Debouncing ---
int lastModeSwitchState = HIGH;      // Last read state of the toggle switch
unsigned long lastModeSwitchChangeTime = 0; // Last time the mode switch state changed
const long MODE_SWITCH_DEBOUNCE_DELAY = 50; // Debounce time for the toggle switch


// --- System Mode Enumerations (for manual control sub-modes) ---
enum SystemMode {
  NORMAL_MODE,   // Servo controlled by pot, LED blinks very slowly
  SET_LOW_MODE,  // LED blinking (short ON, long OFF), long press stores low
  SET_HIGH_MODE, // LED blinking (short ON, shorter OFF), long press stores high
};
SystemMode currentManualMode = NORMAL_MODE; // Start in normal manual control mode


// --- Main Control Mode Enumeration (for switch between pot control and MIDI) ---
enum MainControlMode {
  MANUAL_POT_CONTROL_MODE, // Potentiometer-controlled mode (with calibration sub-modes)
  MIDI_CONTROL_MODE        // MIDI-controlled mode (CC7 on Channel 1), LED OFF
};
MainControlMode currentMainMode = MANUAL_POT_CONTROL_MODE; // Start in potentiometer control mode


// --- MIDI Library Setup ---
// Create a default MIDI instance. This typically uses Serial for MIDI communication.
MIDI_CREATE_DEFAULT_INSTANCE();

// --- MIDI Control Change Callback Function ---
// This function is called by the MIDI library whenever a Control Change message is received.
void handleControlChange(byte channel, byte number, byte value) {
  // Only process MIDI messages if we are in MIDI_CONTROL_MODE
  if (currentMainMode == MIDI_CONTROL_MODE) {
    // Check if the message is on the desired MIDI Channel and Control Change Number
    if (channel == MIDI_CHANNEL && number == MIDI_CC_NUMBER) {
      // Map the incoming MIDI value (0-127) to the stored Low/High angle range.
      // This ensures the servo moves only within the user-defined limits.
      int mappedAngle = map(value, 0, 127, posStore.getLowPosition(), posStore.getHighPosition());
      
      // Command the servo to the new mapped angle
      servoCtrl.writeAngle(mappedAngle);
      
      // No Serial.print here to avoid D0/RX conflict.
    }
  }
}

// --- Arduino Setup Function ---
void setup() {
  // Serial.begin(9600); // Initialize Serial communication for debugging (be aware of D0 conflict)
  // Serial.println("--- System Booting ---"); // Removed Serial.print

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

  // Initialize LED based on initial mode. MANUAL_POT_CONTROL_MODE starts in NORMAL_MODE.
  ledIndicator.setBlinkPattern(NORMAL_MODE_ON_TIME, NORMAL_MODE_OFF_TIME);

  // --- MIDI Setup ---
  // Start the MIDI library. It listens on the default Serial port (D0/RX).
  // MIDI.begin(MIDI_CHANNEL_OMNI) means it will process messages on any channel initially.
  // We then filter by MIDI_CHANNEL inside the callback.
  MIDI.begin(MIDI_CHANNEL_OMNI); 
  // Register the callback function to handle Control Change messages.
  MIDI.setHandleControlChange(handleControlChange);

  // Serial.println("--- System Ready ---"); // Removed Serial.print
  // Serial.println("Mode switch (D4) toggles between Potentiometer control and MIDI control."); // Removed Serial.print
  // Serial.println("In Potentiometer Mode, the button (D2) allows setting Low/High limits."); // Removed Serial.print
  // Serial.println("In MIDI Mode, MIDI CC#7 on Channel 1 controls the servo between stored limits."); // Removed Serial.print
  // Serial.println("!!! IMPORTANT: Disconnect USB for MIDI input on D0/RX to function reliably. !!!"); // Removed Serial.print
}

// --- Arduino Loop Function ---
void loop() {
  // Always update store button state
  storeButton.update();
  
  // Continuously read MIDI messages. This is crucial for MIDI control.
  // This non-blocking call processes any incoming MIDI data.
  MIDI.read(); 

  // --- Handle Main Mode Toggle Switch ---
  // Read the current state of the physical toggle switch
  int currentModeSwitchState = digitalRead(MODE_SWITCH_PIN);

  // Debounce the toggle switch to prevent false readings
  if (currentModeSwitchState != lastModeSwitchState) {
    lastModeSwitchChangeTime = millis(); // Record time of state change
  }

  // After debounce delay, check if the debounced state is actually different
  // and trigger a mode change if necessary.
  if ((millis() - lastModeSwitchChangeTime) > MODE_SWITCH_DEBOUNCE_DELAY) {
    // If the switch is LOW and we are in Potentiometer Control Mode, switch to MIDI Control
    if (currentModeSwitchState == LOW && currentMainMode == MANUAL_POT_CONTROL_MODE) {
      currentMainMode = MIDI_CONTROL_MODE;
      ledIndicator.turnOff(); // Ensure LED is off in MIDI mode
      // Serial.println("Switched to MIDI_CONTROL_MODE. Servo now controlled by MIDI CC#7."); // Removed Serial.print
    } 
    // If the switch is HIGH and we are in MIDI Control Mode, switch back to Potentiometer Control
    else if (currentModeSwitchState == HIGH && currentMainMode == MIDI_CONTROL_MODE) {
      currentMainMode = MANUAL_POT_CONTROL_MODE;
      // When returning to manual mode, set LED to NORMAL_MODE pattern
      currentManualMode = NORMAL_MODE; // Ensure sub-mode is also reset to normal
      ledIndicator.setBlinkPattern(NORMAL_MODE_ON_TIME, NORMAL_MODE_OFF_TIME); 
      // Serial.println("Switched to MANUAL_POT_CONTROL_MODE. Servo now controlled by Potentiometer."); // Removed Serial.print
      // Re-sync servo to current potentiometer position immediately (NO SMOOTHING)
      int rawPotValue = analogRead(POT_PIN);
      servoCtrl.writeMappedAngle(rawPotValue, CUSTOM_POT_MIN, CUSTOM_POT_MAX);
    }
  }
  lastModeSwitchState = currentModeSwitchState; // Update last state for next iteration


  // --- Main Control Mode Logic ---
  switch (currentMainMode) {
    case MANUAL_POT_CONTROL_MODE:
      // In this mode, the potentiometer controls the servo.
      // The button (D2) is used to enter/exit calibration sub-modes (SET_LOW/HIGH).
      
      // Read raw potentiometer value directly (NO SMOOTHING)
      int rawPotValue = analogRead(POT_PIN);
      
      // Map pot range to the servo's actual angle range
      servoCtrl.writeMappedAngle(rawPotValue, CUSTOM_POT_MIN, CUSTOM_POT_MAX);

      // No Serial.print for pot/servo values here to avoid D0/RX conflict.

      // --- Mode State Machine (Your original SystemMode logic for setting limits) ---
      switch (currentManualMode) {
        case NORMAL_MODE:
          // Ensure LED shows the Normal Mode blink pattern if no other blink is active
          if (!ledIndicator.isCurrentPattern(NORMAL_MODE_ON_TIME, NORMAL_MODE_OFF_TIME)) {
              ledIndicator.setBlinkPattern(NORMAL_MODE_ON_TIME, NORMAL_MODE_OFF_TIME);
          }

          // In Normal Mode, short press transitions to SET_LOW_MODE for calibration.
          if (storeButton.isShortPressTriggered()) {
            currentManualMode = SET_LOW_MODE;
            ledIndicator.setBlinkPattern(LOW_MODE_ON_TIME, LOW_MODE_OFF_TIME); // Low Mode blink pattern
            // Serial.println("\n--- Entering SET_LOW_MODE: Adjust pot to desired LOW angle, then LONG PRESS to save. ---"); // Removed Serial.print
          }
          // Long press in Normal Mode currently has no action.
          if (storeButton.isLongPressTriggered()) {
            // Serial.println("Long press in Manual NORMAL_MODE has no action."); // Removed Serial.print
          }
          break;

        case SET_LOW_MODE:
          // Short press transitions to SET_HIGH_MODE for calibration.
          if (storeButton.isShortPressTriggered()) {
            currentManualMode = SET_HIGH_MODE;
            ledIndicator.setBlinkPattern(HIGH_MODE_ON_TIME, HIGH_MODE_OFF_TIME); // High Mode blink pattern
            // Serial.println("\n--- Entering SET_HIGH_MODE: Adjust pot to desired HIGH angle, then LONG PRESS to save. ---"); // Removed Serial.print
          }
          // Long press stores the current angle as the Low limit.
          if (storeButton.isLongPressTriggered()) {
            // Serial.print("Attempting to store LOW angle: "); // Removed Serial.print
            // Serial.print(servoCtrl.getCurrentAngle());
            // Serial.println(" degrees."); // Removed Serial.print
            posStore.storePosition(servoCtrl.getCurrentAngle(), true); // 'true' indicates setting Low
            ledIndicator.startFeedbackBlink(); // Start rapid blink feedback
            currentManualMode = NORMAL_MODE; // Return to normal mode after storing
            // Serial.println("Manual Sub-Mode: NORMAL (LED: Off, after rapid blink)"); // Removed Serial.print
          }
          break;

        case SET_HIGH_MODE:
          // Short press transitions back to NORMAL_MODE, exiting calibration.
          if (storeButton.isShortPressTriggered()) {
            currentManualMode = NORMAL_MODE;
            ledIndicator.setBlinkPattern(NORMAL_MODE_ON_TIME, NORMAL_MODE_OFF_TIME); // Set back to normal mode blink
            // Serial.println("\n--- Exiting SET_HIGH_MODE: Back to NORMAL_MODE (LED: Off). ---"); // Removed Serial.print
          }
          // Long press stores the current angle as the High limit.
          if (storeButton.isLongPressTriggered()) {
            // Serial.print("Attempting to store HIGH angle: "); // Removed Serial.print
            // Serial.print(servoCtrl.getCurrentAngle());
            // Serial.println(" degrees."); // Removed Serial.print
            posStore.storePosition(servoCtrl.getCurrentAngle(), false); // 'false' indicates setting High
            ledIndicator.startFeedbackBlink(); // Start rapid blink feedback
            currentManualMode = NORMAL_MODE; // Return to normal mode after storing
            // Serial.println("Manual Sub-Mode: NORMAL (LED: Off, after rapid blink)"); // Removed Serial.print
          }
          break;
      }
      break; // End of MANUAL_POT_CONTROL_MODE

    case MIDI_CONTROL_MODE:
      // In this mode, the servo is controlled by incoming MIDI CC#7 messages.
      // The potentiometer has no effect, and the store button is inactive.
      // The servo's position is updated by the handleControlChange() callback,
      // which is called by MIDI.read() above.
      // The LED remains off in this mode.
      // Serial.println("Current Mode: MIDI Control Mode"); // Removed Serial.print
      break; // End of MIDI_CONTROL_MODE
  }

  // Always update the LED (for blinking or solid state, and turning off blink)
  ledIndicator.update();

  // Small delay for smooth operation and readability
  delay(10);
}
