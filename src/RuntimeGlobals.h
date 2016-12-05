#ifndef VTPBUDDY_RUNTIMEGLOBALS_H
#define VTPBUDDY_RUNTIMEGLOBALS_H

#include "Singleton.h"

class Encapsulation;
class ConfigurationGenerator;

/**
 * Class maintaining runtime storage
 */
class RuntimeGlobalsContainer
{
    friend class Singleton<RuntimeGlobalsContainer>;
    public:

        // initialized runtime globals
        void InitializeRuntime();

        // sets currently used interface MAC address
        void SetSourceMAC(uint8_t* source_mac);
        // retrieves source MAC address
        const uint8_t* GetSourceMAC() const;

        // sets currently used interface IP address
        void SetSourceIP(uint32_t source_ip);
        // retrieves source IP address
        const uint32_t GetSourceIP() const;

        // uses config values to initialize global output encapsulation
        void InitOutputEncapsulation();
        // retrieves stored input encapsulation
        Encapsulation* GetOutputEncapsulation(bool tagged);

        // initializes configuration generator
        void InitConfigurationGenerator();
        // retrieves configuration generator
        ConfigurationGenerator* GetConfigurationGenerator();

    protected:
        // protected singleton constructor
        RuntimeGlobalsContainer();

        // output encapsulation (not tagged)
        Encapsulation* m_outputEncapsulation;
        // output encapsulation (tagged)
        Encapsulation* m_outputTaggedEncapsulation;

        // configuration generator
        ConfigurationGenerator* m_configGenerator;

        // source MAC address (of interface used)
        uint8_t m_sourceAddr[6];
        // source IP address (of interface used)
        uint32_t m_sourceIPAddr;
};

#define sRuntimeGlobals Singleton<RuntimeGlobalsContainer>::getInstance()

#endif
