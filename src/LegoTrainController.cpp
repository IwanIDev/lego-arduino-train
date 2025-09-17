#include "LegoTrainController.h"

LegoTrainController::LegoTrainController() 
    : lightSensorController(),
      reedSwitchSensorController(),
      positionAwareSensorController(&reedSwitchSensorController, &lightSensorController),
      trainManager(&positionAwareSensorController, &reedSwitchSensorController, &lightSensorController),
      switchController(),
      initialized(false) {
}

LegoTrainController::~LegoTrainController() {
    // Cleanup handled by smart pointers and destructors
}

bool LegoTrainController::begin() {
    Serial.println("Initializing LEGO Train Controller Library");
    
    // Initialize the train manager
    if (trainManager.initialize()) {
        initialized = true;
        Serial.println("LEGO Train Controller initialized successfully");
        return true;
    } else {
        Serial.println("Failed to initialize LEGO Train Controller");
        return false;
    }
}

size_t LegoTrainController::addTrain(const String& hubName, byte motorPort, const SensorLocation& initialPosition) {
    TrainConfig config;
    config.hubName = hubName;
    config.motorPort = motorPort;
    config.initialPosition = initialPosition;
    config.fastButtonPin = -1; // No button by default
    config.slowButtonPin = -1; // No button by default
    
    return trainManager.addTrain(config);
}

size_t LegoTrainController::addTrain(const TrainConfig& config) {
    return trainManager.addTrain(config);
}

bool LegoTrainController::addLightSensor(int pin, int threshold, const SensorLocation& location) {
    LightSensor* sensor = new LightSensor(pin, threshold, location);
    lightSensorController.addSensor(sensor);
    return true;
}

bool LegoTrainController::addReedSwitchSensor(int pin, const SensorLocation& location) {
    // Constructor will use default nullptr for action (position-based system)
    ReedSwitchSensor* sensor = new ReedSwitchSensor(pin, location);
    reedSwitchSensorController.addSensor(sensor);
    return true;
}

int LegoTrainController::addSwitch(int relayPin, int initialPosition) {
    return switchController.addSwitch(relayPin, static_cast<SwitchPosition>(initialPosition));
}

bool LegoTrainController::setTrainSpeed(size_t trainIndex, int speed) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getTrainController()) {
        train->getTrainController()->setSpeed(speed);
        return true;
    }
    return false;
}

bool LegoTrainController::stopTrain(size_t trainIndex) {
    return setTrainSpeed(trainIndex, 0);
}

bool LegoTrainController::reverseTrain(size_t trainIndex) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getTrainController()) {
        train->getTrainController()->reverse();
        return true;
    }
    return false;
}

bool LegoTrainController::operateSwitch(int switchId, int position) {
    return switchController.operateSwitch(switchId, static_cast<SwitchPosition>(position));
}

void LegoTrainController::addTrackSegment(const SensorLocation& location, const SensorLocation& nextForward, const SensorLocation& nextReverse, size_t trainIndex) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getPositionTracker()) {
        TrackSegment segment;
        segment.location = location;
        segment.nextForward = nextForward;
        segment.nextReverse = nextReverse;
        train->getPositionTracker()->addTrackSegment(segment);
    }
}

void LegoTrainController::addStopAction(const SensorLocation& location, size_t trainIndex, int speed) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getPositionTracker()) {
        auto action = std::unique_ptr<SensorAction>(new StopAction(speed));
        train->getPositionTracker()->addForwardAction(location, std::move(action));
    }
}

void LegoTrainController::addReverseAction(const SensorLocation& location, size_t trainIndex, int speed) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getPositionTracker()) {
        auto action = std::unique_ptr<SensorAction>(new ReverseAction(speed));
        train->getPositionTracker()->addForwardAction(location, std::move(action));
    }
}

void LegoTrainController::addSpeedAction(const SensorLocation& location, size_t trainIndex, int speed, int targetSpeed) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getPositionTracker()) {
        auto action = std::unique_ptr<SensorAction>(new SpeedAction(targetSpeed, speed));
        train->getPositionTracker()->addForwardAction(location, std::move(action));
    }
}

void LegoTrainController::addSwitchAction(const SensorLocation& location, size_t trainIndex, int switchId, int position, int speed) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getPositionTracker()) {
        auto action = std::unique_ptr<SensorAction>(new SwitchAction(switchId, static_cast<SwitchPosition>(position), speed, &switchController));
        train->getPositionTracker()->addForwardAction(location, std::move(action));
    }
}

void LegoTrainController::addSequentialAction(const SensorLocation& location, size_t trainIndex, const std::vector<ActionConfig>& actionConfigs) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getPositionTracker()) {
        std::vector<std::unique_ptr<SensorAction>> actions;
        
        for (const auto& config : actionConfigs) {
            switch (config.type) {
                case TrainActionType::STOP:
                    actions.push_back(std::unique_ptr<SensorAction>(new StopAction(config.speed)));
                    break;
                case TrainActionType::REVERSE:
                    actions.push_back(std::unique_ptr<SensorAction>(new ReverseAction(config.speed)));
                    break;
                case TrainActionType::SPEED:
                    actions.push_back(std::unique_ptr<SensorAction>(new SpeedAction(config.targetSpeed, config.speed)));
                    break;
                case TrainActionType::SWITCH:
                    actions.push_back(std::unique_ptr<SensorAction>(new SwitchAction(config.switchId, static_cast<SwitchPosition>(config.switchPosition), config.speed, &switchController)));
                    break;
                case TrainActionType::DELAY:
                    // For delayed actions, we need the nested action
                    if (config.delayedAction) {
                        auto nestedAction = createActionFromConfig(*config.delayedAction);
                        if (nestedAction) {
                            actions.push_back(std::unique_ptr<SensorAction>(new DelayedAction(std::move(nestedAction), config.delayMs)));
                        }
                    }
                    break;
            }
        }
        
        auto sequentialAction = std::unique_ptr<SensorAction>(new SequentialAction(std::move(actions)));
        train->getPositionTracker()->addForwardAction(location, std::move(sequentialAction));
    }
}

std::unique_ptr<SensorAction> LegoTrainController::createActionFromConfig(const ActionConfig& config) {
    switch (config.type) {
        case TrainActionType::STOP:
            return std::unique_ptr<SensorAction>(new StopAction(config.speed));
        case TrainActionType::REVERSE:
            return std::unique_ptr<SensorAction>(new ReverseAction(config.speed));
        case TrainActionType::SPEED:
            return std::unique_ptr<SensorAction>(new SpeedAction(config.targetSpeed, config.speed));
        case TrainActionType::SWITCH:
            return std::unique_ptr<SensorAction>(new SwitchAction(config.switchId, static_cast<SwitchPosition>(config.switchPosition), config.speed, &switchController));
        default:
            return nullptr;
    }
}

int LegoTrainController::getTrainPosition(size_t trainIndex) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getPositionTracker()) {
        return train->getPositionTracker()->getCurrentPosition().getId();
    }
    return -1;
}

int LegoTrainController::getTrainSpeed(size_t trainIndex) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getTrainController()) {
        return train->getTrainController()->getCurrentSpeed();
    }
    return 0;
}

bool LegoTrainController::isTrainConnected(size_t trainIndex) {
    TrainInstance* train = trainManager.getTrain(trainIndex);
    if (train && train->getBluetoothController()) {
        return train->getBluetoothController()->isConnected();
    }
    return false;
}

size_t LegoTrainController::getTrainCount() {
    return trainManager.getTrainCount();
}

void LegoTrainController::update() {
    if (initialized) {
        trainManager.update();
    }
}

void LegoTrainController::enableDebug(bool enable) {
    // Set debug flags for all components
    trainManager.setDebugMode(enable);
}

void LegoTrainController::printStatus() {
    Serial.println("=== LEGO Train Controller Status ===");
    Serial.print("Initialized: ");
    Serial.println(initialized ? "YES" : "NO");
    Serial.print("Train Count: ");
    Serial.println(getTrainCount());
    
    for (size_t i = 0; i < getTrainCount(); i++) {
        Serial.print("Train ");
        Serial.print(i);
        Serial.print(": Position=");
        Serial.print(getTrainPosition(i));
        Serial.print(", Speed=");
        Serial.print(getTrainSpeed(i));
        Serial.print(", Connected=");
        Serial.println(isTrainConnected(i) ? "YES" : "NO");
    }
    Serial.println("====================================");
}