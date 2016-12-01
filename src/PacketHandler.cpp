#include "general.h"
#include "PacketHandler.h"

#include <arpa/inet.h>

static uint16_t _ReadU16(uint8_t* buffer, size_t offset)
{
    return ntohs(*((uint16_t*)(&buffer[offset])));
}

void DispatchReceivedPacket(uint8_t* buffer, size_t len)
{
    uint8_t msg_type, version;

    // TODO: verify buffer lenght every time

    size_t base_offset = 0;

    if (_ReadU16(buffer, SNAP_8023_OFFSET) == SNAP_DSAP_SSAP_IDENTIFIER)
        base_offset = SNAP_8023_OFFSET;
    else if (_ReadU16(buffer, SNAP_ETHERNETII_OFFSET) == SNAP_DSAP_SSAP_IDENTIFIER)
        base_offset = SNAP_ETHERNETII_OFFSET;
    else
        return;

    if (_ReadU16(buffer, base_offset + VTP_ID_OFFSET) != VTP_IDENTIFIER)
        return;

    version = buffer[base_offset + VTP_VERSION_OFFSET];
    msg_type = buffer[base_offset + VTP_MSGTYPE_OFFSET];

    // TODO: proper logging
    //std::cout << "Received VTP version " << (int)version << " packet" << std::endl;

    switch (msg_type)
    {
        case VTP_MSG_SUMMARY_ADVERT:
            std::cout << "Received VTP summary advert packet" << std::endl;
            //
            break;
        case VTP_MSG_SUBSET_ADVERT:
            std::cout << "Received VTP subset advert packet" << std::endl;
            //
            break;
        case VTP_MSG_ADVERT_REQUEST:
            std::cout << "Received VTP advert request packet" << std::endl;
            //
            break;
        case VTP_MSG_TAKEOVER_REQUEST:
            std::cout << "Received VTP takeover request packet" << std::endl;
            //
            break;
        case VTP_MSG_TAKEOVER_RESPONSE:
            std::cout << "Received VTP takeover response packet" << std::endl;
            //
            break;
        default:
            std::cerr << "Received unknown VTP packet type, not handling." << std::endl;
            break;
    }
}
