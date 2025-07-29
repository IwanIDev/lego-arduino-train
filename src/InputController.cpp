#include "InputController.hpp"
#include "TrainController.hpp"
#include <Arduino.h>

InputController::InputController(TrainController* controller, int fastButtonPin, int slowButtonPin)
: trainController(controller), fastButton(fastButtonPin), slowButton(slowButtonPin)
{
    pinMode(fastButton, INPUT_PULLUP);
    pinMode(slowButton, INPUT_PULLUP);
}

void InputController::handleButtonInput() {
    int fastButtonState = digitalRead(fastButton);
    int slowButtonState = digitalRead(slowButton);

    if (fastButtonState == LOW && slowButtonState == LOW) {
        trainController->setState(STOPPED);
        delay(100);
    } else if (fastButtonState == LOW) {    // button pressed
        trainController->setState(FAST);
    } else if (slowButtonState == LOW) {    // button pressed
        trainController->setState(SLOW);
    }
}

void InputController::handleSerialInput() {
    const char STOP_COMMAND[] = "stop";
    const int STOP_COMMAND_LENGTH = sizeof(STOP_COMMAND) - 1;
    const char SLOW_COMMAND[] = "slow";
    const int SLOW_COMMAND_LENGTH = sizeof(SLOW_COMMAND) - 1;
    const char FAST_COMMAND[] = "fast";
    const int FAST_COMMAND_LENGTH = sizeof(FAST_COMMAND) - 1;
    const char REVERSE_COMMAND[] = "reverse";
    const int REVERSE_COMMAND_LENGTH = sizeof(REVERSE_COMMAND) - 1;

    const char* commands[] = {STOP_COMMAND, SLOW_COMMAND, FAST_COMMAND, REVERSE_COMMAND};
    const int commandLengths[] = {STOP_COMMAND_LENGTH, SLOW_COMMAND_LENGTH, FAST_COMMAND_LENGTH, REVERSE_COMMAND_LENGTH};
    const SPEED states[] = {STOPPED, SLOW, FAST, REVERSE};

    const int numCommands = sizeof(commands) / sizeof(commands[0]);

    String recievedData = "";

    if (Serial.available() <= 0) return;

    recievedData = Serial.readStringUntil('\n');
    recievedData.trim();

    for (int i = 0; i < numCommands; i++) {
        if (recievedData.length() != commandLengths[i]) continue;
        if (!recievedData.equals(commands[i])) continue;

        trainController->setState(states[i]);
        return; // Exit after processing the command
    }
    Serial.println("Unknown command. Use 'stop', 'slow', 'fast', or 'reverse'.");
    trainController->printState(); // Print current state after command processing
    Serial.flush(); // Ensure all data is sent before returning
}
