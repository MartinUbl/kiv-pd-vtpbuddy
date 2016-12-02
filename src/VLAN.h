#ifndef VTPBUDDY_VLAN_H
#define VTPBUDDY_VLAN_H

// Existing VLAN types
enum VLANType
{
    // TODO
};

/**
 * Interval VLAN record structure
 */
struct VLANRecord
{
    // implicit constructor
    VLANRecord() { id = 0; type = 0; index80210 = 0; name = ""; };
    // full-parameter constructor
    VLANRecord(uint8_t _type, uint16_t _id, uint32_t _index80210, const char* _name) : type(_type), id(_id), index80210(_index80210), name(_name) { };

    // VLAN type
    uint8_t type;
    // VLAN ID
    uint16_t id;
    // 802.10 index of VLAN
    uint32_t index80210;
    // VLAN name
    std::string name;
};

#endif
