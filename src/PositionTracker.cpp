#include "PositionTracker.hpp"
#include <Arduino.h>

PositionTracker::PositionTracker(SensorLocation startPosition) 
    : currentPosition(startPosition), previousPosition(startPosition), currentDirection(TrainDirection::FORWARD) {
}

void PositionTracker::updatePosition(SensorLocation newPosition) {
    if (newPosition == currentPosition) {
        // No change in position
        return;
    }
    
    // Update direction based on position change and track layout
    updateDirection(newPosition);
    
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
    // Simple direction detection based on sensor sequence
    // This assumes sensors are arranged in a sequence on your track
    int currentIndex = static_cast<int>(currentPosition);
    int newIndex = static_cast<int>(newPosition);
    
    if (newIndex > currentIndex) {
        currentDirection = TrainDirection::FORWARD;
    } else if (newIndex < currentIndex) {
        currentDirection = TrainDirection::REVERSE;
    }
    // If equal, keep current direction
}

std::vector<std::unique_ptr<SensorAction>> PositionTracker::getActionsForPosition(SensorLocation position, TrainDirection direction) {
    std::vector<std::unique_ptr<SensorAction>> actions;
    
    for (const auto& segment : trackMap) {
        if (segment.location == position) {
            const auto& sourceActions = (direction == TrainDirection::FORWARD) ? segment.forwardActions : segment.reverseActions;
            
            // Clone actions for execution
            for (const auto& action : sourceActions) {
                if (!action) continue; // Skip null actions
                actions.push_back(action->clone());
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