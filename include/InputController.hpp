#include "TrainController.hpp"

class InputController {
private:
  const int forwardButton;
  const int backwardButton;
  TrainController* trainController;
public:
  InputController(TrainController* controller, int forwardButtonPin, int backwardButtonPin);

  void handleButtonInput(SPEED oldState);
  void handleSerialInput();
};
