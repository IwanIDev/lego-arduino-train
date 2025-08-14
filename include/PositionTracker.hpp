#ifndef POSITIONTRACKER_HPP
#define POSITIONTRACKER_HPP

#include "Action/SensorAction.hpp"
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
    std::vector<TrackSegment> trackMap;
    
    // Helper method to update direction based on position change
    void updateDirection(SensorLocation newPosition);
    
public:
    PositionTracker(SensorLocation startPosition);
    
    void updatePosition(SensorLocation newPosition);
    SensorLocation getCurrentPosition() const { return currentPosition; }
    SensorLocation getPreviousPosition() const { return previousPosition; }
    TrainDirection getDirection() const { return currentDirection; }
    void setDirection(TrainDirection direction) { currentDirection = direction; }
    
    void addTrackSegment(const TrackSegment& segment);
    std::vector<std::unique_ptr<SensorAction>> getActionsForPosition(SensorLocation position, TrainDirection direction);
    SensorLocation getNextExpectedPosition() const;
};

#endif // POSITIONTRACKER_HPP