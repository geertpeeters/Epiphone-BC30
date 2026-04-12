#include <Servo.h>
Servo testServo;
const int SERVO_PIN = 9; // Your servo pin

void setup() {
  Serial.begin(9600);
  testServo.attach(SERVO_PIN); // Attach with default pulse widths first
  Serial.println("--- Calibrating Wide-Angle Servo ---");
  Serial.println("Enter microsecond pulse widths (e.g., 500 to 2500) to move the servo.");
  Serial.println("Find the minimum and maximum pulse widths where the servo JUST starts/stops moving.");
  Serial.println("Typical starting points: 500us (extreme CCW) to 2500us (extreme CW)");
}

void loop() {
  if (Serial.available()) {
    int pulseUs = Serial.parseInt();
    if (pulseUs >= 0 && pulseUs <= 3000) { // Allow a wider range for testing
      testServo.writeMicroseconds(pulseUs); // This is the function to test specific pulse widths
      Serial.print("Set servo to: ");
      Serial.print(pulseUs);
      Serial.println(" us");
    } else {
      Serial.println("Invalid pulse width. Enter 0-3000.");
    }
  }
}