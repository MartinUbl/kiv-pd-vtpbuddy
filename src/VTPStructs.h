#ifndef VTPBUDDY_VTPSTRUCTS_H
#define VTPBUDDY_VTPSTRUCTS_H

#include "EthernetStructs.h"

#define SNAP_NOT_TAGGED_OFFSET 14
#define SNAP_TAGGED_OFFSET 18

#define VTP_ID_OFFSET 6
#define VTP_VERSION_OFFSET 8
#define VTP_MSGTYPE_OFFSET 9

// VTP message codes
enum VTPMsgCode
{
    VTP_MSG_SUMMARY_ADVERT      = 1,
    VTP_MSG_SUBSET_ADVERT       = 2,
    VTP_MSG_ADVERT_REQUEST      = 3,
    VTP_MSG_JOIN                = 4,    // TODO: verify
    VTP_MSG_TAKEOVER_REQUEST    = 5,
    VTP_MSG_TAKEOVER_RESPONSE   = 6
};

// domain name field length
#define MAX_VTP_DOMAIN_LENGTH 32

// length of timestamp field (summary advert)
#define VTP_TIMESTAMP_LENGTH 12
// length of MD5 hash (summary advert)
#define VTP_MD5_LENGTH 16

// for exact match, we need to eliminate additional padding
#pragma pack(push)
#pragma pack(1)

// since GCC6
//#pragma scalar_storage_order big-endian

struct VTPHeader
{
    uint8_t version;                            // VTP version (1, 2 or 3)
    uint8_t code;                               // packet type (see enum VTPMsgCode)
    union
    {
        uint8_t followers;                      // summary advert only - is this packet followed by subset advert packet?
        uint8_t sequence_nr;                    // subset advert only  - sequence number of packet, starting with 1
        uint8_t reserved;                       // advertisement request only - reserved, undefined value
    };
    uint8_t domain_len;                         // length of domain name (real)
    uint8_t domain_name[MAX_VTP_DOMAIN_LENGTH]; // domain name, padded with zeros to 32 bytes
};

struct SummaryAdvertPacketBody
{
    uint32_t revision;                              // revision of VLAN database
    uint32_t updater_id;                            // updater identity
    uint8_t update_timestamp[VTP_TIMESTAMP_LENGTH]; // timestamp of update
    uint8_t md5_digest[VTP_MD5_LENGTH];             // MD5 hash of VTP password (if set)
};

struct SubsetAdvertPacketBody
{
    uint32_t revision;                          // revision number
    uint8_t data;                               // data indicator; used just as base address member
};

struct AdvertRequestPacketBody
{
    uint32_t start_revision;                    // requested advertised revision
};

struct SubsetVLANInfoBody
{
    uint8_t length;                             // this portion length
    uint8_t status;                             // VLAN status
    uint8_t type;                               // VLAN type (see VLANType enum)
    uint8_t name_length;                        // length of vlan name
    uint16_t isl_vlan_id;                       // ISL VLAN ID
    uint16_t mtu_size;                          // MTU size
    uint32_t index80210;                        // 802.10 VLAN index
    uint8_t data;                               // data indicator; used just as base address member
};

// since GCC6
//#pragma scalar_storage_order default

#pragma pack(pop)

#endif
