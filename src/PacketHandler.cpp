#include "general.h"
#include "PacketHandler.h"
#include "VTPStructs.h"
#include "DomainMgr.h"
#include "VTPFieldValues.h"
#include "Network.h"

#include <arpa/inet.h>

static uint16_t _ReadU16(uint8_t* buffer, size_t offset)
{
    return ntohs(*((uint16_t*)(&buffer[offset])));
}

static uint32_t _ReadU32(uint8_t* buffer, size_t offset)
{
    return ntohl(*((uint32_t*)(&buffer[offset])));
}

static uint16_t _ReadU16_H(uint8_t* buffer, size_t offset)
{
    return *((uint16_t*)(&buffer[offset]));
}

static uint32_t _ReadU32_H(uint8_t* buffer, size_t offset)
{
    return *((uint32_t*)(&buffer[offset]));
}

template<> void ToNetworkEndianity<SummaryAdvertPacketBody>(SummaryAdvertPacketBody* pkt)
{
    pkt->revision = htonl(pkt->revision);
    pkt->updater_id = htonl(pkt->updater_id);
}

template<> void ToHostEndianity<SummaryAdvertPacketBody>(SummaryAdvertPacketBody* pkt)
{
    pkt->revision = ntohl(pkt->revision);
    pkt->updater_id = ntohl(pkt->updater_id);
}

template<> void ToNetworkEndianity<SubsetAdvertPacketBody>(SubsetAdvertPacketBody* pkt)
{
    pkt->revision = htonl(pkt->revision);
}

template<> void ToHostEndianity<SubsetAdvertPacketBody>(SubsetAdvertPacketBody* pkt)
{
    pkt->revision = ntohl(pkt->revision);
}

template<> void ToNetworkEndianity<AdvertRequestPacketBody>(AdvertRequestPacketBody* pkt)
{
    pkt->start_revision = htonl(pkt->start_revision);
}

template<> void ToHostEndianity<AdvertRequestPacketBody>(AdvertRequestPacketBody* pkt)
{
    pkt->start_revision = ntohl(pkt->start_revision);
}

template<> void ToNetworkEndianity<SubsetVLANInfoBody>(SubsetVLANInfoBody* pkt)
{
    pkt->isl_vlan_id = htons(pkt->isl_vlan_id);
    pkt->mtu_size = htons(pkt->mtu_size);
    pkt->index80210 = htonl(pkt->index80210);
}

template<> void ToHostEndianity<SubsetVLANInfoBody>(SubsetVLANInfoBody* pkt)
{
    pkt->isl_vlan_id = ntohs(pkt->isl_vlan_id);
    pkt->mtu_size = ntohs(pkt->mtu_size);
    pkt->index80210 = ntohl(pkt->index80210);
}

void DispatchReceivedPacket(uint8_t* buffer, size_t len)
{
    uint8_t msg_type, version;

    // TODO: verify buffer lenght every time

    size_t base_offset = 0;

    // TODO: recognize ISL vs. 802.1Q
    //       ISL could be identified by destination multicast address of 01:00:0C:00:00:00

    if (_ReadU16(buffer, SNAP_NOT_TAGGED_OFFSET) == SNAP_DSAP_SSAP_IDENTIFIER)
        base_offset = SNAP_NOT_TAGGED_OFFSET;
    else if (_ReadU16(buffer, SNAP_TAGGED_OFFSET) == SNAP_DSAP_SSAP_IDENTIFIER)
        base_offset = SNAP_TAGGED_OFFSET;
    else
        return;

    if (_ReadU16(buffer, base_offset + VTP_ID_OFFSET) != VTP_IDENTIFIER)
        return;

    VTPHeader* hdr = (VTPHeader*)&buffer[base_offset + VTP_VERSION_OFFSET];

    // find domain
    std::string domName((const char*)hdr->domain_name, (size_t)hdr->domain_len);
    VTPDomain* domain = sDomainMgr->GetDomainByName(domName.c_str());
    if (!domain)
    {
        // for now, create the domain; this is not desired behaviour for future!!!
        //domain = sDomainMgr->CreateDomain(domName.c_str(), nullptr);
        //std::cout << "TEMP: Creating domain " << domName.c_str() << std::endl;
        return;
    }

    // frame length is stored in previous 2 bytes (when using 802.1Q, frame header "tail" is the same as 802.3 Ethernet)
    uint16_t frameLength = _ReadU16(buffer, base_offset - 2);

    // disambiguate using VTP message code
    switch (hdr->code)
    {
        case VTP_MSG_SUMMARY_ADVERT:
        {
            std::cout << "Received VTP summary advert packet" << std::endl;
            SummaryAdvertPacketBody* pkt = (SummaryAdvertPacketBody*)&buffer[base_offset + VTP_VERSION_OFFSET + sizeof(VTPHeader)];
            ToHostEndianity(pkt);

            domain->HandleSummaryAdvert(pkt, hdr->followers);

            break;
        }
        case VTP_MSG_SUBSET_ADVERT:
        {
            std::cout << "Received VTP subset advert packet" << std::endl;
            SubsetAdvertPacketBody* pkt = (SubsetAdvertPacketBody*)&buffer[base_offset + VTP_VERSION_OFFSET + sizeof(VTPHeader)];
            ToHostEndianity(pkt);

            domain->HandleSubsetAdvert(pkt, hdr->sequence_nr, frameLength - VTP_VERSION_OFFSET - sizeof(VTPHeader) - sizeof(SubsetAdvertPacketBody) + 1 /* for one byte "data" placeholder */);

            break;
        }
        case VTP_MSG_ADVERT_REQUEST:
        {
            std::cout << "Received VTP advert request packet" << std::endl;
            AdvertRequestPacketBody* pkt = (AdvertRequestPacketBody*)&buffer[base_offset + VTP_VERSION_OFFSET + sizeof(VTPHeader)];
            ToHostEndianity(pkt);

            domain->HandleAdvertRequest(pkt);

            break;
        }
        case VTP_MSG_JOIN:
            std::cout << "Received VTP join packet" << std::endl;
            // NYI
            break;
        case VTP_MSG_TAKEOVER_REQUEST:
            std::cout << "Received VTP takeover request packet" << std::endl;
            // NYI
            break;
        case VTP_MSG_TAKEOVER_RESPONSE:
            std::cout << "Received VTP takeover response packet" << std::endl;
            // NYI
            break;
        default:
            std::cerr << "Received unknown VTP packet type, not handling." << std::endl;
            break;
    }
}
