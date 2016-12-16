#include "general.h"
#include "Enums.h"
#include "Config.h"

#include <sstream>
#include <fstream>

ConfigMgr::ConfigMgr()
{
    //
}

void ConfigMgr::InitDefaults()
{
    _InitConfigEnumValue(CONF_MODE, "mode", OM_CLIENT);
    _InitConfigStringValue(CONF_INTERFACE, "interface", "eth1");
    _InitConfigEnumValue(CONF_ENCAPS, "encapsulation", ENCAPS_DOT1Q);
    _InitConfigIntValue(CONF_VTP_VERSION, "vtp_version", 2);
    _InitConfigStringValue(CONF_PRIM_DOMAIN, "domain", "NOT_CONFIGURED");
    _InitConfigStringValue(CONF_PRIM_PASSWORD, "password", "");
    _InitConfigEnumValue(CONF_VLAN_CONF_TYPE, "vlan_conf_type", CONFIGURATION_TYPE_VLANDB);
}

int64_t ConfigMgr::_ParseEnumValue(ConfigOpts opt, std::string str)
{
    if (opt == CONF_ENCAPS)
    {
        if (str == "isl")
            return ENCAPS_ISL;
        if (str == "dot1q")
            return ENCAPS_DOT1Q;
    }
    else if (opt == CONF_VLAN_CONF_TYPE)
    {
        if (str == "config")
            return CONFIGURATION_TYPE_CONFIG;
        if (str == "vlandb")
            return CONFIGURATION_TYPE_VLANDB;
    }
    else if (opt == CONF_MODE)
    {
        if (str == "server")
            return OM_SERVER;
        else if (str == "client")
            return OM_CLIENT;
        else if (str == "transparent")
            return OM_TRANSPARENT;
    }

    return -1;
}

void ConfigMgr::_InitConfigIntValue(ConfigOpts opt, const char* name, int value)
{
    m_options[opt].name = name;
    _SetConfigIntValue(opt, value, false);
}

void ConfigMgr::_InitConfigEnumValue(ConfigOpts opt, const char* name, int value)
{
    m_options[opt].name = name;
    _SetConfigIntValue(opt, value, false);
    m_options[opt].type = CONF_OPT_TYPE_ENUM;
}

void ConfigMgr::_SetConfigIntValue(ConfigOpts opt, int value, bool initialized)
{
    m_options[opt].intValue = value;
    m_options[opt].opt = opt;
    m_options[opt].type = CONF_OPT_TYPE_INT;
    m_options[opt].filled = initialized;
}

void ConfigMgr::_InitConfigStringValue(ConfigOpts opt, const char* name, const char* value)
{
    m_options[opt].name = name;
    _SetConfigStringValue(opt, value, false);
}

void ConfigMgr::_SetConfigStringValue(ConfigOpts opt, const char* value, bool initialized)
{
    m_options[opt].strValue = value;
    m_options[opt].opt = opt;
    m_options[opt].type = CONF_OPT_TYPE_STRING;
    m_options[opt].filled = initialized;
}

int64_t ConfigMgr::GetConfigIntValue(ConfigOpts opt)
{
    if (m_options[opt].type != CONF_OPT_TYPE_INT && m_options[opt].type != CONF_OPT_TYPE_ENUM)
        return 0;
    return m_options[opt].intValue;
}

const char* ConfigMgr::GetConfigStringValue(ConfigOpts opt)
{
    if (m_options[opt].type != CONF_OPT_TYPE_STRING)
        return "";
    return m_options[opt].strValue.c_str();
}

bool ConfigMgr::_ParseConfigOpt(const std::string line, std::string &name, std::string &value)
{
    const char* ln = line.c_str();

    bool firstpass = true;
    int first = -1, last = -1;

    // go through whole line
    for (int i = 0; i < line.length(); i++)
    {
        // ignore spaces
        if (ln[i] == ' ')
            continue;

        // set first non-space character index when not set
        if (first == -1)
        {
            // if it's comment, ignore
            if (ln[i] == '#')
                return false;

            first = i;
        }

        // "=" sign stores everything parsed until now and resets parser to second pass
        if (ln[i] == '=' && firstpass)
        {
            name = std::string(&ln[first], last - first + 1);
            first = -1;
            last = -1;
            firstpass = false;
        }

        // always store last valid character
        last = i;
    }

    // if no second pass ever started on this line, it's considered error
    if (firstpass)
    {
        if (first != -1)
            std::cerr << "Invalid line in config file: " << line.c_str() << std::endl;
        return false;
    }

    // if the value was entered, store it
    if (first != -1)
        value = std::string(&ln[first], last - first + 1);
    else
        value = "";

    return true;
}

ConfigStoredOpt* ConfigMgr::_FindConfigOpt(const char* name)
{
    for (int i = 0; i < MAX_CONF_OPT; i++)
    {
        if (m_options[i].name == name)
            return &m_options[i];
    }

    return nullptr;
}

std::string ConfigMgr::_FindConfigFile()
{
    // use C-style check, it's way faster than ifstream

    FILE* f;

    f = fopen(SYS_CONFIG_FILENAME, "r");
    if (f)
    {
        fclose(f);
        return SYS_CONFIG_FILENAME;
    }

    f = fopen(SYS_CONFIG_PATH, "r");
    if (f)
    {
        fclose(f);
        return SYS_CONFIG_PATH;
    }

    return "";
}

bool ConfigMgr::LoadConfig()
{
    ConfigStoredOpt* opt;

    InitDefaults();

    std::string configPath = _FindConfigFile();

    if (configPath.length() == 0)
    {
        std::cerr << "No config file found! Please, create one as " << SYS_CONFIG_PATH << std::endl;
        return false;
    }

    // ifstream scope
    {
        std::ifstream ifs(configPath.c_str(), std::ifstream::in);

        if (ifs.is_open())
        {
            std::string line, name, value;

            // while there's something to parse
            while (ifs.good() && std::getline(ifs, line))
            {
                // try to parse line
                if (!_ParseConfigOpt(line, name, value))
                    continue;

                // find option (must be previously added in InitDefaults
                opt = _FindConfigOpt(name.c_str());
                if (!opt)
                {
                    std::cerr << "Invalid config option: " << name.c_str() << std::endl;
                    continue;
                }

                // integer value - convert to integer and store
                if (opt->type == CONF_OPT_TYPE_INT)
                {
                    size_t lpos;
                    int64_t tval = std::stoll(value, &lpos);

                    if (lpos != value.length())
                    {
                        std::cerr << "Invalid numeric value '" << value.c_str() << "' for option: " << name.c_str() << std::endl;
                        continue;
                    }

                    _SetConfigIntValue(opt->opt, tval, true);
                }
                // enum value - match integer value and store
                else if (opt->type == CONF_OPT_TYPE_ENUM)
                {
                    int64_t tval = _ParseEnumValue(opt->opt, value);
                    if (tval >= 0)
                        _SetConfigIntValue(opt->opt, tval, true);
                    else
                        std::cerr << "Invalid enum value '" << value.c_str() << "' for option: " << name.c_str() << std::endl;
                }
                // string value - just store
                else if (opt->type == CONF_OPT_TYPE_STRING)
                    _SetConfigStringValue(opt->opt, value.c_str(), true);
            }
        }
    }

    return true;
}
