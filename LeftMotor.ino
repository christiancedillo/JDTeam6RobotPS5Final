//LeftMotor.ino File
// TB6612FNG (1), channel A -> Wheel1Motor
const int AIN1 = 14;      // D14
const int AIN2 = 27;      // D27
const int LEFT_PWM = 19;  // PWMA -> D19
const int DEADZONE = 15;

void setupLeftMotor() {
  pinMode(AIN1, OUTPUT);
  pinMode(AIN2, OUTPUT);
  pinMode(LEFT_PWM, OUTPUT);
  digitalWrite(AIN1, LOW);
  digitalWrite(AIN2, LOW);
  analogWrite(LEFT_PWM, 0);
}

void LeftMotorTask(void *pvParameters) {
  for (;;) {
    // IN1 = IN2 = LOW with PWM = 0 -> driver output is high-impedance (no current)
    if (estop || !isConnected || abs(leftY) < DEADZONE) {
      analogWrite(LEFT_PWM, 0);
      digitalWrite(AIN1, LOW);
      digitalWrite(AIN2, LOW);
    } else {
      int speed = constrain(map(abs(leftY), DEADZONE, 127, 0, 255), 0, 255);
      digitalWrite(AIN1, leftY > 0 ? HIGH : LOW);
      digitalWrite(AIN2, leftY > 0 ? LOW : HIGH);
      analogWrite(LEFT_PWM, speed);
    }
    vTaskDelay(10 / portTICK_PERIOD_MS);
  }
}
