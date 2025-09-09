
// JogTrainer.ino
// Arduino sketch for CNC Jog Trainer with 2-axis G-code interpreter


#include "StepperModule.h"
#include "LimitSwitch.h"
#include "PiezoBuzzer.h"
#include "ClockModule.h"
#include "GCodeHandler.h"

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


const float stepsPerMM_X = 80.0; // adjust for your hardware
const float stepsPerMM_Y = 80.0; // adjust for your hardware


StepperModule stepperX(X_STEP_PIN, X_DIR_PIN, X_EN_PIN);
StepperModule stepperY(Y_STEP_PIN, Y_DIR_PIN, Y_EN_PIN);
LimitSwitch limitX(X_LIMIT_PIN);
LimitSwitch limitY(Y_LIMIT_PIN);
PiezoBuzzer buzzer(BUZZER_PIN);
ClockModule clock;

float defaultFeedrate = 600.0;

GCodeHandler gcodeHandler(stepperX, stepperY, limitX, limitY, buzzer, clock, stepsPerMM_X, stepsPerMM_Y, defaultFeedrate);


void setup() {
  Serial.begin(115200);
  stepperX.begin();
  stepperY.begin();
  limitX.begin();
  limitY.begin();
  buzzer.begin();
  //clock.begin();
  Serial.println("CNC JogTrainer G-code Ready. Manual: X+/X-/Y+/Y-/LIM?/BUZ/CLOCK. G-code: G0/G1 X Y F");
}


void loop() {
  if (Serial.available() > 0) {
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();
    gcodeHandler.handleLine(cmd);
  }
}
