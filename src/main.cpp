#include "general.h"
#include "Config.h"
#include "Network.h"

#include <signal.h>

void signal_handler(int signo)
{
    std::cout << std::endl << "Exiting..." << std::endl;

    sNetwork->Terminate();

    exit(0);
}

int main(int argc, char** argv)
{
    // hook SIGINT, so we could properly terminate whole process
    signal(SIGINT, signal_handler);

    // load config
    if (!sConfig->LoadConfig())
    {
        std::cerr << "Could not load config file!" << std::endl;
        return 1;
    }

    // initialize network
    if (!sNetwork->InitSocket(sConfig->GetConfigStringValue(CONF_INTERFACE)))
        return 2;

    sNetwork->StartListener();

    // cli loop
    while (true)
    {
        std::string line;

        std::cout << "VTPBuddy> ";

        std::getline(std::cin, line);

        // TODO: process commands
    }

    return 0;
}
