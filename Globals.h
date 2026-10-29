//Globals.h File
#ifndef GLOBALS_H
#define GLOBALS_H

#include <PS4Controller.h>
#include <ESP32Servo.h>

extern int leftY;
extern int rightY;
extern int l2Trigger;
extern int r2Trigger;
extern bool r1Pressed;
extern bool l1Pressed;
extern bool isConnected;

// Set by the Options button. While true, every motor is forced off.
extern volatile bool estop;

#endif
