#ifndef VTPBUDDY_PACKETHANDLER_H
#define VTPBUDDY_PACKETHANDLER_H

// dispatches received packet, if it's VTP packet
void DispatchReceivedPacket(uint8_t* buffer, size_t len);

#endif
