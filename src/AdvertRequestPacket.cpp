/*----------------------------------------------------------------------------
 * AdvertRequestPacket.cpp
 *
 *  Created on: 5. 12. 2014
 *      Author: Lukáš Kvídera
 *	   Version: 0.0
 *		Jabber:	segfault@dione.zcu.cz
 ---------------------------------------------------------------------------*/
#include "AdvertRequestPacket.h"

#include <cstring>

#ifdef _WIN32
#include <Windows.h>
#else
#include <arpa/inet.h>
#endif

#include "VTP3CommonHeader.h"
#include "VTP3Output.h"

using namespace VTP3;

int AdvertRequestPacket::send(Connection const& connection)
{
    AdvertRequestPacket pkt;

    std::memcpy(&pkt.header, &header, sizeof(header));
    pkt.start = htonl(start);

    return VTP3::send(connection, &pkt, sizeof(*this));
}

std::ostream& VTP3::operator<<(std::ostream& os, AdvertRequestPacket const& pkt)
{
    os << pkt.header << "\n" << "\tStart:\t" << pkt.start;
    return os;
}
