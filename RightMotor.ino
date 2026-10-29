//RightMotor.ino file
// TB6612FNG (1), channel B -> Wheel2Motor
const int BIN1 = 26;       // D26
const int BIN2 = 25;       // D25
const int RIGHT_PWM = 13;  // PWMB -> D13
const int R_DEADZONE = 15;

void setupRightMotor() {
  pinMode(BIN1, OUTPUT);
  pinMode(BIN2, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);
  digitalWrite(BIN1, LOW);
  digitalWrite(BIN2, LOW);
  analogWrite(RIGHT_PWM, 0);
}

void RightMotorTask(void *pvParameters) {
  for (;;) {
    // IN1 = IN2 = LOW with PWM = 0 -> driver output is high-impedance (no current)
    if (estop || !isConnected || abs(rightY) < R_DEADZONE) {
      analogWrite(RIGHT_PWM, 0);
      digitalWrite(BIN1, LOW);
      digitalWrite(BIN2, LOW);
    } else {
      int speed = constrain(map(abs(rightY), R_DEADZONE, 127, 0, 255), 0, 255);
      digitalWrite(BIN1, rightY > 0 ? HIGH : LOW);
      digitalWrite(BIN2, rightY > 0 ? LOW : HIGH);
      analogWrite(RIGHT_PWM, speed);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
