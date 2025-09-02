#ifndef NON_BLOCKING_ACTION_HPP
#define NON_BLOCKING_ACTION_HPP

#include "TrainController.hpp"

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
     * @param actionController The action controller for managing actions.
     * @return true if the action has completed, false if it's still executing.
     */
    virtual bool update(TrainController& controller, ActionController& actionController) = 0;
    
    /**
     * Check if the action has finished executing.
     * @return true if the action has completed, false if it's still executing.
     */
    virtual bool isFinished() const = 0;
    
    /**
     * Reset the action to its initial state so it can be executed again.
     */
    virtual void reset() = 0;
    
    /**
     * Check if the action is a non-blocking action.
     * This method provides a way to identify non-blocking actions without dynamic casting.
     * @return true always, since this is a non-blocking action interface.
     */
    virtual bool isNonBlockingAction() const { return true; }
};

#endif // NON_BLOCKING_ACTION_HPP
