/*----------------------------------------------------------------------------
 * common.h
 *
 *  Created on: 4. 12. 2014
 *      Author: Lukáš Kvídera
 *	   Version: 0.0
 *		Jabber:	segfault@dione.zcu.cz
 ---------------------------------------------------------------------------*/
#ifndef COMMON_H_
#define COMMON_H_

#include <cstdint>
#include <memory>

#ifndef ETH_FRAME_LEN
#define ETH_FRAME_LEN 1518
#endif

#ifndef ETH_P_8021Q
#define ETH_P_8021Q 0x8100
#endif

#ifndef ETH_ALEN
#define ETH_ALEN 6
#endif

#ifndef ETH_P_ALL
#define ETH_P_ALL 0x0003
#endif

#define SNAP_LEN 1518 /* default snap length - maximum bytes per packet to capture */
#define PROMISC_MODE 1 /* promiscuous mode */
#define READ_TIME_OUT 1000 /* read time out in milliseconds. just use non-zero timeout */
#define SNAP_8023_OFFSET 14
#define SNAP_ETHERNETII_OFFSET 18
#define SNAP_DSAP_SSAP_HEADER 0xAAAA
#define VTP_ID 0x2003
#define VTP_ID_OFFSET 6
#define VTP_VERSION_OFFSET 8
#define VTP_MESSAGE_OFFSET 1
#define LLC_HEADER_SIZE 8
#define BUF_MAX_LEN 65536

#define ORG_CODE_LENGTH 3

namespace VTP3 {

    enum MessageType
    {
        SMMARY_ADVERT = 0x01,
        SUBSET_ADVERT = 0x02,
        ADVERT_REQUEST = 0x03
    };

    /* SNAP encapsulated in LLC */
    struct LLCHeader
    {
        uint8_t dsap;                       // SNAP, 1B field
        uint8_t ssap;                       // SNAP, 1B field
        uint8_t ctrl;                       // SNAP, 1B field
        uint8_t org_code[ORG_CODE_LENGTH];  // SNAP, 3B field called OUI - organizationally unique identifier - cisco
        uint16_t pid;                       // SNAP, 2B field called TYPE - 0x2003 for VTP
    };

    struct VlanInfoExtended
    {
        uint8_t type;                       // Captured packets have this field 1 byte long... but cisco says it should be 2
        uint8_t length;                     // this too
        /* space[length] */
    };
}

#endif /* COMMON_H_ */
