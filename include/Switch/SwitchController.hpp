#ifndef SWITCHCONTROLLER_HPP
#define SWITCHCONTROLLER_HPP

#include <map>
#include <Arduino.h>

enum class SwitchPosition {
    STRAIGHT,
    DIVERGED
};

struct Switch {
    unsigned int id;
    unsigned int relayPin;
    SwitchPosition state;  // STRAIGHT = one direction, DIVERGED = other direction
};

class SwitchController {
private:
    std::map<int, Switch> switches;
    int nextAvailableId;
    void controlRelay(int relayPin, SwitchPosition state);
    
public:
    SwitchController();
    void setSwitchState(int switchId, SwitchPosition state);
    SwitchPosition getSwitchState(int switchId) const;
    void toggleSwitch(int switchId);
    int addSwitch(int id, int relayPin, SwitchPosition initialState = SwitchPosition::STRAIGHT);
    int addSwitch(int relayPin, SwitchPosition initialState = SwitchPosition::STRAIGHT);
    bool switchExists(int switchId) const;
};

#endif // SWITCHCONTROLLER_HPP