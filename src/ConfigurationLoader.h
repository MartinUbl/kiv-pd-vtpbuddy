#ifndef VTPBUDDY_CONFIGURATIONLOADER_H
#define VTPBUDDY_CONFIGURATIONLOADER_H

#include "Enums.h"
#include "VTPSettingTypes.h"
#include "ConfigurationGenerator.h"

struct VLANRecord;

/**
 * Base class for all configuration loaders
 */
class ConfigurationLoader
{
    public:
        ConfigurationLoader(ConfigurationGeneratorType type) : m_type(type) { };

        // retrieves configuration loader type
        ConfigurationGeneratorType GetType() { return m_type; };

        // loads single VLAN from supplied input stream
        virtual bool LoadVLAN(std::ifstream& ifs, VLANRecord* target) = 0;

    protected:
        //

    private:
        // type of this loader
        ConfigurationGeneratorType m_type;
};

/**
 * Config-mode configuration loader
 */
class CMConfigurationLoader : public ConfigurationLoader
{
    public:
        CMConfigurationLoader();

        virtual bool LoadVLAN(std::ifstream& ifs, VLANRecord* target);

    protected:
        //

    private:
        //
};

/**
 * VLAN database-mode configuration loader
 */
class VDConfigurationLoader : public ConfigurationLoader
{
    public:
        VDConfigurationLoader();

        virtual bool LoadVLAN(std::ifstream& ifs, VLANRecord* target);

    protected:
        //

    private:
        //
};

#endif
