#include "InputController.hpp"
#include "TrainController.hpp"
#include <Arduino.h>

InputController::InputController(TrainController& controller, int fastButtonPin, int slowButtonPin)
: trainController(controller), fastButton(fastButtonPin), slowButton(slowButtonPin)
{
  pinMode(fastButton, INPUT_PULLUP);
  pinMode(slowButton, INPUT_PULLUP);
}

void InputController::handleButtonInput() {
  int fastButtonState = digitalRead(fastButton);
  int slowButtonState = digitalRead(slowButton);

  if (fastButtonState == LOW && slowButtonState == LOW) {
    trainController.setState(STOPPED);
    delay(100);
  } else if (fastButtonState == LOW) {  // button pressed
    trainController.setState(FAST);
  } else if (slowButtonState == LOW) {  // button pressed
    trainController.setState(SLOW);
  }
}

void InputController::handleSerialInput() {
  const char STOP_COMMAND[] = "stop";
  const int STOP_COMMAND_LENGTH = sizeof(STOP_COMMAND) - 1;
  const char SLOW_COMMAND[] = "slow";
  const int SLOW_COMMAND_LENGTH = sizeof(STOP_COMMAND) - 1;
  const char FAST_COMMAND[] = "fast";
  const int FAST_COMMAND_LENGTH = sizeof(STOP_COMMAND) - 1;

  String recievedData = "";

  if (Serial.available() <= 0) return;

  recievedData = Serial.readStringUntil('\n');
  recievedData.trim();

  if (recievedData.equals(STOP_COMMAND)) trainController.setState(STOPPED);
  if (recievedData.equals(SLOW_COMMAND)) trainController.setState(SLOW);
  if (recievedData.equals(FAST_COMMAND)) trainController.setState(FAST);
}
