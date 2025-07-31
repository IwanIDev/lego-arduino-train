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
    if (trainController->getReverse()) {
        // When in reverse, decrease speed until stopped, then switch to forward
        if (oldState != STOPPED) {
            trainController->decrementSpeed();
            if (trainController->getSpeedMultiplier() <= 0) {
                trainController->setState(STOPPED);
            }
        } else {
            trainController->setReverse(false); // Switch to forward
            trainController->incrementSpeed();  // Start moving forward
        }
    } else {
        // When in forward, increase speed
        trainController->incrementSpeed();
    }
}

void InputController::setBackwardState(SPEED oldState) {
    if (trainController->getReverse()) {
        // When in reverse, increase speed
        trainController->incrementSpeed();
    } else {
        // When in forward, decrease speed until stopped, then switch to reverse
        if (oldState != STOPPED) {
            trainController->decrementSpeed();
            if (trainController->getSpeedMultiplier() <= 0) {
                trainController->setState(STOPPED);
            }
        } else {
            trainController->setReverse(true); // Switch to reverse
            trainController->incrementSpeed(); // Start moving backward
        }
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

    if (isForwardButtonPressed) {
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
    const SPEED states[] = {STOPPED, GO, GO, GO}; // Assuming GO is the state for both FAST and SLOW commands

    const int numCommands = sizeof(commands) / sizeof(commands[0]);

    String recievedData = "";

    if (Serial.available() <= 0) return;

    recievedData = Serial.readStringUntil('\n');
    recievedData.trim();
    if (recievedData.length() == 0) return; // Exit if no data received
    recievedData.toLowerCase(); // Normalize to lowercase for command comparison

    bool commandFound = false;

    for (int i = 0; i < numCommands; i++) {
        if (recievedData.length() != commandLengths[i]) continue;
        if (!recievedData.equals(commands[i])) continue;

        if (i == 3) { // If the command is "reverse"
            trainController->setReverse(!trainController->getReverse()); // Toggle reverse state
            Serial.print("Reverse state set to: ");
            Serial.println(trainController->getReverse() ? "ON" : "OFF");
            commandFound = true;
            break; // Exit after processing the command
        }

        trainController->setState(states[i]);
        commandFound = true;
        break; // Exit after processing the command
    }
    if (!commandFound) {
        Serial.println("Unknown command. Use 'stop', 'slow', 'fast', or 'reverse'.");
    }
    trainController->printState(); // Print current state after command processing
    Serial.flush(); // Ensure all data is sent before returning
}
