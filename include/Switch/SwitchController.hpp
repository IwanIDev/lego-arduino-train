#ifndef SWITCHCONTROLLER_HPP
#define SWITCHCONTROLLER_HPP

#include <map>

struct Switch {
    int id;
    int relayPin;
    bool state;  // true = one direction, false = other direction
};

class SwitchController {
private:
    std::map<int, Switch> switches;
    int nextAvailableId;
    void controlRelay(int relayPin, bool state);
    
public:
    SwitchController();
    void setSwitchState(int switchId, bool state);
    bool getSwitchState(int switchId) const;
    void toggleSwitch(int switchId);
    int addSwitch(int id, int relayPin, bool initialState = false);
    int addSwitch(int relayPin, bool initialState = false);
    bool switchExists(int switchId) const;
};

#endif // SWITCHCONTROLLER_HPP