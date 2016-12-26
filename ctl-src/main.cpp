#include "general.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>

// only purpose of this communicator is to connect to remote socket, perform action, read response and exit

int main(int argc, char** argv)
{
    // at least one parameter is needed
    if (argc < 2)
    {
        std::cerr << "Not enough parameters." << std::endl
                  << "Usage: " << argv[0] << " <command>" << std::endl;
        return 1;
    }

    int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    int res;

    // TODO: load IP from config
    std::string addrstr = "127.0.0.1";

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    // TODO: load port from config
    addr.sin_port = htons(3460);
    if (inet_pton(AF_INET, addrstr.c_str(), &addr.sin_addr) != 1)
    {
        std::cerr << "Invalid bind address: " << addrstr.c_str() << std::endl;
        return false;
    }

    res = connect(sock, (sockaddr*)&addr, sizeof(sockaddr_in));

    if (res != 0)
    {
        std::cerr << "Could not connect to target" << std::endl;
        return 2;
    }

    // concat all arguments to one string
    std::string concat = "";
    for (size_t i = 1; i < argc; i++)
    {
        if (i != 1)
            concat += " ";
        concat += argv[i];
    }

    // send request
    send(sock, concat.c_str(), concat.length(), 0);

    uint32_t len;
    std::string instr;

    // receive length following
    res = recv(sock, &len, 4, 0);
    if (res != 4)
        return 3;

    // convert to host byte order
    len = ntohl(len);

    // receive everything available
    char buf[128];
    uint32_t rec = 0;
    while (rec < len)
    {
        res = recv(sock, buf, 127, 0);
        if (res <= 0)
            break;

        buf[res] = '\0';

        instr += buf;
    }

    // output
    std::cout << instr << std::endl;

    // end
    close(sock);

    return 0;
}
