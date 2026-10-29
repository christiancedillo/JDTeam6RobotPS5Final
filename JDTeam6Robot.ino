//JDTeam6Robot.ino file
#include "Globals.h"

int leftY = 0;
int rightY = 0;
int l2Trigger = 0;
int r2Trigger = 0;
bool r1Pressed = false;
bool l1Pressed = false;
bool isConnected = false;
volatile bool estop = false;  // true = all motors cut (toggled by Options button)

const int STBY1 = 32;  // TB6612FNG (1) - wheel motors
const int STBY2 = 33;  // TB6612FNG (2) - gripper motor

// STBY LOW puts the TB6612FNG in standby: all outputs go high-impedance,
// so no current can flow through the motors.
void setDriversEnabled(bool enabled) {
  digitalWrite(STBY1, enabled ? HIGH : LOW);
  digitalWrite(STBY2, enabled ? HIGH : LOW);
}

void setup() {
  Serial.begin(115200);

  // 0. Give ESP32Servo all four LEDC timers BEFORE any PWM pin is set up.
  //    The servo (50 Hz) and the motor analogWrite() pins (1 kHz) need different
  //    timers; if they share one, the servo signal gets corrupted.
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  // 1. Immediately ensure the driver is ASLEEP on boot
  pinMode(STBY1, OUTPUT);
  pinMode(STBY2, OUTPUT);
  setDriversEnabled(false);

  // 2. Initialize all motor pins to 0 (This runs the code in your other tabs)
  setupLeftMotor();
  setupRightMotor();
  setupGripperMotor();
  setupMastServo();

  // 3. Now that the pins are safely at 0, WAKE UP the drivers
  setDriversEnabled(true);

  // 4. Initialize Bluetooth (INSERT MAC ADDRESS)
  ps5.begin("4c:c3:82:d3:00:20"); 
  Serial.println("Ready. Press PS button to connect...");

  // Create 4 independent, concurrent tasks
  xTaskCreatePinnedToCore(LeftMotorTask, "LeftTask", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(RightMotorTask, "RightTask", 2048, NULL, 1, NULL, 1);
  xTaskCreatePinnedToCore(GripperMotorTask, "GripperTask", 2048, NULL, 1, NULL, 1);
xTaskCreatePinnedToCore(MastServoTask, "ServoTask", 2048, NULL, 1, NULL, 0);
}

void loop() {
  static bool lastOptions = false;

  static unsigned long lastPrint = 0;

  isConnected = ps5.isConnected();

  if (isConnected) {
    leftY = ps5.LStickY();
    rightY = ps5.RStickY();
    l2Trigger = ps5.L2Value();
    r2Trigger = ps5.R2Value();
    r1Pressed = ps5.R1();
    l1Pressed = ps5.L1();

    // Options button: toggle emergency stop on each new press (edge-detected)
    bool optionsNow = ps5.Options();
    if (optionsNow && !lastOptions) {
      if (!estop) {
        estop = true;                // tasks see this and zero their outputs
        setDriversEnabled(false);    // and the drivers go to standby
        Serial.println("E-STOP: all motors cut. Press Options again to re-enable.");
      } else {
        setDriversEnabled(true);     // outputs are already zeroed, so this is safe
        estop = false;
        Serial.println("E-STOP released: motors enabled.");
      }
    }
    lastOptions = optionsNow;

    // Telemetry: print battery + stick/trigger values every 100 ms
    if (millis() - lastPrint > 100) {
      lastPrint = millis();
      int battery = ps5.Battery();  // raw level, roughly 0 (empty) to 11 (full)
      int batteryPct = constrain(battery * 100 / 11, 0, 100);
      Serial.printf("Battery=%d (~%d%%)%s | LY=%d RY=%d | L2=%d R2=%d\n",
                    battery, batteryPct, ps5.Charging() ? " charging" : "",
                    leftY, rightY, l2Trigger, r2Trigger);
    }
  } else {
    leftY = 0; rightY = 0;
    l2Trigger = 0; r2Trigger = 0;
    r1Pressed = false; l1Pressed = false;
    lastOptions = false;
  }
  
  delay(10); 
}
