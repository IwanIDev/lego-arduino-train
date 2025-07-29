#include "TrainController.hpp"

class InputController {
private:
  const int forwardButton;
  const int backwardButton;
  TrainController* trainController;

  void setForwardState(SPEED oldState);
  void setBackwardState(SPEED oldState);
public:
  InputController(TrainController* controller, int forwardButtonPin, int backwardButtonPin);

  void handleButtonInput(SPEED oldState);
  void handleSerialInput();
};
