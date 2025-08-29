#include "PositionTracker.hpp"
#include <Arduino.h>

PositionTracker::PositionTracker(SensorLocation startPosition) 
    : currentPosition(startPosition), previousPosition(startPosition), currentDirection(TrainDirection::FORWARD), directionManuallySet(false) {
}

void PositionTracker::updatePosition(SensorLocation newPosition) {
    if (newPosition == currentPosition) {
        // Same position triggered again - this often happens when reversing over the same sensor
        // Don't update direction based on sensor movement in this case, trust the current direction
        Serial.print("Same sensor triggered again at position: ");
        Serial.print(static_cast<int>(currentPosition));
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
    Serial.print(static_cast<int>(previousPosition));
    Serial.print(" to ");
    Serial.print(static_cast<int>(currentPosition));
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
    Serial.print(static_cast<int>(currentPosition));
    Serial.print(" to ");
    Serial.print(static_cast<int>(newPosition));
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
    Serial.println(static_cast<int>(segment.location));
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