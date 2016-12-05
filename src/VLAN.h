#ifndef VTPBUDDY_VLAN_H
#define VTPBUDDY_VLAN_H

// VLAN features
enum VLANFeature
{
    VLAN_FEAT_NONE          = 0x00, // probably not used by Cisco IOS
    VLAN_FEAT_RING_NO       = 0x01,
    VLAN_FEAT_BRIDGE_NO     = 0x02,
    VLAN_FEAT_STP           = 0x03,
    VLAN_FEAT_PARENT        = 0x04,
    VLAN_FEAT_TRANS1        = 0x05,
    VLAN_FEAT_TRANS2        = 0x06,
    VLAN_FEAT_BRIDGE_MODE   = 0x07,
    VLAN_FEAT_ARE_HOPS      = 0x08,
    VLAN_FEAT_STE_HOPS      = 0x09,
    MAX_VLAN_FEATURE        = 0x0A, // unknown, maybe backupcrf has some additional value
};

/**
 * Interval VLAN record structure
 */
struct VLANRecord
{
    // implicit constructor
    VLANRecord() { id = 0; status = 0; type = 0; index80210 = 0; name = ""; };
    // full-parameter constructor
    VLANRecord(uint8_t _type, uint8_t _status, uint16_t _id, uint16_t _mtu, uint32_t _index80210, const char* _name) : type(_type), status(_status), id(_id), mtu(_mtu), index80210(_index80210), name(_name) { };

    // VLAN type
    uint8_t type;
    // VLAN status
    uint8_t status;
    // VLAN ID
    uint16_t id;
    // MTU size
    uint16_t mtu;
    // 802.10 index of VLAN
    uint32_t index80210;
    // VLAN name
    std::string name;
    // VLAN features
    std::map<uint16_t, uint16_t> features;
};

#endif
