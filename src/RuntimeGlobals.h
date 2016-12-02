#ifndef VTPBUDDY_RUNTIMEGLOBALS_H
#define VTPBUDDY_RUNTIMEGLOBALS_H

#include "Singleton.h"

class Encapsulation;

class RuntimeGlobalsContainer
{
    friend class Singleton<RuntimeGlobalsContainer>;
    public:

        // sets currently used interface MAC address
        void SetSourceMAC(uint8_t* source_mac);

        // uses config values to initialize global output encapsulation
        void InitOutputEncapsulation();
        // retrieves stored input encapsulation
        Encapsulation* GetOutputEncapsulation(bool tagged);

    protected:
        RuntimeGlobalsContainer();

        // output encapsulation (not tagged)
        Encapsulation* m_outputEncapsulation;
        // output encapsulation (tagged)
        Encapsulation* m_outputTaggedEncapsulation;

        // source MAC address (of interface used)
        uint8_t m_sourceAddr[6];
};

#define sRuntimeGlobals Singleton<RuntimeGlobalsContainer>::getInstance()

#endif
