#ifndef VTPBUDDY_VTPSETTINGTYPES_H
#define VTPBUDDY_VTPSETTINGTYPES_H

// VLAN active state
enum VLANState
{
    VLAN_STATE_ACTIVE       = 0x0000,
    VLAN_STATE_SUSPEND      = 0x0001,
    MAX_VLAN_STATE
};

// Existing VLAN media types
enum VLANType
{
    VLAN_TYPE_NONE          = 0x0000, // not used by Cisco IOS
    VLAN_TYPE_ETHERNET      = 0x0001,
    VLAN_TYPE_FDDI          = 0x0002,
    VLAN_TYPE_TRCRF         = 0x0003, // token ring
    VLAN_TYPE_FDDI_NET      = 0x0004,
    VLAN_TYPE_TRBRF         = 0x0005,
    MAX_VLAN_TYPE
};

// STP modes
enum VLANSTPMode
{
    VLAN_STP_NONE           = 0x0000, // not used by Cisco IOS
    VLAN_STP_IEEE           = 0x0001,
    VLAN_STP_IBM            = 0x0002,
    MAX_VLAN_STP_MODE
};

// Bridge mode
enum VLANBridgeMode
{
    VLAN_BRMODE_NONE        = 0x0000, // not used by Cisco IOS
    VLAN_BRMODE_SRT         = 0x0001,
    VLAN_BRMODE_SRB         = 0x0002,
    MAX_VLAN_BRMODE
};

#endif
