#include "general.h"
#include "Config.h"
#include "Network.h"
#include "PacketHandler.h"
#include "RuntimeGlobals.h"
#include "Encapsulation.h"

#include <algorithm>

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

PreparedPacket* PreparedPacket::Prepare(uint8_t _vtp_code, size_t _data_len, std::string _domain_name)
{
    PreparedPacket* pkt = new PreparedPacket();

    // TODO: figure out tagging rules
    pkt->tagged = false;

    pkt->header.version = sConfig->GetConfigIntValue(CONF_VTP_VERSION);
    pkt->header.code = _vtp_code;
    pkt->header.reserved = 0;
    pkt->header.domain_len = std::min(_domain_name.length(), (size_t)MAX_VTP_DOMAIN_LENGTH);
    memcpy(pkt->header.domain_name, _domain_name.c_str(), pkt->header.domain_len);

    pkt->data = new uint8_t[_data_len];
    pkt->data_len = _data_len;
}

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

    // retrieve MAC address
    if (ioctl(m_socket, SIOCGIFHWADDR, &m_ifopts) == -1)
    {
        std::cerr << "Could not retrieve MAC address of interface " << iface << ", errno = " << errno << std::endl;
        return false;
    }

    sRuntimeGlobals->SetSourceMAC((uint8_t*)m_ifopts.ifr_hwaddr.sa_data);

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

void VTPNetwork::SendPreparedPacket(PreparedPacket* pkt)
{
    Encapsulation* encaps = sRuntimeGlobals->GetOutputEncapsulation(pkt->tagged);
    size_t totalLen = encaps->GetEncapsulationSize() + sizeof(VTPHeader) + pkt->data_len;

    uint8_t* outBuffer = new uint8_t[totalLen];

    // set total frame length (the encapsulating header does not count)
    encaps->SetFrameLength(totalLen - encaps->GetEncapsulationSize());
    encaps->AppendEncapsData(outBuffer);

    size_t offset = encaps->GetEncapsulationSize();

    memcpy(outBuffer + offset, &pkt->header, sizeof(VTPHeader));
    offset += sizeof(VTPHeader);
    memcpy(outBuffer + offset, pkt->data, pkt->data_len);

    // send!

    // retrieve device index, so the sendto function would know, where to send the frame
    sockaddr_ll saddr;
    if (ioctl(m_socket, SIOCGIFINDEX, &m_ifopts) < 0)
        std::cerr << "Error when trying to retrieve interface index; is interface still available? errno = " << errno << std::endl;

    saddr.sll_ifindex = m_ifopts.ifr_ifindex;
    saddr.sll_halen = ETH_ALEN;

    if (sendto(m_socket, outBuffer, totalLen, 0, (sockaddr*)&saddr, sizeof(sockaddr_ll)) < 0)
        std::cerr << "Failed to send packet through requested interface, errno = " << errno << std::endl;

    // cleanup
    delete pkt->data;
    delete pkt;
    delete outBuffer;
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
