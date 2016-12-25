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

    // TODO: use tagged frame if requested by config - probably won't be used in most end-device cases, but
    // in case of i.e. OpenWRT-based node, this may be needed
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

    memset(&m_ifopts, 0, sizeof(m_ifopts));
    snprintf(m_ifopts.ifr_name, sizeof(m_ifopts.ifr_name), iface);
    if (ioctl(m_socket, SIOCGIFINDEX, &m_ifopts) < 0)
    {
        //
    }

    int ifIndex = m_ifopts.ifr_ifindex;

    struct sockaddr_ll sll;
    memset(&sll, 0, sizeof(sockaddr_ll));
    sll.sll_family = AF_PACKET;
    sll.sll_ifindex = m_ifopts.ifr_ifindex;
    sll.sll_protocol = htons(ETH_P_ALL);
    sll.sll_pkttype = PACKET_HOST;
    if ((bind(m_socket , (struct sockaddr*)&sll , sizeof(sll))) < 0)
    {
        std::cerr << "Could not bind to interface " << iface << ", errno = " << errno << std::endl;
    }

    // bind to specific device (interface), so we would not receive excessive amount of data
    if (setsockopt(m_socket, SOL_SOCKET, SO_BINDTODEVICE, (void*)&m_ifopts, sizeof(m_ifopts)) != 0)
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

    struct packet_mreq mreq;
    mreq.mr_ifindex = ifIndex;
    mreq.mr_type = PACKET_MR_PROMISC;
    mreq.mr_alen = ETH_ALEN;
    memcpy(mreq.mr_address, (uint8_t*)m_ifopts.ifr_hwaddr.sa_data, ETH_ALEN);
    setsockopt(m_socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof(packet_mreq));

    sRuntimeGlobals->SetSourceMAC((uint8_t*)m_ifopts.ifr_hwaddr.sa_data);

    // set some implicit IP in case the interface doesn't have assigned one
    in_addr addr;
    addr.s_addr = htonl(0xFDFDFDFD);

    // retrieve IP address; may not be assigned
    if (ioctl(m_socket, SIOCGIFADDR, &m_ifopts) == 0)
        addr = ((struct sockaddr_in *)&m_ifopts.ifr_addr)->sin_addr;

    sRuntimeGlobals->SetSourceIP((uint32_t)addr.s_addr);

    // set base timeout
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(struct timeval));

    return true;
}

void VTPNetwork::_ListenerRun()
{
    int res, res2;
    struct sockaddr_ll addr;
    socklen_t addr_len = sizeof(addr);

    // loop while we want it
    while (m_listenThreadRunning)
    {
        res = recvfrom(m_socket, m_netBuffer, NET_BUFFER_SIZE, 0, (struct sockaddr*)&addr, &addr_len);
        if (res <= 0 && errno == EAGAIN)
            continue;
        if (res < 0)
        {
            std::cerr << "recvfrom(): failed, errno = " << errno << std::endl;
            break; // break, this may be fatal error, and we cannot risk filling syslog with infinite loop generated messages
        }
        else if (addr.sll_pkttype == PACKET_OUTGOING)
            continue;

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
    const size_t totalLen = encaps->GetEncapsulationSize() + sizeof(VTPHeader) + pkt->data_len;

    // TODO: if tagged frame, call encaps->SetVLANId to set value of tag field

    uint8_t* outBuffer = new uint8_t[totalLen];

    // set total frame length (the encapsulating header does not count)
    encaps->SetFrameLength(totalLen - encaps->GetEncapsulationSize() + 8);
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

    saddr.sll_family = AF_PACKET;
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
