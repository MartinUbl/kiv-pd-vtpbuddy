#include "general.h"
#include "EthernetStructs.h"
#include "Encapsulation.h"

#include <string.h>
#include <arpa/inet.h>

Dot1QEncapsulation::Dot1QEncapsulation() : Encapsulation(ENCAPS_REAL_DOT1Q)
{
    memset(&m_header, 0, sizeof(Dot1QHeader));
}

void Dot1QEncapsulation::SetParameters(const uint8_t* dst_mac, const uint8_t* src_mac, uint8_t dsap, uint8_t ssap, uint8_t control, const uint8_t* oui, uint16_t protocol_id, uint16_t tpid, uint16_t pcp, uint16_t dei)
{
    memcpy(m_header.dest_mac, dst_mac, MAC_ADDR_LENGTH);
    memcpy(m_header.src_mac, src_mac, MAC_ADDR_LENGTH);

    // 802.1Q tag
    m_header.tpid = htons(tpid);
    m_header.tci = ((pcp & 0x0007) << 13) | ((dei & 0x0001) << 12);

    // LLC
    m_header.dsap = dsap;
    m_header.ssap = ssap;
    m_header.control = control;

    // SNAP extension
    memcpy(m_header.oui, oui, SNAP_OUI_LEN);
    m_header.protocol_id = htons(protocol_id);
}

void Dot1QEncapsulation::AppendEncapsData(uint8_t* target)
{
    memcpy(target, &m_header, sizeof(Dot1QHeader));
}

size_t Dot1QEncapsulation::GetEncapsulationSize()
{
    return sizeof(Dot1QHeader);
}

void Dot1QEncapsulation::SetFrameLength(size_t len)
{
    m_header.frame_length = htons(len);
}

void Dot1QEncapsulation::SetVLANId(uint16_t vlan_id)
{
    m_header.tci |= (vlan_id & 0x0FFF);
}
