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
    previousPosition = currentPosition;
    currentPosition = newPosition;
    
    // Determine direction based on position change
    // This is simplified - you'd implement actual track layout logic
    Serial.print("Train moved from ");
    Serial.print(static_cast<int>(previousPosition));
    Serial.print(" to ");
    Serial.println(static_cast<int>(currentPosition));
}

std::vector<std::unique_ptr<SensorAction>> PositionTracker::getActionsForPosition(SensorLocation position, TrainDirection direction) {
    std::vector<std::unique_ptr<SensorAction>> actions;
    
    for (const auto& segment : trackMap) {
        if (segment.location == position) {
            const auto& sourceActions = (direction == TrainDirection::FORWARD) ? segment.forwardActions : segment.reverseActions;
            
            // Clone actions for execution
            for (const auto& action : sourceActions) {
                actions.push_back(action->clone());
            }
            break;
        }
    }
    
    return actions;
}