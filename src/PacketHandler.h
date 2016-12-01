#ifndef VTPBUDDY_PACKETHANDLER_H
#define VTPBUDDY_PACKETHANDLER_H

#define SNAP_8023_OFFSET 14
#define SNAP_ETHERNETII_OFFSET 18
#define SNAP_DSAP_SSAP_IDENTIFIER 0xAAAA

// TODO: find proper values
#define VTP_ID_OFFSET 6
#define VTP_VERSION_OFFSET 8
#define VTP_MSGTYPE_OFFSET 9

#define VTP_IDENTIFIER 0x2003

enum VTPMsgType
{
    VTP_MSG_SUMMARY_ADVERT      = 1,
    VTP_MSG_SUBSET_ADVERT       = 2,
    VTP_MSG_ADVERT_REQUEST      = 3,
    VTP_MSG_TAKEOVER_REQUEST    = 5,
    VTP_MSG_TAKEOVER_RESPONSE   = 6
};

void DispatchReceivedPacket(uint8_t* buffer, size_t len);

#endif
