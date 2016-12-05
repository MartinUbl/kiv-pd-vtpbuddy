#include "general.h"
#include "ConfigurationGenerator.h"

VDConfigurationGenerator::VDConfigurationGenerator() : ConfigurationGenerator(CONFIGURATION_TYPE_VLANDB)
{
    //
}

bool VDConfigurationGenerator::BlockStart(std::string &target, uint16_t vlanId, const char* name)
{
    m_vlanId = vlanId;

    target = "vlan " + std::to_string(m_vlanId) + " name " + name;

    return true;
}

bool VDConfigurationGenerator::VLANType(std::string &target, uint8_t type)
{
    if (type >= MAX_VLAN_TYPE || type <= 0)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " media " + VLANCFG_mediaType[type];

    return true;
}

bool VDConfigurationGenerator::ID80210(std::string &target, uint32_t value)
{
    if (value == 0)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " said " + std::to_string(value);
    return true;
}

bool VDConfigurationGenerator::AREHops(std::string &target, uint16_t value)
{
    if (value > 13)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " are " + std::to_string(value);
    return true;
}

bool VDConfigurationGenerator::STEHops(std::string &target, uint16_t value)
{
    if (value > 13)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " ste " + std::to_string(value);
    return true;
}

bool VDConfigurationGenerator::BridgeMode(std::string &target, uint16_t value)
{
    if (value == 0 || value >= MAX_VLAN_BRMODE)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " bridge type " + VLANCFG_bridgeMode[value];
    return true;
}

bool VDConfigurationGenerator::BridgeNumber(std::string &target, uint16_t value)
{
    if (value >= 16)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " bridge " + std::to_string(value);
    return true;
}

bool VDConfigurationGenerator::MTUSize(std::string &target, uint16_t value)
{
    if (value < 1500 || value > 18190)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " mtu " + std::to_string(value);
    return true;
}

bool VDConfigurationGenerator::ParentVLAN(std::string &target, uint16_t value)
{
    if (value >= 4096)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " parent " + std::to_string(value);
    return true;
}

bool VDConfigurationGenerator::VLANState(std::string &target, uint16_t value)
{
    if (value >= MAX_VLAN_STATE)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " state " + VLANCFG_activeState[value];
    return true;
}

bool VDConfigurationGenerator::RingNumber(std::string &target, uint16_t value)
{
    if (value == 0 || value >= 4096)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " ring " + std::to_string(value);
    return true;
}

bool VDConfigurationGenerator::STPMode(std::string &target, uint16_t value)
{
    if (value >= MAX_VLAN_STP_MODE)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " stp type " + VLANCFG_stpMode[value];
    return true;
}

bool VDConfigurationGenerator::VLANTrans1(std::string &target, uint16_t value)
{
    if (value >= 4096)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " tb-vlan1 " + std::to_string(value);
    return true;
}

bool VDConfigurationGenerator::VLANTrans2(std::string &target, uint16_t value)
{
    if (value >= 4096)
        return false;

    target = "vlan " + std::to_string(m_vlanId) + " tb-vlan2 " + std::to_string(value);
    return true;
}
