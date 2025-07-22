#include "TrainController.hpp"

class InputController {
private:
  const int fastButton;
  const int slowButton;
  TrainController* trainController;
public:
  InputController(TrainController* controller, int fastButtonPin, int slowButtonPin);

  void handleButtonInput();
  void handleSerialInput();
};
