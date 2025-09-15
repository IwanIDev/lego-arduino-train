#include "Switch/SwitchController.hpp"
#include <Arduino.h>

SwitchController::SwitchController() : nextAvailableId(1) {
    // Constructor - switches will be added dynamically
}

/**
* Controls the relay connected to the specified pin.
* @param relayPin The pin number where the relay is connected.
* @param state The desired state of the relay (true for one direction, false for the other).
*/
void SwitchController::controlRelay(int relayPin, bool state) {
    // Control the relay - HIGH typically closes the relay, LOW opens it
    // This assumes active-high relay control
    digitalWrite(relayPin, state ? HIGH : LOW);
}

/**
 * Sets the state of the specified switch.
 * @param switchId The ID of the switch to control.
 * @param state The desired state of the switch (true for one direction, false for the other).
 */
void SwitchController::setSwitchState(int switchId, bool state) {
    auto it = switches.find(switchId);
    if (it != switches.end()) {
        it->second.state = state;
        controlRelay(it->second.relayPin, state);
    }
}

/**
 * Gets the current state of the specified switch.
 * @param switchId The ID of the switch to query.
 * @return The current state of the switch (true for one direction, false for the other).
 */
bool SwitchController::getSwitchState(int switchId) const {
    auto it = switches.find(switchId);
    if (it == switches.end()) return false; // Default state if switch doesn't exist
    return it->second.state;
}

/**
 * Toggles the state of the specified switch.
 * @param switchId The ID of the switch to toggle.
 */
void SwitchController::toggleSwitch(int switchId) {
    auto it = switches.find(switchId);
    if (it != switches.end()) {
        bool newState = !it->second.state;
        it->second.state = newState;
        controlRelay(it->second.relayPin, newState);
    }
}

/**
 * Adds a new switch with the specified ID, relay pin, and initial state.
 * @param id The unique ID for the switch.
 * @param relayPin The pin number where the relay is connected.
 * @param initialState The initial state of the switch (default is false).
 * @return The ID of the added switch.
 */
int SwitchController::addSwitch(int id, int relayPin, bool initialState) {
    // Initialize the relay pin as output
    pinMode(relayPin, OUTPUT);
    
    // Create the switch
    Switch newSwitch;
    newSwitch.id = id;
    newSwitch.relayPin = relayPin;
    newSwitch.state = initialState;
    
    // Set initial relay state
    controlRelay(relayPin, initialState);
    
    // Add to switches map
    switches[id] = newSwitch;
    
    return id;
}

/**
 * Adds a new switch with an automatically generated unique ID.
 * @param relayPin The pin number where the relay is connected.
 * @param initialState The initial state of the switch (default is false).
 * @return The ID of the added switch.
 */
int SwitchController::addSwitch(int relayPin, bool initialState) {
    // Generate a unique ID
    int generatedId = nextAvailableId;
    
    // Make sure the ID doesn't already exist (in case IDs were manually set)
    while (switches.find(generatedId) != switches.end()) {
        generatedId++;
    }
    
    // Update nextAvailableId for future automatic ID generation
    nextAvailableId = generatedId + 1;
    
    // Use the existing addSwitch method with the generated ID
    return addSwitch(generatedId, relayPin, initialState);
}

bool SwitchController::switchExists(int switchId) const {
    return switches.find(switchId) != switches.end();
}
