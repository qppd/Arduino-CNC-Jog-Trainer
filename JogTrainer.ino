// JogTrainer.ino
// Arduino sketch for CNC Jog Trainer
#include "StepperModule.h"

// Pin assignments for Motor 1 (X axis)
const uint8_t X_STEP_PIN = 2;
const uint8_t X_DIR_PIN  = 3;
const uint8_t X_EN_PIN   = 4;

// Pin assignments for Motor 2 (Y axis)
const uint8_t Y_STEP_PIN = 5;
const uint8_t Y_DIR_PIN  = 6;
const uint8_t Y_EN_PIN   = 7;

StepperModule stepperX(X_STEP_PIN, X_DIR_PIN, X_EN_PIN);
StepperModule stepperY(Y_STEP_PIN, Y_DIR_PIN, Y_EN_PIN);

void setup() {
  Serial.begin(9600);
  stepperX.begin();
  stepperY.begin();
  Serial.println("JogTrainer Initialized. Send commands: X+/X-/Y+/Y-");
}

void loop() {
  // Simple serial command test for debugging
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "X+") {
      Serial.println("Jog X+");
      stepperX.step(true, 200, 500); // 200 steps forward
    } else if (cmd == "X-") {
      Serial.println("Jog X-");
      stepperX.step(false, 200, 500); // 200 steps backward
    } else if (cmd == "Y+") {
      Serial.println("Jog Y+");
      stepperY.step(true, 200, 500); // 200 steps forward
    } else if (cmd == "Y-") {
      Serial.println("Jog Y-");
      stepperY.step(false, 200, 500); // 200 steps backward
    } else {
      Serial.println("Unknown command. Use X+/X-/Y+/Y-");
    }
  }
}
