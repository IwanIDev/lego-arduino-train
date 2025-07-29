#include "InputController.hpp"
#include "TrainController.hpp"
#include <Arduino.h>

InputController::InputController(TrainController* controller, int forwardButtonPin, int backwardButtonPin)
: trainController(controller), forwardButton(forwardButtonPin), backwardButton(backwardButtonPin), lastButtonPressTime(0)
{
    pinMode(forwardButton, INPUT_PULLUP);
    pinMode(backwardButton, INPUT_PULLUP);
}

void InputController::setForwardState(SPEED oldState) {
    switch (oldState) {
        case STOPPED:
            trainController->setState(SLOW);
            break;
        case SLOW:
            trainController->setState(FAST);
            break;
        case REVERSE:
            trainController->setState(STOPPED);
            break;
        case FAST:
            break;
        default:
            break;
    }
}

void InputController::setBackwardState(SPEED oldState) {
    switch (oldState) {
        case STOPPED:
            trainController->setState(REVERSE);
            break;
        case SLOW:
            trainController->setState(STOPPED);
            break;
        case FAST:
            trainController->setState(SLOW);
            break;
        case REVERSE:
            break;
        default:
            break;
    }
}

void InputController::handleButtonInput(SPEED oldState) {
    const int forwardButtonState = digitalRead(forwardButton);
    const int backwardButtonState = digitalRead(backwardButton);

    const bool isForwardButtonPressed = (forwardButtonState == LOW);
    const bool isBackwardButtonPressed = (backwardButtonState == LOW);

    unsigned long currentTime = millis();
    if (currentTime - lastButtonPressTime < DEBOUNCE_DELAY) {
        return; // Exit if not enough time has passed since last press
    }

    if (isForwardButtonPressed && isBackwardButtonPressed) {
        trainController->setState(STOPPED);
        lastButtonPressTime = currentTime;
    } else if (isForwardButtonPressed) {
        setForwardState(oldState);
        lastButtonPressTime = currentTime;
    } else if (isBackwardButtonPressed) {
        setBackwardState(oldState);
        lastButtonPressTime = currentTime;
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
