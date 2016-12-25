#include "general.h"
#include "Config.h"
#include "Network.h"
#include "DomainMgr.h"
#include "Enums.h"
#include "RuntimeGlobals.h"
#include "Versioning.h"
#include "Shared.h"
#include "CtlCommunicator.h"
#include "DelayWorkThread.h"

#include <sstream>
#include <signal.h>

bool _appRunning = true;

void signal_handler(int signo)
{
    std::cout << std::endl << "Exiting..." << std::endl;

    sDomainMgr->Finalize();

    sNetwork->Terminate();

    sDelayWorker->Finalize();

    exit(0);
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

    // initialize delay worker
    if (!sDelayWorker->Init())
    {
        std::cerr << "Could not spawn delay worker" << std::endl;
        return 4;
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

    if (!sDomainMgr->Init())
        std::cerr << "Could not initialize periodic summary advertisement sender; application will not advertise its configuration automatically" << std::endl;

    if (!sCtlComm->Init())
    {
        std::cerr << "Could not initialize control communicator!" << std::endl;
        return 5;
    }

    sCtlComm->Run();

    sDomainMgr->Finalize();

    sNetwork->Terminate();

    sDelayWorker->Finalize();

    return 0;
}
