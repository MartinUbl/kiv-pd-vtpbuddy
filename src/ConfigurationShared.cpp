#include "general.h"
#include "ConfigurationGenerator.h"

uint16_t FindConfActiveStateId(const char* name)
{
    for (uint16_t i = 0; i < MAX_VLAN_STATE; i++)
        if (strcmp(name, VLANCFG_activeState[i]) == 0)
            return i;

    return FIND_IDENTIFIER_ERROR;
}

uint16_t FindConfMediaTypeId(const char* name)
{
    for (uint16_t i = 0; i < MAX_VLAN_TYPE; i++)
        if (strcmp(name, VLANCFG_mediaType[i]) == 0)
            return i;

    return FIND_IDENTIFIER_ERROR;
}

uint16_t FindConfSTPModeId(const char* name)
{
    for (uint16_t i = 0; i < MAX_VLAN_STP_MODE; i++)
        if (strcmp(name, VLANCFG_stpMode[i]) == 0)
            return i;

    return FIND_IDENTIFIER_ERROR;
}

uint16_t FindConfBridgeModeId(const char* name)
{
    for (uint16_t i = 0; i < MAX_VLAN_BRMODE; i++)
        if (strcmp(name, VLANCFG_bridgeMode[i]) == 0)
            return i;

    return FIND_IDENTIFIER_ERROR;
}
