#ifndef VTPBUDDY_VLAN_H
#define VTPBUDDY_VLAN_H

enum VLANType
{
    //
};

struct VLANRecord
{
    VLANRecord() { id = 0; type = 0; index80210 = 0; name = ""; };
    VLANRecord(uint8_t _type, uint16_t _id, uint32_t _index80210, const char* _name) : type(_type), id(_id), index80210(_index80210), name(_name) { };

    uint8_t type;
    uint16_t id;
    uint32_t index80210;
    std::string name;
};

#endif
