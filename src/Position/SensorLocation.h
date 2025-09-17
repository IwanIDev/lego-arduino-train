#ifndef SENSOR_LOCATION_H
#define SENSOR_LOCATION_H

#include <string>
#include <memory>

/**
 * @brief Represents a sensor location with flexible positioning
 * 
 * This class replaces the previous SensorLocation enum to allow for
 * custom positioning instead of hardcoded location names.
 */
class SensorLocation {
private:
    std::string name;
    int id;
    
public:
    /**
     * @brief Construct a new Sensor Location with a name and optional ID
     * 
     * @param locationName Name identifier for this location
     * @param locationId Unique numeric identifier
     */
    SensorLocation(const std::string& locationName, int locationId)
        : name(locationName), id(locationId) {}
    
    /**
     * @brief Default constructor creating an unknown location
     */
    SensorLocation() : name("UNKNOWN"), id(-1) {}
    
    /**
     * @brief Get the name of this location
     * @return const std::string& Location name
     */
    const std::string& getName() const { return name; }
    
    /**
     * @brief Get the ID of this location
     * @return int Location ID
     */
    int getId() const { return id; }
    
    /**
     * @brief Set the ID of this location
     * @param newId New ID value
     */
    void setId(int newId) { id = newId; }
    
    /**
     * @brief Check if this is an unknown/undefined location
     * @return true if location is unknown
     */
    bool isUnknown() const { return name == "UNKNOWN"; }
    
    // Comparison operators
    bool operator==(const SensorLocation& other) const {
        return name == other.name && id == other.id;
    }
    
    bool operator!=(const SensorLocation& other) const {
        return !(*this == other);
    }
    
    bool operator<(const SensorLocation& other) const {
        if (name != other.name) return name < other.name;
        return id < other.id;
    }
    
    // Static factory methods for common locations (maintains backward compatibility)
    static SensorLocation createWestStation() { return SensorLocation("WEST_STATION", 0); }
    static SensorLocation createWestTunnel() { return SensorLocation("WEST_TUNNEL", 1); }
    static SensorLocation createEastStation() { return SensorLocation("EAST_STATION", 2); }
    static SensorLocation createEastTunnel() { return SensorLocation("EAST_TUNNEL", 3); }
    static SensorLocation createUnknown() { return SensorLocation("UNKNOWN", -1); }
};

#endif // SENSOR_LOCATION_H
