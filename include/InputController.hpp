#ifndef INPUTCONTROLLER_HPP
#define INPUTCONTROLLER_HPP
#include "TrainController.hpp"

class InputController {
private:
  const int forwardButton;
  const int backwardButton;
  TrainController* trainController;

  void setForwardState(SPEED oldState);
  void setBackwardState(SPEED oldState);
  unsigned long lastButtonPressTime;
  const unsigned long DEBOUNCE_DELAY = 250; // 250ms delay between button presses
public:
  InputController(TrainController* controller, int forwardButtonPin, int backwardButtonPin);

  void handleButtonInput(SPEED oldState);
  void handleSerialInput();
};

#endif // INPUTCONTROLLER_HPP
