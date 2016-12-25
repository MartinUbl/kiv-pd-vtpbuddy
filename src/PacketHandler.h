#ifndef VTPBUDDY_PACKETHANDLER_H
#define VTPBUDDY_PACKETHANDLER_H

// dispatches received packet, if it's VTP packet; returns 0 on success, 2 if some bytes still needs to be retrieved
size_t DispatchReceivedPacket(uint8_t* buffer, size_t len);

#endif
