#include "general.h"
#include "RuntimeGlobals.h"
#include "Encapsulation.h"
#include "Enums.h"
#include "VTPFieldValues.h"
#include "Config.h"
#include "EthernetStructs.h"
#include "ConfigurationGenerator.h"
#include "Versioning.h"

#include <string.h>

RuntimeGlobalsContainer::RuntimeGlobalsContainer()
{
    m_outputEncapsulation = nullptr;
    m_outputTaggedEncapsulation = nullptr;
    m_versioningTool = nullptr;
    m_configGenerator = nullptr;
}

void RuntimeGlobalsContainer::InitializeRuntime()
{
    InitOutputEncapsulation();
    InitConfigurationGenerator();
    InitVersioningTool();
}

void RuntimeGlobalsContainer::SetSourceMAC(uint8_t* source_mac)
{
    memcpy(m_sourceAddr, source_mac, MAC_ADDR_LENGTH);
}

const uint8_t* RuntimeGlobalsContainer::GetSourceMAC() const
{
    return m_sourceAddr;
}

void RuntimeGlobalsContainer::SetSourceIP(uint32_t source_ip)
{
    m_sourceIPAddr = source_ip;
}

const uint32_t RuntimeGlobalsContainer::GetSourceIP() const
{
    return m_sourceIPAddr;
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

void RuntimeGlobalsContainer::InitConfigurationGenerator()
{
    int64_t tp = sConfig->GetConfigIntValue(CONF_VLAN_CONF_TYPE);
    if (tp == CONFIGURATION_TYPE_CONFIG)
        m_configGenerator = new CMConfigurationGenerator();
    else if (tp == CONFIGURATION_TYPE_VLANDB)
        m_configGenerator = new VDConfigurationGenerator();
}

ConfigurationGenerator* RuntimeGlobalsContainer::GetConfigurationGenerator()
{
    return m_configGenerator;
}

void RuntimeGlobalsContainer::InitVersioningTool()
{
    int64_t tp = sConfig->GetConfigIntValue(CONF_VERSION_TOOL);
    if (tp == VERSIONING_SVN)
        m_versioningTool = new SVNVersioning();
    else if (tp == VERSIONING_GIT)
        m_versioningTool = new GITVersioning();
}

VersioningBase* RuntimeGlobalsContainer::GetVersioningTool()
{
    return m_versioningTool;
}
