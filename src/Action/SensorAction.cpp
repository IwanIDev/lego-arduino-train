#include "Action/SensorAction.hpp"
#include "ActionController.hpp"

/**
 * Default implementation that delegates to the single-parameter execute method.
 * Actions that need to handle DelayedAction specially should override this method.
 * @param controller The train controller to operate on.
 * @param actionController The action controller for managing delayed actions.
 */
void SensorAction::execute(TrainController& controller, ActionController& actionController) {
    // Default behavior: just call the single-parameter version
    execute(controller);
}
