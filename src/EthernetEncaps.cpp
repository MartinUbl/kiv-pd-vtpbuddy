#include "general.h"
#include "EthernetStructs.h"
#include "Encapsulation.h"

#include <string.h>
#include <arpa/inet.h>

EthernetEncapsulation::EthernetEncapsulation() : Encapsulation(ENCAPS_REAL_8023)
{
    memset(&m_header, 0, sizeof(EthernetHeader));
}

void EthernetEncapsulation::SetParameters(const uint8_t* dst_mac, const uint8_t* src_mac, uint8_t dsap, uint8_t ssap, uint8_t control, const uint8_t* oui, uint16_t protocol_id)
{
    memcpy(m_header.dest_mac, dst_mac, MAC_ADDR_LENGTH);
    memcpy(m_header.src_mac, src_mac, MAC_ADDR_LENGTH);

    // LLC
    m_header.dsap = dsap;
    m_header.ssap = ssap;
    m_header.control = control;

    // SNAP extension
    memcpy(m_header.oui, oui, SNAP_OUI_LEN);
    m_header.protocol_id = htons(protocol_id);
}

void EthernetEncapsulation::AppendEncapsData(uint8_t* target)
{
    memcpy(target, &m_header, sizeof(EthernetHeader));
}

size_t EthernetEncapsulation::GetEncapsulationSize()
{
    return sizeof(EthernetHeader);
}

void EthernetEncapsulation::SetFrameLength(size_t len)
{
    m_header.frame_length = htons(len);
}
