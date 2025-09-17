#include "PositionTracker.h"
#include <Arduino.h>

PositionTracker::PositionTracker(SensorLocation startPosition) 
    : currentPosition(startPosition), previousPosition(startPosition), currentDirection(TrainDirection::FORWARD), directionManuallySet(false) {
}

void PositionTracker::updatePosition(SensorLocation newPosition) {
    if (newPosition == currentPosition) {
        // Same position triggered again - this often happens when reversing over the same sensor
        // Don't update direction based on sensor movement in this case, trust the current direction
        Serial.print("Same sensor triggered again at position: ");
        Serial.print(currentPosition.getName().c_str());
        Serial.print(", keeping current direction: ");
        Serial.println(currentDirection == TrainDirection::FORWARD ? "FORWARD" : "REVERSE");
        return;
    }
    
    // Only update direction based on track layout if it wasn't manually set by ReverseAction
    if (!directionManuallySet) {
        // Update direction based on position change and track layout
        updateDirection(newPosition);
    } else {
        // Direction was manually set, so reset the flag for next time
        directionManuallySet = false;
        Serial.println("PositionTracker: Keeping manually set direction, not updating based on track layout");
    }
    
    previousPosition = currentPosition;
    currentPosition = newPosition;
    
    Serial.print("Train moved from ");
    Serial.print(previousPosition.getName().c_str());
    Serial.print(" to ");
    Serial.print(currentPosition.getName().c_str());
    Serial.print(" direction: ");
    Serial.println(currentDirection == TrainDirection::FORWARD ? "FORWARD" : "REVERSE");
}

void PositionTracker::updateDirection(SensorLocation newPosition) {
    // Check direction based on track layout
    for (const auto& segment : trackMap) {
        if (segment.location != currentPosition) continue; // Skip segments not matching current position
        
        // Check if movement matches forward direction
        if (segment.nextForward == newPosition) {
            currentDirection = TrainDirection::FORWARD;
            return;
        }
        // Check if movement matches reverse direction 
        else if (segment.nextReverse == newPosition) {
            currentDirection = TrainDirection::REVERSE;
            return;
        }
        break;
    }
    // Unexpected movement, log to serial
    Serial.print("Unexpected movement from ");
    Serial.print(currentPosition.getName().c_str());
    Serial.print(" to ");
    Serial.print(newPosition.getName().c_str());
    Serial.println(" detected.");
}

std::vector<std::unique_ptr<SensorAction>> PositionTracker::getActionsForPosition(SensorLocation position, TrainDirection direction) {
    std::vector<std::unique_ptr<SensorAction>> actions;
    
    for (const auto& segment : trackMap) {
        if (segment.location == position) {
            const auto& sourceActions = (direction == TrainDirection::FORWARD) ? segment.forwardActions : segment.reverseActions;
            
            // Clone actions for execution
            for (const auto& action : sourceActions) {
                if (!action) continue; // Skip null actions
                auto clonedAction = action->clone();
                if (!clonedAction) continue; // Skip cloning failures
                actions.push_back(std::move(clonedAction));
            }
            break;
        }
    }
    
    return actions;
}

void PositionTracker::addTrackSegment(const TrackSegment& segment) {
    trackMap.push_back(TrackSegment{
        segment.location,
        {},  // Will be populated with cloned actions
        {},  // Will be populated with cloned actions
        segment.nextForward,
        segment.nextReverse
    });
    
    // Clone the actions to avoid ownership issues
    auto& lastSegment = trackMap.back();
    for (const auto& action : segment.forwardActions) {
        lastSegment.forwardActions.push_back(action->clone());
    }
    for (const auto& action : segment.reverseActions) {
        lastSegment.reverseActions.push_back(action->clone());
    }
    
    Serial.print("Added track segment for position: ");
    Serial.println(segment.location.getName().c_str());
}

SensorLocation PositionTracker::getNextExpectedPosition() const {
    for (const auto& segment : trackMap) {
        if (segment.location == currentPosition) {
            return (currentDirection == TrainDirection::FORWARD) ? segment.nextForward : segment.nextReverse;
        }
    }
    return currentPosition; // Default to current if no track map found
}

SensorLocation PositionTracker::getNextExpectedPosition(TrainDirection direction) const {
    for (const auto& segment : trackMap) {
        if (segment.location == currentPosition) {
            return (direction == TrainDirection::FORWARD) ? segment.nextForward : segment.nextReverse;
        }
    }
    return currentPosition; // Default to current if no track map found
}

bool PositionTracker::canReachPosition(SensorLocation position) const {
    // If the train is already at the triggered position, it should respond
    // (this happens when reversing direction over the same sensor)
    if (currentPosition == position) {
        return true;
    }
    
    // Otherwise, check if the position is reachable from current position
    for (const auto& segment : trackMap) {
        if (segment.location == currentPosition) {
            return (segment.nextForward == position) || (segment.nextReverse == position);
        }
    }
    return false; // Cannot reach if no track segment found
}

void PositionTracker::addForwardAction(SensorLocation position, std::unique_ptr<SensorAction> action) {
    for (auto& segment : trackMap) {
        if (segment.location == position) {
            segment.forwardActions.push_back(std::move(action));
            Serial.print("Added forward action for position: ");
            Serial.println(position.getName().c_str());
            return;
        }
    }
    // If no existing segment found, create a new one
    TrackSegment newSegment;
    newSegment.location = position;
    newSegment.forwardActions.push_back(std::move(action));
    newSegment.nextForward = position; // Default to same position
    newSegment.nextReverse = position; // Default to same position
    trackMap.push_back(std::move(newSegment));
    
    Serial.print("Created new track segment with forward action for position: ");
    Serial.println(position.getName().c_str());
}

void PositionTracker::addReverseAction(SensorLocation position, std::unique_ptr<SensorAction> action) {
    for (auto& segment : trackMap) {
        if (segment.location == position) {
            segment.reverseActions.push_back(std::move(action));
            Serial.print("Added reverse action for position: ");
            Serial.println(position.getName().c_str());
            return;
        }
    }
    // If no existing segment found, create a new one
    TrackSegment newSegment;
    newSegment.location = position;
    newSegment.reverseActions.push_back(std::move(action));
    newSegment.nextForward = position; // Default to same position
    newSegment.nextReverse = position; // Default to same position
    trackMap.push_back(std::move(newSegment));
    
    Serial.print("Created new track segment with reverse action for position: ");
    Serial.println(position.getName().c_str());
}