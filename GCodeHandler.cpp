#include "GCodeHandler.h"
#include "StepperModule.h"
#include "LimitSwitch.h"
#include "PiezoBuzzer.h"
#include "ClockModule.h"
#include <math.h>

GCodeHandler::GCodeHandler(StepperModule& sx, StepperModule& sy, LimitSwitch& lx, LimitSwitch& ly, PiezoBuzzer& buz, ClockModule& clk,
                           float stepsPerMM_X, float stepsPerMM_Y, float defaultFeedrate)
    : stepperX(sx), stepperY(sy), limitX(lx), limitY(ly), buzzer(buz), clock(clk),
      stepsPerMM_X(stepsPerMM_X), stepsPerMM_Y(stepsPerMM_Y), defaultFeedrate(defaultFeedrate),
      posX_steps(0), posY_steps(0) {}

long GCodeHandler::getXSteps() const { return posX_steps; }
long GCodeHandler::getYSteps() const { return posY_steps; }

void GCodeHandler::jogCommand(const String& cmd) {
    if (cmd == "X+") {
        if (limitX.isPressed()) {
            Serial.println("X+ limit reached! Movement blocked.");
        } else {
            Serial.println("Jog X+");
            stepperX.step(true, 200, 500);
            posX_steps += 200;
        }
    } else if (cmd == "X-") {
        Serial.println("Jog X-");
        stepperX.step(false, 200, 500);
        posX_steps -= 200;
    } else if (cmd == "Y+") {
        if (limitY.isPressed()) {
            Serial.println("Y+ limit reached! Movement blocked.");
        } else {
            Serial.println("Jog Y+");
            stepperY.step(true, 200, 500);
            posY_steps += 200;
        }
    } else if (cmd == "Y-") {
        Serial.println("Jog Y-");
        stepperY.step(false, 200, 500);
        posY_steps -= 200;
    } else if (cmd == "LIM?") {
        Serial.print("X limit: ");
        Serial.print(limitX.isPressed() ? "PRESSED" : "OPEN");
        Serial.print(" | Y limit: ");
        Serial.println(limitY.isPressed() ? "PRESSED" : "OPEN");
    } else if (cmd == "BUZ") {
        Serial.println("Buzzer test");
        buzzer.beep(200);
    } else if (cmd == "CLOCK") {
        Serial.print("Current time: ");
        clock.printNow(Serial);
        Serial.println();
    } else {
        Serial.println("Unknown command. Use X+/X-/Y+/Y-/LIM?/BUZ/CLOCK");
    }
}

float GCodeHandler::parseGcodeValue(const String& line, char code, float fallback) {
    int idx = line.indexOf(code);
    if (idx == -1) return fallback;
    int start = idx + 1;
    int end = start;
    while (end < line.length() && (isDigit(line[end]) || line[end] == '.' || line[end] == '-')) end++;
    return line.substring(start, end).toFloat();
}

void GCodeHandler::moveTo(long targetX, long targetY, float feedrate_mm_min) {
    long dx = targetX - posX_steps;
    long dy = targetY - posY_steps;
    long abs_dx = abs(dx);
    long abs_dy = abs(dy);
    int dirX = (dx >= 0);
    int dirY = (dy >= 0);

    // Check limit switches before moving in each direction
    if ((dx > 0 && limitX.isPressed()) || (dx < 0 && limitX.isPressed())) {
        Serial.println("X limit reached! Move blocked.");
        return;
    }
    if ((dy > 0 && limitY.isPressed()) || (dy < 0 && limitY.isPressed())) {
        Serial.println("Y limit reached! Move blocked.");
        return;
    }

    float dist_mm = sqrt(sq(dx / stepsPerMM_X) + sq(dy / stepsPerMM_Y));
    if (dist_mm < 0.001) {
        Serial.println("No move");
        Serial.println("ok");
        return;
    }

    float feedrate = (feedrate_mm_min > 0) ? feedrate_mm_min : defaultFeedrate;
    float move_time_min = dist_mm / feedrate;
    float move_time_ms = move_time_min * 60000.0;
    long total_steps = max(abs_dx, abs_dy);
    if (total_steps == 0) total_steps = 1;
    float step_delay = move_time_ms / total_steps;
    if (step_delay < 1) step_delay = 1;

    long x = posX_steps;
    long y = posY_steps;
    long sx = (dx > 0) ? 1 : -1;
    long sy = (dy > 0) ? 1 : -1;
    long err = abs_dx - abs_dy;

    for (long i = 0; i < total_steps; i++) {
        long e2 = 2 * err;
        if (e2 > -abs_dy) {
            err -= abs_dy;
            x += sx;
            if (sx > 0 && limitX.isPressed()) break;
            if (sx < 0 && limitX.isPressed()) break;
            stepperX.step(sx > 0, 1, 0);
        }
        if (e2 < abs_dx) {
            err += abs_dx;
            y += sy;
            if (sy > 0 && limitY.isPressed()) break;
            if (sy < 0 && limitY.isPressed()) break;
            stepperY.step(sy > 0, 1, 0);
        }
        delayMicroseconds((unsigned long)(step_delay * 1000));
    }
    posX_steps = x;
    posY_steps = y;
    Serial.println("ok");
}

void GCodeHandler::handleGcode(const String& line) {
    String l = line;
    l.trim();
    l.toUpperCase();
    if (l.startsWith("G0") || l.startsWith("G1")) {
        float x_mm = parseGcodeValue(l, 'X', posX_steps / stepsPerMM_X);
        float y_mm = parseGcodeValue(l, 'Y', posY_steps / stepsPerMM_Y);
        float f = parseGcodeValue(l, 'F', defaultFeedrate);
        long targetX = l.indexOf('X') >= 0 ? lround(x_mm * stepsPerMM_X) : posX_steps;
        long targetY = l.indexOf('Y') >= 0 ? lround(y_mm * stepsPerMM_Y) : posY_steps;
        moveTo(targetX, targetY, f);
    } else {
        Serial.println("Unknown or unsupported G-code");
        Serial.println("ok");
    }
}

void GCodeHandler::handleLine(const String& line) {
    if (line.length() == 0) return;
    if (line == "X+" || line == "X-" || line == "Y+" || line == "Y-" || line == "LIM?" || line == "BUZ" || line == "CLOCK") {
        jogCommand(line);
    } else if (line.startsWith("G")) {
        handleGcode(line);
    } else {
        Serial.println("Unknown command. Use X+/X-/Y+/Y-/LIM?/BUZ/CLOCK or G-code");
    }
}
