#include "general.h"
#include "RuntimeGlobals.h"
#include "Encapsulation.h"
#include "Enums.h"
#include "VTPFieldValues.h"
#include "Config.h"
#include "EthernetStructs.h"

#include <string.h>

RuntimeGlobalsContainer::RuntimeGlobalsContainer()
{
    m_outputEncapsulation = nullptr;
    m_outputTaggedEncapsulation = nullptr;
}

void RuntimeGlobalsContainer::SetSourceMAC(uint8_t* source_mac)
{
    memcpy(m_sourceAddr, source_mac, MAC_ADDR_LENGTH);
}

void RuntimeGlobalsContainer::InitOutputEncapsulation()
{
    uint64_t encaps = sConfig->GetConfigIntValue(CONF_ENCAPS);

    // ISL encapsulation
    if (encaps == ENCAPS_ISL)
    {
        std::cerr << "ISL encapsulation is not yet supported" << std::endl;
    }
    // 802.1Q encapsulation
    else if (encaps == ENCAPS_DOT1Q)
    {
        // construct non-tagged encaps class (pure 802.3 Ethernet)
        EthernetEncapsulation* eencaps = new EthernetEncapsulation();
        eencaps->SetParameters(VTP_Dest_MAC, m_sourceAddr, SNAP_DSAP_IDENTIFIER, SNAP_SSAP_IDENTIFIER, SNAP_CONTROL_VALUE, OUI_Cisco, VTP_IDENTIFIER);

        m_outputEncapsulation = eencaps;

        // construct tagged encaps class (abuses Ethernet II ethertype field to store TPID)
        Dot1QEncapsulation* qencaps = new Dot1QEncapsulation();
        qencaps->SetParameters(VTP_Dest_MAC, m_sourceAddr, SNAP_DSAP_IDENTIFIER, SNAP_SSAP_IDENTIFIER, SNAP_CONTROL_VALUE, OUI_Cisco, VTP_IDENTIFIER,
                               DOT1Q_VTP_TPID, DOT1Q_VTP_PCP, DOT1Q_VTP_DEI);

        m_outputTaggedEncapsulation = qencaps;
    }
}

Encapsulation* RuntimeGlobalsContainer::GetOutputEncapsulation(bool tagged)
{
    return tagged ? m_outputTaggedEncapsulation : m_outputEncapsulation;
}
