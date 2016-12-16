#ifndef VTPBUDDY_CONFIG_H
#define VTPBUDDY_CONFIG_H

#include "Singleton.h"

// config filename; this is constant
#define SYS_CONFIG_FILENAME "vtpbuddy.cfg"

// path to config file in system configuration directory
#define SYS_CONFIG_PATH "/etc/" SYS_CONFIG_FILENAME

// present config options
enum ConfigOpts
{
    CONF_MODE       = 0,
    CONF_INTERFACE  = 1,
    CONF_ENCAPS     = 2,
    CONF_VTP_VERSION    = 3,
    CONF_PRIM_DOMAIN    = 4,
    CONF_PRIM_PASSWORD  = 5,
    CONF_VLAN_CONF_TYPE = 6,
    MAX_CONF_OPT
};

// possible types of config values
enum ConfigOptType
{
    CONF_OPT_TYPE_INT       = 0,
    CONF_OPT_TYPE_STRING    = 1,
    CONF_OPT_TYPE_ENUM      = 2,    // is later considered as CONF_OPT_TYPE_INT
    MAX_CONF_OPT_TYPE
};

// stored option structure
struct ConfigStoredOpt
{
    // option name (identifier in config file)
    std::string name;
    // data type of config option
    ConfigOptType type;
    // option internal identifier
    ConfigOpts opt;

    // string value in case of string value
    std::string strValue;
    // integer value in case of integer or enum value
    int64_t intValue;

    // is filled from config? true = set from config, false = defaults
    bool filled;
};

/**
 * Configuration manager, loads app config, manages retrieving options from outside
 */
class ConfigMgr
{
    friend class Singleton<ConfigMgr>;
    public:
        // loads config from implicit paths
        bool LoadConfig();

        // retrieves integer value from config
        int64_t GetConfigIntValue(ConfigOpts opt);
        // retrieves string value from config
        const char* GetConfigStringValue(ConfigOpts opt);

    protected:
        // protected singleton constructor
        ConfigMgr();

        // initializes config defaults, pushes options to map so they can be overwriteen later from file
        void InitDefaults();

        // parses config option line, may return false when the line should not be considered
        bool _ParseConfigOpt(const std::string line, std::string &name, std::string &value);
        // finds already stored config option
        ConfigStoredOpt* _FindConfigOpt(const char* name);

        // Finds existing config file and returns full path
        std::string _FindConfigFile();

        // initializes integer-type config value
        void _InitConfigIntValue(ConfigOpts opt, const char* name, int value);
        // initializes enum-type config value
        void _InitConfigEnumValue(ConfigOpts opt, const char* name, int value);
        // sets integer-type value to config
        void _SetConfigIntValue(ConfigOpts opt, int value, bool initialized = false);
        // initializes string-type config value
        void _InitConfigStringValue(ConfigOpts opt, const char* name, const char* value);
        // sets string-type config value to config
        void _SetConfigStringValue(ConfigOpts opt, const char* value, bool initialized = false);

        // parses enumerator value; returns -1 on error (not found)
        int64_t _ParseEnumValue(ConfigOpts opt, std::string str);

    private:
        // stored config options
        ConfigStoredOpt m_options[MAX_CONF_OPT];
};

#define sConfig Singleton<ConfigMgr>::getInstance()

#endif
