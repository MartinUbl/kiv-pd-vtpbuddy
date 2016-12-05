#ifndef VTPBUDDY_CONFIGURATIONGENERATOR_H
#define VTPBUDDY_CONFIGURATIONGENERATOR_H

#include "Enums.h"
#include "VTPSettingTypes.h"

// Following arrays are just for value -> name translation for configs

// VLAN active state
static const char* VLANCFG_activeState[MAX_VLAN_STATE] = {
    "active",       // VLAN_STATE_ACTIVE
    "suspend"       // VLAN_STATE_SUSPEND
};

// VLAN media types
static const char* VLANCFG_mediaType[MAX_VLAN_TYPE] = {
    "none",         // VLAN_TYPE_NONE
    "ethernet",     // VLAN_TYPE_ETHERNET
    "fddi",         // VLAN_TYPE_FDDI
    "trcrf",        // VLAN_TYPE_TRCRF
    "fd-net",       // VLAN_TYPE_FDDI_NET
    "trbrf"         // VLAN_TYPE_TRBRF
};

// VLAN STP modes
static const char* VLANCFG_stpMode[MAX_VLAN_STP_MODE] =  {
    "none",         // VLAN_STP_NONE
    "ieee",         // VLAN_STP_IEEE
    "ibm"           // VLAN_STP_IBM
};

// VLAN bridge modes
static const char* VLANCFG_bridgeMode[MAX_VLAN_BRMODE] = {
    "none",         // VLAN_BRMODE_NONE
    "srt",          // VLAN_BRMODE_SRT
    "srb"           // VLAN_BRMODE_SRB
};

/**
 * Base class for all configuration generators
 */
class ConfigurationGenerator
{
    public:
        // constructor retaining type
        ConfigurationGenerator(ConfigurationGeneratorType type) : m_type(type) {};

        // generates VLAN block start
        virtual bool BlockStart(std::string &target, uint16_t vlanId, const char* name) = 0;
        // generates VLAN type configuration
        virtual bool VLANType(std::string &target, uint8_t type) = 0;
        // generates 802.10 ID block
        virtual bool ID80210(std::string &target, uint32_t value) = 0;
        // generates ARE hops block
        virtual bool AREHops(std::string &target, uint16_t value) = 0;
        // generates STE hops block
        virtual bool STEHops(std::string &target, uint16_t value) = 0;
        // generates bridge mode block
        virtual bool BridgeMode(std::string &target, uint16_t value) = 0;
        // generates bridge number block
        virtual bool BridgeNumber(std::string &target, uint16_t value) = 0;
        // generates MTU setting block
        virtual bool MTUSize(std::string &target, uint16_t value) = 0;
        // generates parent VLAN block
        virtual bool ParentVLAN(std::string &target, uint16_t value) = 0;
        // generates VLAN state block
        virtual bool VLANState(std::string &target, uint16_t value) = 0;
        // generates ring number block
        virtual bool RingNumber(std::string &target, uint16_t value) = 0;
        // generates STP setting block
        virtual bool STPMode(std::string &target, uint16_t value) = 0;
        // generates translational VLAN 1 block
        virtual bool VLANTrans1(std::string &target, uint16_t value) = 0;
        // generates translational VLAN 2 block
        virtual bool VLANTrans2(std::string &target, uint16_t value) = 0;

    protected:
        //

    private:
        // generator type
        ConfigurationGeneratorType m_type;
};

/**
 * Configure mode configuration generator
 */
class CMConfigurationGenerator : public ConfigurationGenerator
{
    public:
        CMConfigurationGenerator();

        virtual bool BlockStart(std::string &target, uint16_t vlanId, const char* name);
        virtual bool VLANType(std::string &target, uint8_t type);
        virtual bool ID80210(std::string &target, uint32_t value);
        virtual bool AREHops(std::string &target, uint16_t value);
        virtual bool STEHops(std::string &target, uint16_t value);
        virtual bool BridgeMode(std::string &target, uint16_t value);
        virtual bool BridgeNumber(std::string &target, uint16_t value);
        virtual bool MTUSize(std::string &target, uint16_t value);
        virtual bool ParentVLAN(std::string &target, uint16_t value);
        virtual bool VLANState(std::string &target, uint16_t value);
        virtual bool RingNumber(std::string &target, uint16_t value);
        virtual bool STPMode(std::string &target, uint16_t value);
        virtual bool VLANTrans1(std::string &target, uint16_t value);
        virtual bool VLANTrans2(std::string &target, uint16_t value);

    protected:
        //

    private:
        //
};

/**
 * Vlan Database mode configuration generator
 */
class VDConfigurationGenerator : public ConfigurationGenerator
{
    public:
        VDConfigurationGenerator();
        
        virtual bool BlockStart(std::string &target, uint16_t vlanId, const char* name);
        virtual bool VLANType(std::string &target, uint8_t type);
        virtual bool ID80210(std::string &target, uint32_t value);
        virtual bool AREHops(std::string &target, uint16_t value);
        virtual bool STEHops(std::string &target, uint16_t value);
        virtual bool BridgeMode(std::string &target, uint16_t value);
        virtual bool BridgeNumber(std::string &target, uint16_t value);
        virtual bool MTUSize(std::string &target, uint16_t value);
        virtual bool ParentVLAN(std::string &target, uint16_t value);
        virtual bool VLANState(std::string &target, uint16_t value);
        virtual bool RingNumber(std::string &target, uint16_t value);
        virtual bool STPMode(std::string &target, uint16_t value);
        virtual bool VLANTrans1(std::string &target, uint16_t value);
        virtual bool VLANTrans2(std::string &target, uint16_t value);

    protected:
        //

    private:
        // vlan ID stored for later command generation
        uint16_t m_vlanId;
};

#endif
