#ifndef NON_BLOCKING_ACTION_H
#define NON_BLOCKING_ACTION_H

#include "../Controllers/TrainController.h"

// Forward declaration to avoid circular dependency
class ActionController;

/**
 * Interface for actions that support non-blocking execution.
 * Actions implementing this interface can be executed over multiple update cycles
 * without blocking the main execution thread.
 */
class NonBlockingAction {
public:
    virtual ~NonBlockingAction() = default;
    
    /**
     * Non-blocking update method that should be called repeatedly until the action completes.
     * @param controller The train controller to operate on.
     * @param actionController The action controller managing this action.
     * @return true if the action should continue, false if it's complete.
     */
    virtual bool update(TrainController& controller, ActionController& actionController) = 0;
    
    /**
     * Checks if the action has finished executing.
     * @return true if the action is finished, false otherwise.
     */
    virtual bool isFinished() const = 0;
    
    /**
     * Resets the action to its initial state.
     */
    virtual void reset() = 0;
};

#endif // NON_BLOCKING_ACTION_H