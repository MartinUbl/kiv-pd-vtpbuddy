#include "general.h"
#include "Network.h"
#include "PacketHandler.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>

VTPNetwork::VTPNetwork()
{
    m_listenThreadRunning = false;
}

bool VTPNetwork::InitSocket(const char* iface)
{
    int sockopt;

    // create socket
    m_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));

    if (m_socket <= 0)
    {
        std::cerr << "Could not initialize raw socket on interface " << iface << ", errno = " << errno << std::endl;
        return false;
    }

    // set SO_REUSEADDR on socket
    sockopt = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt)) != 0)
    {
        std::cerr << "Could not set REUSE flag to socket, errno = " << errno << std::endl;
        return false;
    }

    // bind to specific device (interface), so we would not receive excessive amount of data
    if (setsockopt(m_socket, SOL_SOCKET, SO_BINDTODEVICE, iface, strlen(iface)) != 0)
    {
        std::cerr << "Could not bind to interface " << iface << ", errno = " << errno << std::endl;
        return false;
    }

    // switch interface to promiscuous mode
    strncpy(m_ifopts.ifr_name, iface, IFNAMSIZ-1);

    ioctl(m_socket, SIOCGIFFLAGS, &m_ifopts);
    m_ifopts.ifr_flags |= IFF_PROMISC;

    if (ioctl(m_socket, SIOCSIFFLAGS, &m_ifopts) == -1)
    {
        std::cerr << "Could not set promiscuous mode on interface " << iface << ", errno = " << errno << std::endl;
        return false;
    }

    return true;
}

void VTPNetwork::_ListenerRun()
{
    int res;

    // loop while we want it
    while (m_listenThreadRunning)
    {
        res = recvfrom(m_socket, m_netBuffer, NET_BUFFER_SIZE, 0, nullptr, nullptr);
        if (res <= 0 && errno == EAGAIN)
            continue;
        if (res < 0)
        {
            std::cerr << "recvfrom(): failed, errno = " << errno << std::endl;
            // break;
        }

        DispatchReceivedPacket((uint8_t*)m_netBuffer, res);
    }
}

void VTPNetwork::Terminate()
{
    // terminate listener thread
    m_listenThreadRunning = false;

    // switch promiscuous mode on interface back to non-promiscuous
    ioctl(m_socket, SIOCGIFFLAGS, &m_ifopts);
    m_ifopts.ifr_flags &= ~(IFF_PROMISC);

    if (ioctl(m_socket, SIOCSIFFLAGS, &m_ifopts) == -1)
        std::cerr << "Could not unset promiscuous mode on interface " << m_ifopts.ifr_name << ", errno = " << errno << std::endl;

    // close socket
    close(m_socket);

    // wait for listener thread to be terminated
    if (m_listenThread && m_listenThread->joinable())
        m_listenThread->join();
}

void VTPNetwork::StartListener()
{
    // secure single listener to be running at one time
    if (m_listenThreadRunning)
    {
        std::cerr << "Attempt to start second network listener thread" << std::endl;
        return;
    }

    // start new thread
    m_listenThreadRunning = true;

    m_listenThread = new std::thread(&VTPNetwork::_ListenerRun, this);
}
