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
    if (Serial.available() <= 0) return;

    String receivedData = Serial.readStringUntil('\n');
    receivedData.trim();
    if (receivedData.length() == 0) return; // Exit if no data received
    receivedData.toLowerCase(); // Normalize to lowercase for command comparison

    bool commandFound = false;

    if (receivedData.equals("stop")) {
        trainController->setState(STOPPED);
        Serial.println("Train stopped");
        commandFound = true;
    } else if (receivedData.equals("up")) {
        SPEED currentState = trainController->getState();
        setForwardState(currentState);
        Serial.println("Speed increased / Moving forward");
        commandFound = true;
    } else if (receivedData.equals("down")) {
        SPEED currentState = trainController->getState();
        setBackwardState(currentState);
        Serial.println("Speed decreased / Moving backward");
        commandFound = true;
    } else if (receivedData.equals("reverse")) {
        trainController->setReverse(!trainController->getReverse());
        Serial.print("Reverse state set to: ");
        Serial.println(trainController->getReverse() ? "ON" : "OFF");
        commandFound = true;
    } else {
        Serial.println("Unknown command. Use 'stop', 'up', 'down', or 'reverse'.");
    }

    if (commandFound) {
        trainController->printState(); // Print current state after command processing
    }
    Serial.flush(); // Ensure all data is sent before returning
}
