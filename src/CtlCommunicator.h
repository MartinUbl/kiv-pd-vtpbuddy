#ifndef VTPBUDDY_CTLCOMMUNICATOR_H
#define VTPBUDDY_CTLCOMMUNICATOR_H

#include "Singleton.h"

class CtlCommunicator
{
    friend class Singleton<CtlCommunicator>;
    public:
        // inits ctl communicator
        bool Init();

        // runs ctl communicator
        void Run();

    protected:
        // protected singleton constructor
        CtlCommunicator();

        // process incoming request through socket
        void _ProcessRequest(int sock, std::string request);

        // send response to socket
        void _SendResponse(int sock, std::string response);

    private:
        // communicator socket
        int m_socket;

        // running flag
        bool m_isActive;
};

#define sCtlComm Singleton<CtlCommunicator>::getInstance()

// response builder block

namespace CtlResponseBuilder
{
    // retrieves formatted string - overall daemon status
    std::string GetOverallStatus();
    // retrieves formatted string - list of domains
    std::string ListDomains();
    // retrieves formatted string - list of VLANs within domain
    std::string ListVLANs(const char* domain);
}

#endif
