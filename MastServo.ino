//MastServo.ino File
Servo mastServo;
const int SERVO_PIN = 18;

// 5-turn dual mode servo (1800°): 500 µs = 0°, 2500 µs = 1800° (in servo mode)
const int SERVO_MIN_US = 500;
const int SERVO_MAX_US = 2500;
const int SERVO_MAX_DEG = 1800;

// Degrees moved per 15 ms tick while a button is held (4 = ~267°/s).
// Keep this at or below the servo's real top speed; if the commanded target
// runs ahead of the servo, it keeps travelling after you release the button
// (that is the "delay"). Raise it gradually if you want a faster mast.
const int SERVO_STEP_DEG = 4;

// Stop sending pulses after the mast has been idle this long, so the servo
// isn't drawing holding current. Set SERVO_DETACH_WHEN_IDLE to false if the
// mast sags without holding torque.
const bool SERVO_DETACH_WHEN_IDLE = true;
const unsigned long SERVO_IDLE_TIMEOUT_MS = 1000;

int servoAngle = 900;  // mid-range (2.5 turns)
bool servoAttached = false;
unsigned long servoLastActiveMs = 0;

void writeMastServo(int angleDeg) {
  angleDeg = constrain(angleDeg, 0, SERVO_MAX_DEG);
  int pulseUs = map(angleDeg, 0, SERVO_MAX_DEG, SERVO_MIN_US, SERVO_MAX_US);
  mastServo.writeMicroseconds(pulseUs);
}

void attachMastServo() {
  if (servoAttached) return;
  mastServo.setPeriodHertz(50);
  mastServo.attach(SERVO_PIN, SERVO_MIN_US, SERVO_MAX_US);
  writeMastServo(servoAngle);  // resume at the last commanded position
  servoAttached = true;
  servoLastActiveMs = millis();
}

void detachMastServo() {
  if (!servoAttached) return;
  mastServo.detach();
  pinMode(SERVO_PIN, OUTPUT);
  digitalWrite(SERVO_PIN, LOW);  // no pulses on the signal line
  servoAttached = false;
}

void setupMastServo() {
  // LEDC timers are allocated once in setup() in JDTeam6Robot.ino, before any
  // PWM pin is used, so the 50 Hz servo signal and the 1 kHz motor PWM each
  // get their own timer instead of overwriting each other.
  attachMastServo();
}

void MastServoTask(void *pvParameters) {
  for (;;) {
    bool moving = false;

    if (!estop && isConnected) {
      if (r1Pressed) {
        servoAngle += SERVO_STEP_DEG;
        if (servoAngle > SERVO_MAX_DEG) servoAngle = SERVO_MAX_DEG;
        moving = true;
      }
      if (l1Pressed) {
        servoAngle -= SERVO_STEP_DEG;
        if (servoAngle < 0) servoAngle = 0;
        moving = true;
      }
    }

    if (estop) {
      detachMastServo();
    } else if (moving) {
      attachMastServo();  // no-op if already attached
      writeMastServo(servoAngle);
      servoLastActiveMs = millis();
    } else if (SERVO_DETACH_WHEN_IDLE) {
      if (servoAttached && millis() - servoLastActiveMs > SERVO_IDLE_TIMEOUT_MS) {
        detachMastServo();
      }
    } else {
      attachMastServo();  // keep holding; also re-attaches after an e-stop
    }

    vTaskDelay(15 / portTICK_PERIOD_MS);
  }
}
