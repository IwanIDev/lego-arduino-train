#ifndef POSITION_TRACKER_H
#define POSITION_TRACKER_H

#include "../Actions/SensorAction.h"
#include "SensorLocation.h"
#include <vector>
#include <memory>

enum class TrainDirection {
    FORWARD,
    REVERSE
};

struct TrackSegment {
    SensorLocation location;
    std::vector<std::unique_ptr<SensorAction>> forwardActions;
    std::vector<std::unique_ptr<SensorAction>> reverseActions;
    SensorLocation nextForward;
    SensorLocation nextReverse;
};

class PositionTracker {
private:
    SensorLocation currentPosition;
    SensorLocation previousPosition;
    TrainDirection currentDirection;
    bool directionManuallySet; // Flag to track if direction was manually set (e.g., by ReverseAction)
    std::vector<TrackSegment> trackMap;
    
    // Helper method to update direction based on position change
    void updateDirection(SensorLocation newPosition);
    
public:
    PositionTracker(SensorLocation startPosition);
    
    void updatePosition(SensorLocation newPosition);
    SensorLocation getCurrentPosition() const { return currentPosition; }
    SensorLocation getPreviousPosition() const { return previousPosition; }
    TrainDirection getDirection() const { return currentDirection; }
    void setDirection(TrainDirection direction) { 
        currentDirection = direction; 
        directionManuallySet = true; // Mark as manually set
    }
    
    void addTrackSegment(const TrackSegment& segment);
    void addForwardAction(SensorLocation position, std::unique_ptr<SensorAction> action);
    void addReverseAction(SensorLocation position, std::unique_ptr<SensorAction> action);
    std::vector<std::unique_ptr<SensorAction>> getActionsForPosition(SensorLocation position, TrainDirection direction);
    SensorLocation getNextExpectedPosition() const;
    SensorLocation getNextExpectedPosition(TrainDirection direction) const;
    bool canReachPosition(SensorLocation position) const;
};

#endif // POSITION_TRACKER_H