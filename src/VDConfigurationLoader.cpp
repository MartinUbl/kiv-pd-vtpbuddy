#include "general.h"
#include "ConfigurationLoader.h"
#include "Shared.h"
#include "VLAN.h"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

VDConfigurationLoader::VDConfigurationLoader() : ConfigurationLoader(CONFIGURATION_TYPE_VLANDB)
{
    //
}

bool VDConfigurationLoader::LoadVLAN(std::ifstream& ifs, VLANRecord* target)
{
    // nothing on input at all - exit
    if (!ifs.good())
        return false;

    std::string line;

    // Ignore lines until start tag is reaged
    while (ifs.good())
    {
        std::getline(ifs, line);
        if (line == "#!VLAN:START")
            break;
    }

    // we reached end during seek for next vlan tag - exit
    if (!ifs.good())
        return false;

    uint16_t parsedVlan;
    uint32_t tmp;
    size_t off;
    std::vector<std::string> tokens;

    while (ifs.good())
    {
        // retrieve line, trim it
        std::getline(ifs, line);
        line = str_trim(line);
        if (line.length() == 0)
            continue;

        // ending tag
        if (line == "#!VLAN:END")
            break;

        // tokenize input line

        tokens.clear();

        std::istringstream ss(line);
        std::string token;

        while(std::getline(ss, token, ' '))
            tokens.push_back(token);

        // at least 3 tokens (vlan <id> ...)
        if (tokens.size() <= 2)
            continue;

        if (tokens[0] != "vlan")
        {
            std::cerr << "Invalid line in VLAN configuration: " << line.c_str() << std::endl;
            continue;
        }

        tmp = std::stoll(tokens[1], &off);
        if (off != tokens[1].length() || tmp == 0 || tmp >= 4096)
        {
            std::cerr << "Invalid VLAN ID in VLAN configuration: " << line.c_str() << std::endl;
            continue;
        }

        parsedVlan = (uint16_t)tmp;
        std::string comName = tokens[2];
        int64_t tmp;

        target->id = parsedVlan;

        // try to parse the rest of line

        try
        {
            if (comName == "name" && secureArgCount(tokens, 4))
            {
                target->name = tokens[3];
            }
            else if (comName == "media" && secureArgCount(tokens, 4))
            {
                tmp = FindConfMediaTypeId(tokens[3].c_str());
                if (tmp == FIND_IDENTIFIER_ERROR)
                    throw std::invalid_argument(tokens[3]);
                target->type = (uint16_t)tmp;
            }
            else if (comName == "state" && secureArgCount(tokens, 4))
            {
                tmp = FindConfActiveStateId(tokens[3].c_str());
                if (tmp == FIND_IDENTIFIER_ERROR)
                    throw std::invalid_argument(tokens[3]);
                target->status = (uint16_t)tmp;
            }
            else if (comName == "said" && secureArgCount(tokens, 4))
            {
                target->index80210 = parseLong_ex(tokens[3].c_str());
            }
            else if (comName == "are" && secureArgCount(tokens, 4))
            {
                tmp = parseLong_ex(tokens[3].c_str());
                target->features[VLAN_FEAT_ARE_HOPS] = (uint16_t)tmp;
            }
            else if (comName == "ste" && secureArgCount(tokens, 4))
            {
                tmp = parseLong_ex(tokens[3].c_str());
                target->features[VLAN_FEAT_STE_HOPS] = (uint16_t)tmp;
            }
            else if (comName == "bridge" && secureArgCount(tokens, 4))
            {
                if (tokens[3] == "type" && secureArgCount(tokens, 5))
                {
                    tmp = FindConfBridgeModeId(tokens[4].c_str());
                    if (tmp == FIND_IDENTIFIER_ERROR)
                        throw std::invalid_argument(tokens[4]);
                    target->features[VLAN_FEAT_BRIDGE_MODE] = (uint16_t)tmp;
                }
                else
                {
                    tmp = parseLong_ex(tokens[3].c_str());
                    target->features[VLAN_FEAT_BRIDGE_NO] = (uint16_t)tmp;
                }
            }
            else if (comName == "parent" && secureArgCount(tokens, 4))
            {
                tmp = parseLong_ex(tokens[3].c_str());
                target->features[VLAN_FEAT_PARENT] = (uint16_t)tmp;
            }
            else if (comName == "ring" && secureArgCount(tokens, 4))
            {
                tmp = parseLong_ex(tokens[3].c_str());
                target->features[VLAN_FEAT_RING_NO] = (uint16_t)tmp;
            }
            else if (comName == "tb-vlan1" && secureArgCount(tokens, 4))
            {
                tmp = parseLong_ex(tokens[3].c_str());
                target->features[VLAN_FEAT_TRANS1] = (uint16_t)tmp;
            }
            else if (comName == "tb-vlan2" && secureArgCount(tokens, 4))
            {
                tmp = parseLong_ex(tokens[3].c_str());
                target->features[VLAN_FEAT_TRANS2] = (uint16_t)tmp;
            }
            else if (comName == "mtu" && secureArgCount(tokens, 4))
            {
                tmp = parseLong_ex(tokens[3].c_str());
                target->mtu = (uint16_t)tmp;
            }
            else if (comName == "stp" && secureArgCount(tokens, 5)) // automatically expecting one more parameter
            {
                if (tokens[3] == "type")
                {
                    tmp = FindConfSTPModeId(tokens[4].c_str());
                    if (tmp == FIND_IDENTIFIER_ERROR)
                        throw std::invalid_argument(tokens[4]);
                    target->features[VLAN_FEAT_STP] = (uint16_t)tmp;
                }
            }
        }
        catch (std::invalid_argument &ex)
        {
            std::cerr << "Invalid property identifier: " << ex.what() << std::endl;
            continue;
        }
        catch (std::domain_error &err)
        {
            std::cerr << "Argument error: " << err.what() << std::endl;
            continue;
        }
    }

    return true;
}
