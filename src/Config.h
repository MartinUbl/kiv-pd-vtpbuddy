#ifndef VTPBUDDY_CONFIG_H
#define VTPBUDDY_CONFIG_H

#include "Singleton.h"

#define SYS_CONFIG_FILENAME "vtpbuddy.cfg"

#define SYS_CONFIG_PATH "/etc/" SYS_CONFIG_FILENAME

enum ConfigOpts
{
    CONF_MODE       = 0,
    CONF_INTERFACE  = 1,
    MAX_CONF_OPT
};

enum ConfigOptType
{
    CONF_OPT_TYPE_INT       = 0,
    CONF_OPT_TYPE_STRING    = 1,
    MAX_CONF_OPT_TYPE
};

struct ConfigStoredOpt
{
    std::string name;
    ConfigOptType type;
    ConfigOpts opt;

    std::string strValue;
    int64_t intValue;

    bool filled;
};

class ConfigMgr
{
    friend class Singleton<ConfigMgr>;
    public:
        bool LoadConfig();

        int64_t GetConfigIntValue(ConfigOpts opt);
        const char* GetConfigStringValue(ConfigOpts opt);

    protected:
        ConfigMgr();

        void InitDefaults();

        bool _ParseConfigOpt(const std::string line, std::string &name, std::string &value);
        ConfigStoredOpt* _FindConfigOpt(const char* name);

        void _InitConfigIntValue(ConfigOpts opt, const char* name, int value);
        void _SetConfigIntValue(ConfigOpts opt, int value, bool initialized = false);
        void _InitConfigStringValue(ConfigOpts opt, const char* name, const char* value);
        void _SetConfigStringValue(ConfigOpts opt, const char* value, bool initialized = false);

    private:
        ConfigStoredOpt m_options[MAX_CONF_OPT];
};

#define sConfig Singleton<ConfigMgr>::getInstance()

#endif
