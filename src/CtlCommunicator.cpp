#include "general.h"
#include "CtlCommunicator.h"
#include "Config.h"

#include <set>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <algorithm>
#include <sstream>

CtlCommunicator::CtlCommunicator()
{
    m_isActive = false;
}

bool CtlCommunicator::Init()
{
    m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int param = 1;
    if (setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, (const char*)&param, sizeof(int)) == -1)
        std::cerr << "Could not set SO_REUSEADDR flag to control socket, bind may fail" << std::endl;

    sockaddr_in addr;

    addr.sin_family = AF_INET;
    addr.sin_port = htons((uint16_t)sConfig->GetConfigIntValue(CONF_CTL_PORT));
    if (inet_pton(AF_INET, sConfig->GetConfigStringValue(CONF_CTL_BIND_IP), &addr.sin_addr) != 1)
    {
        std::cerr << "Invalid bind address: " << sConfig->GetConfigStringValue(CONF_CTL_BIND_IP) << std::endl;
        return false;
    }

    if (bind(m_socket, (sockaddr*)&addr, sizeof(sockaddr_in)) != 0)
    {
        std::cerr << "Could not bind to control socket" << std::endl;
        return false;
    }

    if (listen(m_socket, 10) != 0)
    {
        std::cerr << "Could not create listen backlog" << std::endl;
        return false;
    }

    m_isActive = true;

    return true;
}

void CtlCommunicator::Run()
{
    fd_set mset;
    fd_set rdset, exset;
    std::set<int> itsocks;
    int nfds = m_socket+1;
    FD_ZERO(&mset);

    FD_SET(m_socket, &mset);
    itsocks.insert(m_socket);

    int res;
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    int rdsize;
    char buf[1024];

    while (m_isActive)
    {
        memcpy(&rdset, &mset, sizeof(fd_set));
        memcpy(&exset, &mset, sizeof(fd_set));

        res = select(nfds, &rdset, nullptr, &exset, nullptr);
        if (res < 0)
        {
            std::cerr << "select(): error " << errno << ", result: " << res << std::endl;
            continue;
        }
        else if (res == 0)
        {
            // timeout
        }
        else
        {
            for (int sck : itsocks)
            {
                if (FD_ISSET(sck, &rdset))
                {
                    // our socket - accept incoming connections
                    if (sck == m_socket)
                    {
                        res = accept(m_socket, (sockaddr*)&addr, &addrlen);
                        if (res <= 0)
                            std::cerr << "accept(): error " << errno << ", result: " << res << std::endl;
                        else
                        {
                            itsocks.insert(res);
                            FD_SET(res, &mset);
                            if (res + 1 > nfds)
                                nfds = res + 1;
                        }
                    }
                    else // client socket, read if there's something on input
                    {
                        ioctl(sck, FIONREAD, &rdsize);
                        if (rdsize > 0)
                        {
                            // limit to buffer size
                            if (rdsize > 1023)
                                rdsize = 1023;

                            // receive, add null terminator
                            res = recv(sck, buf, rdsize, 0);
                            if (res == rdsize)
                            {
                                buf[rdsize] = '\0';
                                _ProcessRequest(sck, buf);

                                close(sck);
                                FD_CLR(sck, &mset);
                            }
                            else
                            {
                                // invalid read, close and return
                                close(sck);
                                FD_CLR(sck, &mset);
                            }
                        }
                        else if (rdsize < 0)
                        {
                            // disconnected (remote side closed connection)
                            FD_CLR(sck, &mset);
                        }
                    }
                }

                // an error occurred on socket
                if (FD_ISSET(sck, &exset))
                {
                    // clear socket from fdset
                    FD_CLR(sck, &mset);
                }
            }
        }
    }
}

void CtlCommunicator::_ProcessRequest(int sock, std::string request)
{
    std::vector<std::string> tokens;

    std::istringstream ss(request);
    std::string token;

    while(std::getline(ss, token, ' '))
        tokens.push_back(token);

    if (tokens.size() == 0)
        return;

    if (tokens[0] == "show")
    {
        if (tokens.size() >= 2)
        {
            if (tokens[1] == "status" && tokens.size() == 2)
            {
                _SendResponse(sock, CtlResponseBuilder::GetOverallStatus());
            }
            else if (tokens[1] == "domains" && tokens.size() == 2)
            {
                _SendResponse(sock, CtlResponseBuilder::ListDomains());
            }
            else if (tokens[1] == "vlans" && (tokens.size() == 2 || tokens.size() == 3))
            {
                _SendResponse(sock, CtlResponseBuilder::ListVLANs(tokens.size() == 3 ? tokens[2].c_str() : nullptr));
            }
            else
                _SendResponse(sock, "ERR:PARAMETERS");
        }
        else
            _SendResponse(sock, "ERR:BASEPARAMETERS");
    }
    else if (tokens[0] == "export")
    {
        if (tokens.size() == 2)
        {
            // tokens[1] = domain name

            // TODO: implement
            _SendResponse(sock, "ERR:NOTIMPLEMENTED");
        }
        else if (tokens.size() == 3)
        {
            // tokens[1] = domain name
            // tokens[2] = revision (or specifier)

            // TODO: implement
            _SendResponse(sock, "ERR:NOTIMPLEMENTED");
        }
        else
            _SendResponse(sock, "ERR:PARAMETERS");
    }
    else if (tokens[0] == "exit")
    {
        m_isActive = false;
        _SendResponse(sock, "BYE");
    }
    else
        _SendResponse(sock, "ERR:UNKCMD");
}

void CtlCommunicator::_SendResponse(int sock, std::string response)
{
    // send size
    uint32_t sz = htonl((uint32_t)response.length());
    send(sock, &sz, sizeof(uint32_t), 0);

    // send message itself
    send(sock, response.c_str(), response.length(), 0);
}
