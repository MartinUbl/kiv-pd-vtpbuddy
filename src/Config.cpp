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
    _InitConfigIntValue(CONF_MODE, "mode", OM_SERVER);
    _InitConfigStringValue(CONF_INTERFACE, "interface", "eth1");
}

void ConfigMgr::_InitConfigIntValue(ConfigOpts opt, const char* name, int value)
{
    m_options[opt].name = name;
    _SetConfigIntValue(opt, value, false);
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
    if (m_options[opt].type != CONF_OPT_TYPE_INT)
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

    for (int i = 0; i < line.length(); i++)
    {
        if (ln[i] == ' ')
            continue;

        if (first == -1)
        {
            if (ln[i] == '#')
                return false;

            first = i;
        }

        if (ln[i] == '=' && firstpass)
        {
            name = std::string(&ln[first], last - first + 1);
            first = -1;
            last = -1;
            firstpass = false;
        }

        last = i;
    }

    if (firstpass)
    {
        if (first != -1)
            std::cerr << "Invalid line in config file: " << line.c_str() << std::endl;
        return false;
    }

    if (first != -1)
        value = std::string(&ln[first], last - first + 1);

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

bool ConfigMgr::LoadConfig()
{
    ConfigStoredOpt* opt;

    InitDefaults();

    // TODO: find config in multiple paths, for now, just workdir
    //       in future, search in /etc/vtpbuddy/, etc. also

    // ifstream scope
    {
        std::ifstream ifs(SYS_CONFIG_FILENAME, std::ifstream::in);

        if (ifs.is_open())
        {
            std::cout << "Config found" << std::endl;
            std::string line, name, value;

            while (ifs.good() && std::getline(ifs, line))
            {
                if (!_ParseConfigOpt(line, name, value))
                    continue;

                opt = _FindConfigOpt(name.c_str());
                if (!opt)
                {
                    std::cerr << "Invalid config option: " << name.c_str() << std::endl;
                    continue;
                }

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
                else if (opt->type == CONF_OPT_TYPE_STRING)
                    _SetConfigStringValue(opt->opt, value.c_str(), true);
            }
        }
    }

    return true;
}
