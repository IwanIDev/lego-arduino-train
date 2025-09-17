# LEGO Train Controller

An Arduino library for controlling LEGO Powered Up trains with automated layouts, sensor-based automation, and intelligent routing.

## Features

- **Multi-train management** - Control multiple LEGO Powered Up trains simultaneously
- **Sensor integration** - Support for light sensors and reed switch sensors
- **Track switch control** - Automated control of track switches via relays
- **Position tracking** - Advanced position tracking with collision avoidance
- **Action sequencing** - Complex automated behaviors with sequential actions

## Hardware Requirements

- Arduino ESP32 or compatible microcontroller
- One or more LEGO Powered Up train hubs
- Optional: Reed switch sensors for position detection
- Optional: Relay modules for track switch control

## Installation

### Arduino IDE Library Manager
1. Open Arduino IDE
2. Go to **Sketch > Include Library > Manage Libraries**
3. Search for "LEGO Train Controller"
4. Click "Install"

### Manual Installation
1. Download the library as a ZIP file from GitHub
2. In Arduino IDE: **Sketch > Include Library > Add .ZIP Library**
3. Select the downloaded ZIP file

### Dependencies
This library requires:
- [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino)
- [Legoino](https://github.com/corneliusmunz/legoino)

## Quick Start

```cpp
#include <LegoTrainController.h>

// Create controller instance
LegoTrainController trainController;

void setup() {
  Serial.begin(115200);
  
  // Initialize the library
  trainController.begin();
  
  // Create sensor locations
  SensorLocation westStation("WEST_STATION", 1);
  SensorLocation eastStation("EAST_STATION", 2);
  SensorLocation bridgeEntrance("BRIDGE_ENTRANCE", 3);
  
  // Add a train with starting position
  size_t trainIndex = trainController.addTrain("MyTrain", PoweredUpHubPort::B, westStation);
  
  // Add sensors using locations
  trainController.addReedSwitchSensor(D9, westStation);
  trainController.addReedSwitchSensor(D10, eastStation);
  trainController.addReedSwitchSensor(D11, bridgeEntrance);
  
  // Start the train
  trainController.setTrainSpeed(trainIndex, 30);
}

void loop() {
  trainController.update();
}
```

## Examples

The library includes several examples:

- **BasicTrainControl** - Simple train control with manual commands
- **MultiTrainSystem** - Advanced multi-train automation with sensors and switches

## API Reference

### Core Functions

#### `begin()`
Initialize the train controller system. Call this in `setup()`.

```cpp
bool success = trainController.begin();
```

#### `update()`
Main update loop. Call this in `loop()`.

```cpp
trainController.update();
```

### Train Management

#### `addTrain(hubName, motorPort, initialPosition)`
Add a train to the system.

```cpp
// Create a starting location
SensorLocation depot("DEPOT", 1);
size_t trainIndex = trainController.addTrain("MyTrain", PoweredUpHubPort::B, depot);
```

#### `setTrainSpeed(trainIndex, speed)`
Set train speed (-100 to 100, negative for reverse).

```cpp
trainController.setTrainSpeed(0, 50); // 50% forward
trainController.setTrainSpeed(0, -30); // 30% reverse
```

#### `stopTrain(trainIndex)`
Stop a specific train.

```cpp
trainController.stopTrain(0);
```

### Sensor Management

#### `addReedSwitchSensor(pin, location)`
Add a reed switch sensor at a track position.

```cpp
// Create sensor location
SensorLocation station("MAIN_STATION", 10);
trainController.addReedSwitchSensor(D9, station);
```

#### `addLightSensor(pin, threshold, location)`
Add a light sensor with detection threshold.

```cpp
// Create sensor locations for tunnels and bridges
SensorLocation westTunnel("WEST_TUNNEL", 15);
SensorLocation tunnelEntry("TUNNEL_ENTRY", 20);

trainController.addLightSensor(A0, 50, westTunnel);
trainController.addLightSensor(A1, 30, tunnelEntry);
```

### Track Layout

#### `addTrackSegment(location, nextForward, nextReverse, trainIndex)`
Define track topology for position tracking.

```cpp
// Create custom locations for track layout
SensorLocation westStation("WEST_STATION", 1);
SensorLocation eastStation("EAST_STATION", 2);
SensorLocation westTunnel("WEST_TUNNEL", 3);

trainController.addTrackSegment(
  westStation,
  eastStation,
  westTunnel,
  0 // train index
);
```

### Automated Actions

#### `addStopAction(location, trainIndex, speed)`
Make train stop when reaching a sensor location.

```cpp
// Create station location
SensorLocation mainStation("MAIN_STATION", 5);
trainController.addStopAction(mainStation, 0, 0);
```

#### `addSequentialAction(location, trainIndex, actionConfigs)`
Execute a sequence of actions at a sensor location.

```cpp
// Create turnaround location
SensorLocation turnaround("TURNAROUND_POINT", 25);

std::vector<ActionConfig> actions;
actions.push_back(ActionConfig(TrainActionType::STOP));
actions.push_back(ActionConfig(TrainActionType::REVERSE));
trainController.addSequentialAction(turnaround, 0, actions);
```

## Advanced Features

### Sensor Locations
Create custom sensor locations for flexible track layouts:

```cpp
// Create locations with descriptive names and unique IDs
SensorLocation bridgeEntry("BRIDGE_ENTRANCE", 100);
SensorLocation tunnelExit("TUNNEL_EXIT", 101);
SensorLocation maintenanceBay("MAINTENANCE_BAY", 200);

// Use them with sensors
trainController.addReedSwitchSensor(D8, bridgeEntry);
trainController.addLightSensor(A2, 40, tunnelExit);

// Use them in track layout
trainController.addTrackSegment(bridgeEntry, tunnelExit, maintenanceBay, 0);
```

### Multiple Trains
The library supports multiple trains with collision avoidance:

```cpp
// Create starting positions for each train
SensorLocation westStation("WEST_STATION", 10);
SensorLocation eastStation("EAST_STATION", 20);
SensorLocation depot("MAINTENANCE_DEPOT", 200);

size_t train1 = trainController.addTrain("Train1", PoweredUpHubPort::B, westStation);
size_t train2 = trainController.addTrain("Train2", PoweredUpHubPort::B, eastStation);
size_t train3 = trainController.addTrain("Train3", PoweredUpHubPort::B, depot);
```

### Switch Control
Control track switches with relay modules:

```cpp
int switchId = trainController.addSwitch(D5, SwitchPositions::STRAIGHT);
trainController.operateSwitch(switchId, SwitchPositions::DIVERGED);
```

### Position Tracking
Advanced position tracking with automatic direction detection:

```cpp
int position = trainController.getTrainPosition(0);
bool connected = trainController.isTrainConnected(0);
```

## Troubleshooting

### Debug Output

Enable debug output to troubleshoot issues:

```cpp
trainController.enableDebug(true);
trainController.printStatus();
```

## Contributing

Contributions are welcome, especially if you can port this over to new platforms.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- [Legoino Library](https://github.com/corneliusmunz/legoino) for LEGO Powered Up communication
- [NimBLE-Arduino](https://github.com/h2zero/NimBLE-Arduino) for Bluetooth Low Energy support