// JogTrainer.ino
// Arduino sketch for CNC Jog Trainer

#include "StepperModule.h"
#include "LimitSwitch.h"
#include "PiezoBuzzer.h"



// Pin assignments for Motor 1 (X axis)
const uint8_t X_STEP_PIN = 2;
const uint8_t X_DIR_PIN  = 3;
const uint8_t X_EN_PIN   = 4;
const uint8_t X_LIMIT_PIN = 8;

// Pin assignments for Motor 2 (Y axis)
const uint8_t Y_STEP_PIN = 5;
const uint8_t Y_DIR_PIN  = 6;
const uint8_t Y_EN_PIN   = 7;
const uint8_t Y_LIMIT_PIN = 9;

// Pin assignment for Piezo Buzzer
const uint8_t BUZZER_PIN = 10;

StepperModule stepperX(X_STEP_PIN, X_DIR_PIN, X_EN_PIN);
StepperModule stepperY(Y_STEP_PIN, Y_DIR_PIN, Y_EN_PIN);
LimitSwitch limitX(X_LIMIT_PIN);
LimitSwitch limitY(Y_LIMIT_PIN);
PiezoBuzzer buzzer(BUZZER_PIN);


void setup() {
  Serial.begin(9600);
  stepperX.begin();
  stepperY.begin();
  limitX.begin();
  limitY.begin();
  buzzer.begin();
  Serial.println("JogTrainer Initialized. Send commands: X+/X-/Y+/Y-/BUZ. Limit switches on X and Y.");
}

void loop() {
  // Simple serial command test for debugging
  if (Serial.available()) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    if (cmd == "X+") {
      if (limitX.isPressed()) {
        Serial.println("X+ limit reached! Movement blocked.");
      } else {
        Serial.println("Jog X+");
        stepperX.step(true, 200, 500); // 200 steps forward
      }
    } else if (cmd == "X-") {
      Serial.println("Jog X-");
      stepperX.step(false, 200, 500); // 200 steps backward
    } else if (cmd == "Y+") {
      if (limitY.isPressed()) {
        Serial.println("Y+ limit reached! Movement blocked.");
      } else {
        Serial.println("Jog Y+");
        stepperY.step(true, 200, 500); // 200 steps forward
      }
    } else if (cmd == "Y-") {
      Serial.println("Jog Y-");
      stepperY.step(false, 200, 500); // 200 steps backward
    } else if (cmd == "LIM?") {
      Serial.print("X limit: ");
      Serial.print(limitX.isPressed() ? "PRESSED" : "OPEN");
      Serial.print(" | Y limit: ");
      Serial.println(limitY.isPressed() ? "PRESSED" : "OPEN");
    } else if (cmd == "BUZ") {
      Serial.println("Buzzer test");
      buzzer.beep(200);
    } else {
      Serial.println("Unknown command. Use X+/X-/Y+/Y-/LIM?/BUZ");
    }
  }
}
