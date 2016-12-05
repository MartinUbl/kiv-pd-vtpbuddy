#include "general.h"
#include "ConfigurationGenerator.h"

CMConfigurationGenerator::CMConfigurationGenerator() : ConfigurationGenerator(CONFIGURATION_TYPE_CONFIG)
{
    //
}

bool CMConfigurationGenerator::BlockStart(std::string &target, uint16_t vlanId, const char* name)
{
    target = "vlan " + std::to_string(vlanId) + "\n" +
             "name " + std::string(name);

    return true;
}

bool CMConfigurationGenerator::VLANType(std::string &target, uint8_t type)
{
    if (type >= MAX_VLAN_TYPE || type <= 0)
        return false;

    target = "media " + std::string(VLANCFG_mediaType[type]);

    return true;
}

bool CMConfigurationGenerator::ID80210(std::string &target, uint32_t value)
{
    if (value == 0)
        return false;

    target = "said " + std::to_string(value);
    return true;
}

bool CMConfigurationGenerator::AREHops(std::string &target, uint16_t value)
{
    if (value > 13)
        return false;

    target = "are " + std::to_string(value);
    return true;
}

bool CMConfigurationGenerator::STEHops(std::string &target, uint16_t value)
{
    if (value > 13)
        return false;

    target = "ste " + std::to_string(value);
    return true;
}

bool CMConfigurationGenerator::BridgeMode(std::string &target, uint16_t value)
{
    if (value == 0 || value >= MAX_VLAN_BRMODE)
        return false;

    target = "bridge type " + std::string(VLANCFG_bridgeMode[value]);
    return true;
}

bool CMConfigurationGenerator::BridgeNumber(std::string &target, uint16_t value)
{
    if (value >= 16)
        return false;

    target = "bridge " + std::to_string(value);
    return true;
}

bool CMConfigurationGenerator::MTUSize(std::string &target, uint16_t value)
{
    if (value < 1500 || value > 18190)
        return false;

    target = "mtu " + std::to_string(value);
    return true;
}

bool CMConfigurationGenerator::ParentVLAN(std::string &target, uint16_t value)
{
    if (value >= 4096)
        return false;

    target = "parent " + std::to_string(value);
    return true;
}

bool CMConfigurationGenerator::VLANState(std::string &target, uint16_t value)
{
    if (value >= MAX_VLAN_STATE)
        return false;

    target = "state " + std::string(VLANCFG_activeState[value]);
    return true;
}

bool CMConfigurationGenerator::RingNumber(std::string &target, uint16_t value)
{
    if (value == 0 || value >= 4096)
        return false;

    target = "ring " + std::to_string(value);
    return true;
}

bool CMConfigurationGenerator::STPMode(std::string &target, uint16_t value)
{
    if (value >= MAX_VLAN_STP_MODE)
        return false;

    target = "stp type " + std::string(VLANCFG_stpMode[value]);
    return true;
}

bool CMConfigurationGenerator::VLANTrans1(std::string &target, uint16_t value)
{
    if (value >= 4096)
        return false;

    target = "tb-vlan1 " + std::to_string(value);
    return true;
}

bool CMConfigurationGenerator::VLANTrans2(std::string &target, uint16_t value)
{
    if (value >= 4096)
        return false;

    target = "tb-vlan2 " + std::to_string(value);
    return true;
}
