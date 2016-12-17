#include "general.h"
#include "Config.h"
#include "Network.h"
#include "DomainMgr.h"
#include "Enums.h"
#include "RuntimeGlobals.h"
#include "Versioning.h"
#include "Shared.h"

#include <sstream>
#include <signal.h>

bool _appRunning = true;

void signal_handler(int signo)
{
    std::cout << std::endl << "Exiting..." << std::endl;

    sNetwork->Terminate();

    exit(0);
}

bool process_command(std::string &comm)
{
    std::istringstream ss(comm);
    std::string token;

    std::vector<std::string> tokens;

    while (std::getline(ss, token, ' '))
        tokens.push_back(token);

    if (tokens.size() == 0)
        return false;

    size_t base = 0;
    bool negate = false;
    if (tokens[0] == "no")
    {
        if (tokens.size() == 1)
            return false;
        negate = true;
        base = 1;
    }

    // domain listing and manipulating commands
    if (tokens[base] == "domain")
    {
        // domain list - list all available domains
        if (tokens.size() > base + 1 && tokens[base + 1] == "list")
        {
            DomainMap const& domMap = sDomainMgr->GetDomainMap();

            std::cout << "List of domains in evidence:" << std::endl;
            for (auto itr : domMap)
                std::cout << "      " << itr.second->GetName() << std::endl;
        }
        // domain vlans <domain name> - list all VLANs within domain
        else if (tokens.size() > base + 1 && tokens[base + 1] == "vlans")
        {
            // find domain
            if (tokens.size() > base + 2 && tokens[base + 2] != "")
            {
                VTPDomain* dom = sDomainMgr->GetDomainByName(tokens[base + 2].c_str());
                if (!dom)
                    std::cout << "Domain " << tokens[base + 2].c_str() << " not found" << std::endl;
                else
                {
                    VLANMap const& vlanMap = dom->GetVLANMap();
                    std::cout << "List of VLANs in domain " << tokens[base + 2].c_str() << ": " << std::endl;
                    for (auto itr : vlanMap)
                        std::cout << "      " << itr.second->name.c_str() << std::endl;
                }
            }
        }
        else
        {
            std::cout << "Subcommands available:" << std::endl;
            std::cout << "      list    lists all available domains" << std::endl;
            std::cout << "      vlans   lists all vlans within domain" << std::endl;
        }

        return true;
    }
    // exit command - end execution
    else if (tokens[base] == "exit")
    {
        _appRunning = false;
        return true;
    }

    return false;
}

int main(int argc, char** argv)
{
    std::cout << "VTPBuddy, version 1.0" << std::endl;

    // hook SIGINT, so we could properly terminate whole process
    signal(SIGINT, signal_handler);

    // load config
    if (!sConfig->LoadConfig())
    {
        std::cerr << "Could not load config file!" << std::endl;
        return 1;
    }

    if (SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION)) == "")
    {
        std::cerr << "Invalid path configured as data location!" << std::endl;
        return 3;
    }

    // initialize network
    if (!sNetwork->InitSocket(sConfig->GetConfigStringValue(CONF_INTERFACE)))
        return 2;

    sRuntimeGlobals->InitializeRuntime();

    VersioningBase* verb = sRuntimeGlobals->GetVersioningTool();
    if (!verb || !verb->VerifyToolchainPresence())
    {
        std::cout << "Selected versioning toolchain was not found in the system, or is not supported!" << std::endl;
        return 1;
    }

    sNetwork->StartListener();

    std::cout << "Primary domain:    " << sConfig->GetConfigStringValue(CONF_PRIM_DOMAIN) << std::endl;

    if (sConfig->GetConfigIntValue(CONF_MODE) == OM_SERVER)
    {
        std::cout << "Server mode is not yet supported, please, use client or transparent mode" << std::endl;
        return 1;
    }
    else if (sConfig->GetConfigIntValue(CONF_MODE) == OM_CLIENT)
        std::cout << "Mode:              client" << std::endl;
    else if (sConfig->GetConfigIntValue(CONF_MODE) == OM_TRANSPARENT)
        std::cout << "Mode:              transparent" << std::endl;

    std::cout << "Database revision: " << verb->GetRevisionNumber() << std::endl;

    // create primary domain; this will also load the domain from file, if already stored
    VTPDomain* primaryDomain = sDomainMgr->CreateDomain(sConfig->GetConfigStringValue(CONF_PRIM_DOMAIN), sConfig->GetConfigStringValue(CONF_PRIM_PASSWORD));
    primaryDomain->Startup();

    // cli loop
    while (_appRunning)
    {
        std::string line;

        std::cout << "VTPBuddy> ";

        std::getline(std::cin, line);

        if (!process_command(line))
            std::cerr << "No such command." << std::endl;
    }

    sNetwork->Terminate();

    return 0;
}
