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
const long LONG_PRESS_DURATION = 2000; // 3 seconds for a long press
const int POT_READINGS_TO_AVERAGE = 10; // Number of readings for potentiometer smoothing

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


// --- 1. PotentiometerReader Class ---
class PotentiometerReader {
public:
  PotentiometerReader(int pin, int numReadings)
    : _pin(pin), _numReadings(numReadings), _readIndex(0), _total(0), _averagePotValue(0) {
    for (int i = 0; i < _numReadings; i++) {
      _readings[i] = 0;
    }
  }
  void update() {
    int rawPotValue = analogRead(_pin);
    _total = _total - _readings[_readIndex];
    _readings[_readIndex] = rawPotValue;
    _total = _total + _readings[_readIndex];
    _readIndex = (_readIndex + 1) % _numReadings;
    _averagePotValue = _total / _numReadings;
  }
  int getSmoothedValue() const { return _averagePotValue; }
private:
  int _pin;
  int _numReadings;
  int _readings[10];
  int _readIndex;
  long _total;
  int _averagePotValue;
  friend void setup();
};

// --- 2. ServoController Class ---
class ServoController {
public:
  ServoController(int pin, int minAngle, int maxAngle)
    : _pin(pin), _minAngle(minAngle), _maxAngle(maxAngle) {}
  void attach(int initialPotValue, int potMin, int potMax) {
    _servo.attach(_pin);
    writeMappedAngle(initialPotValue, potMin, potMax);
  }
  void writeMappedAngle(int potValue, int potMin, int potMax) {
    int targetAngle = map(potValue, potMin, potMax, _minAngle, _maxAngle);
    targetAngle = constrain(targetAngle, 0, 180);
    _servo.write(targetAngle);
    _currentAngle = targetAngle;
  }
  void writeAngle(int angle) {
    _servo.write(constrain(angle, 0, 180));
    _currentAngle = constrain(angle, 0, 180);
  }
  int getCurrentAngle() const { return _currentAngle; }
private:
  Servo _servo;
  int _pin;
  int _minAngle;
  int _maxAngle;
  int _currentAngle;
};

// --- 3. PushButton Class (Used only for the store button now) ---
class PushButton {
public:
  PushButton(int pin, long longPressDuration)
    : _pin(pin), _longPressDuration(longPressDuration),
      _lastButtonState(HIGH), _buttonPressStartTime(0), _longPressActive(false),
      _shortPressTriggered(false)
  {
    pinMode(_pin, INPUT_PULLUP);
  }
  void update() {
    int currentButtonState = digitalRead(_pin);
    if (currentButtonState != _lastButtonState) {
      _lastButtonStateChangeTime = millis();
    }
    if ((millis() - _lastButtonStateChangeTime) > _debounceDelay) {
      if (currentButtonState != _debouncedButtonState) {
        _debouncedButtonState = currentButtonState;
        if (_debouncedButtonState == LOW) {
          _buttonPressStartTime = millis();
          _longPressActive = false;
          _shortPressTriggered = false;
        } else {
          unsigned long pressDuration = millis() - _buttonPressStartTime;
          if (pressDuration >= _debounceDelay && pressDuration < _longPressDuration) {
            _shortPressTriggered = true;
          }
          _longPressActive = false;
        }
      }
    }
    _lastButtonState = currentButtonState;
    if (_debouncedButtonState == LOW &&
        (millis() - _buttonPressStartTime >= _longPressDuration) &&
        !_longPressActive) {
      _longPressActive = true;
    }
  }
  bool isLongPressTriggered() {
    if (_longPressActive) {
      _longPressActive = false;
      return true;
    }
    return false;
  }
  bool isShortPressTriggered() {
    if (_shortPressTriggered) {
      _shortPressTriggered = false;
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
  const long _debounceDelay = 50;
};

// --- 4. PositionStore Class ---
class PositionStore {
public:
  PositionStore(int magicAddr, int lowAddr, int highAddr)
    : _magicAddr(magicAddr), _lowAddr(lowAddr), _highAddr(highAddr),
      _lowPosition(0), _highPosition(1) {}
  void loadPositions() {
    int storedMagicNumber = 0;
    EEPROM.get(_magicAddr, storedMagicNumber);
    if (storedMagicNumber == MAGIC_NUMBER) {
      EEPROM.get(_lowAddr, _lowPosition);
      EEPROM.get(_highAddr, _highPosition);
      Serial.print("Loaded from EEPROM: Low = ");
      Serial.print(_lowPosition);
      Serial.print(", High = ");
      Serial.println(_highPosition);
    } else {
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
    if (_lowPosition == DEFAULT_LOW_POSITION && _highPosition == DEFAULT_HIGH_POSITION) {
      _lowPosition = currentAngle;
      _highPosition = currentAngle;
      Serial.println("Initial user values set (Low and High now same).");
      updated = true;
    }
    else {
        if (isLowSet) {
            _lowPosition = currentAngle;
            Serial.println("New Low value stored.");
            updated = true;
        } else {
            _highPosition = currentAngle;
            Serial.println("New High value stored.");
            updated = true;
        }
    }
    if (_lowPosition > _highPosition) {
        int temp = _lowPosition;
        _lowPosition = _highPosition;
        _highPosition = temp;
        Serial.println("Auto-corrected: Low and High values swapped to maintain order.");
        updated = true;
    }
    if (updated) {
      EEPROM.put(_lowAddr, _lowPosition);
      EEPROM.put(_highAddr, _highPosition);
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
  static const int MAGIC_NUMBER = 0xABCD;
  static const int DEFAULT_LOW_POSITION = 0;
  static const int DEFAULT_HIGH_POSITION = 1;
};

// --- FORMAL DEFINITION FOR STATIC CONST MEMBER ---
const int PositionStore::MAGIC_NUMBER;

// --- 5. LedIndicator Class ---
class LedIndicator {
public:
  LedIndicator(int pin)
    : _ledPin(pin), _blinkingActive(false), _onDuration(0), _offDuration(0),
      _lastToggleTime(0), _ledState(LOW), _feedbackBlinkActive(false), _feedbackStartTime(0) {
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, LOW); // Ensure off initially
  }

  // Turns the LED completely off
  void turnOff() {
    _blinkingActive = false;
    _feedbackBlinkActive = false; // Stop any ongoing feedback blink
    digitalWrite(_ledPin, LOW);
  }

  // Sets a continuous blinking pattern for modes
  void setBlinkPattern(long onDuration, long offDuration) {
    _blinkingActive = true;
    _feedbackBlinkActive = false; // Stop any ongoing feedback blink
    _onDuration = onDuration;
    _offDuration = offDuration;
    _lastToggleTime = millis(); // Reset timer for new pattern
    _ledState = HIGH;           // Start new pattern with LED ON
    digitalWrite(_ledPin, HIGH);
  }

  // Starts the rapid blinking for store feedback
  void startFeedbackBlink() {
    _blinkingActive = false;    // Stop any mode blinking
    _feedbackBlinkActive = true;
    _feedbackStartTime = millis(); // Record start time of the feedback
    _lastToggleTime = millis();    // Initialize for immediate first toggle
    _ledState = HIGH;              // Force first state ON
    digitalWrite(_ledPin, HIGH);
  }

  // Call this repeatedly in loop() to update the LED's state
  void update() {
    if (_feedbackBlinkActive) {
      // Manage the rapid flashing for feedback duration
      if (millis() - _feedbackStartTime >= STORE_FEEDBACK_DURATION) {
        _feedbackBlinkActive = false; // Stop feedback blink
        digitalWrite(_ledPin, LOW);   // Ensure LED is OFF after feedback
      } else {
        // Continue rapid flashing
        if (millis() - _lastToggleTime >= STORE_FEEDBACK_TOGGLE_TIME) {
          _ledState = !_ledState; // Toggle LED state
          digitalWrite(_ledPin, _ledState ? HIGH : LOW);
          _lastToggleTime = millis();
        }
      }
    } else if (_blinkingActive) {
      // Manage the current mode's blink pattern
      if (_ledState == HIGH) { // If currently ON
        if (millis() - _lastToggleTime >= _onDuration) {
          digitalWrite(_ledPin, LOW);
          _ledState = LOW;
          _lastToggleTime = millis();
        }
      } else { // If currently OFF
        if (millis() - _lastToggleTime >= _offDuration) {
          digitalWrite(_ledPin, HIGH);
          _ledState = HIGH;
          _lastToggleTime = millis();
        }
      }
    } else {
      // Ensure LED is off if no blinking is active (e.g., in NORMAL_MODE)
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

PotentiometerReader potReader(POT_PIN, POT_READINGS_TO_AVERAGE);
ServoController servoCtrl(SERVO_PIN, 0, 180);
PushButton storeButton(BUTTON_PIN, LONG_PRESS_DURATION);
PositionStore posStore(EEPROM_MAGIC_ADDR, EEPROM_LOW_ADDR, EEPROM_HIGH_ADDR);
LedIndicator ledIndicator(LED_PIN);

// --- Variables for Toggle Switch Debouncing (Manual for the toggle switch) ---
int lastModeSwitchState = HIGH;      // Last read state of the toggle switch
unsigned long lastModeSwitchChangeTime = 0; // Last time the mode switch state changed
const long MODE_SWITCH_DEBOUNCE_DELAY = 50; // Debounce time for the toggle switch


// --- System Mode Enumerations (Existing "manual" mode) ---
enum SystemMode {
  NORMAL_MODE,   // Servo controlled by pot, LED OFF
  SET_LOW_MODE,  // LED blinking (short ON, long OFF), long press stores low
  SET_HIGH_MODE, // LED blinking (short ON, shorter OFF), long press stores high
};
SystemMode currentManualMode = NORMAL_MODE; // Renamed to clarify it's for the manual control branch


// --- Main Control Mode Enumeration (New) ---
enum MainControlMode {
  MANUAL_POT_CONTROL_MODE, // The original potentiometer-controlled mode
  ALTERNATIVE_HOLD_MODE    // The new mode where servo holds position
};
MainControlMode currentMainMode = MANUAL_POT_CONTROL_MODE; // Start in your original manual mode


// --- Arduino Setup Function ---
void setup() {
  Serial.begin(9600);
  Serial.println("--- System Booting ---");

  // Setup the mode switch pin
  pinMode(MODE_SWITCH_PIN, INPUT_PULLUP);
  lastModeSwitchState = digitalRead(MODE_SWITCH_PIN); // Read initial state

  // Load stored positions from EEPROM (handles initialization if blank)
  posStore.loadPositions();

  // Initialize potentiometer reader to get an initial smoothed value
  for(int i = 0; i < potReader._numReadings; ++i) {
       potReader.update();
       delay(1); // Small delay for stable analog sampling
  }
  int initialPotValue = potReader.getSmoothedValue();

  // Attach servo and set initial position based on current pot value
  servoCtrl.attach(initialPotValue, 0, 1023);

  // Initialize LED to off for Normal Mode (initial state)
  ledIndicator.turnOff();

  Serial.println("--- System Ready ---");
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
    // Only act if the debounced state is different from the current main mode
    // Assuming LOW on switch means ALTERNATIVE_HOLD_MODE
    // and HIGH (due to PULLUP) means MANUAL_POT_CONTROL_MODE
    if (currentModeSwitchState == LOW && currentMainMode == MANUAL_POT_CONTROL_MODE) {
      currentMainMode = ALTERNATIVE_HOLD_MODE;
      ledIndicator.turnOff(); // Ensure LED is off in alternative mode
      Serial.println("Switched to ALTERNATIVE_HOLD_MODE. Servo will hold its current position.");
      // Capture the current servo angle to hold it
      servoCtrl.writeAngle(servoCtrl.getCurrentAngle());
    } else if (currentModeSwitchState == HIGH && currentMainMode == ALTERNATIVE_HOLD_MODE) { // Switch is back to HIGH
      currentMainMode = MANUAL_POT_CONTROL_MODE;
      ledIndicator.turnOff(); // Ensure LED is off when returning to normal pot mode
      Serial.println("Switched to MANUAL_POT_CONTROL_MODE.");
      // Re-sync servo to current potentiometer position immediately
      // THIS IS THE CRUCIAL LINE REVERTED TO ORIGINAL BEHAVIOR
      int smoothedPotValue = potReader.getSmoothedValue();
      servoCtrl.writeMappedAngle(smoothedPotValue, 0, 1023); // Reverted: Pot always maps 0-1023 to 0-180
      Serial.println("Servo is now controlled by Potentiometer (Manual Mode).");
    }
  }
  lastModeSwitchState = currentModeSwitchState; // Update last state for next iteration


  // --- Main Control Mode Logic ---
  switch (currentMainMode) {
    case MANUAL_POT_CONTROL_MODE:
      // --- Your Original Loop Logic (Unchanged as requested) ---
      // Always update potentiometer and servo in any mode
      potReader.update();
      int smoothedPotValue = potReader.getSmoothedValue();
      // THIS IS THE CRUCIAL LINE REVERTED TO ORIGINAL BEHAVIOR
      servoCtrl.writeMappedAngle(smoothedPotValue, 0, 1023); // Reverted: Pot always maps 0-1023 to 0-180

      // --- Mode State Machine (Your original SystemMode logic) ---
      switch (currentManualMode) {
        case NORMAL_MODE:
          // In Normal Mode, short press transitions to SET_LOW_MODE
          if (storeButton.isShortPressTriggered()) {
            currentManualMode = SET_LOW_MODE;
            ledIndicator.setBlinkPattern(LOW_MODE_ON_TIME, LOW_MODE_OFF_TIME); // Low Mode blink pattern
            Serial.println("Manual Sub-Mode: SET_LOW (LED: Short ON, Long OFF)");
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
            Serial.println("Manual Sub-Mode: SET_HIGH (LED: Short ON, Shorter OFF)");
          }
          // Long press stores the current angle as Low
          if (storeButton.isLongPressTriggered()) {
            Serial.println("Storing Low position...");
            // The stored position still captures the servo's current angle,
            // which is mapped from the full pot range at this point.
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
            Serial.println("Manual Sub-Mode: NORMAL (LED: Off)");
          }
          // Long press stores the current angle as High
          if (storeButton.isLongPressTriggered()) {
            Serial.println("Storing High position...");
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