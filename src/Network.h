#ifndef VTPBUDDY_NETWORK_H
#define VTPBUDDY_NETWORK_H

#include "Singleton.h"

#include <net/if.h>
#include <thread>

// maximum buffer size for received frames/packets
#define NET_BUFFER_SIZE 65536

/**
 * Network manager class for maintaining all sorts of network routines
 */
class VTPNetwork
{
    friend class Singleton<VTPNetwork>;
    public:
        // Initializes socket for raw receiving on specified interface
        bool InitSocket(const char* iface);

        // Starts listener thread
        void StartListener();
        // Terminates networking, listener thread, cleans up
        void Terminate();

    protected:
        // protected singleton constructor
        VTPNetwork();

        // thread runnable method
        void _ListenerRun();

        // raw socket opened
        int m_socket;
        // stored interface options
        ifreq m_ifopts;
        // is thread listener running?
        bool m_listenThreadRunning;
        // thread listener instance
        std::thread* m_listenThread;

        // receive buffer
        uint8_t m_netBuffer[NET_BUFFER_SIZE];
};

#define sNetwork Singleton<VTPNetwork>::getInstance()

#endif
