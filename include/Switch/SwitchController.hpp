#ifndef SWITCHCONTROLLER_HPP
#define SWITCHCONTROLLER_HPP

#include <map>

struct Switch {
    int id;
    bool state;
};

class SwitchController {
private:
    std::map<int, Switch> switches;
public:
    SwitchController();
    void setSwitchState(int switchId, bool state);
    bool getSwitchState(int switchId) const;
    void toggleSwitch(int switchId);
    int addSwitch(bool initialState = false);
};

#endif // SWITCHCONTROLLER_HPP