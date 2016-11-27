/*----------------------------------------------------------------------------
 * VTP3CommonHeader.h
 *
 *  Created on: 9. 12. 2014
 *      Author: Lukáš Kvídera
 *	   Version: 0.0
 *		Jabber:	segfault@dione.zcu.cz
 ---------------------------------------------------------------------------*/
#ifndef VTP3COMMONHEADER_H_
#define VTP3COMMONHEADER_H_

#include <ostream>
#include <cstdint>

#define MAX_DOMAIN_LENGTH 32

namespace VTP3
{
    struct VTP3CommonHeader
    {
        uint8_t version;
        uint8_t code;
        union
        {
            uint8_t followers;
            uint8_t sequence_nr;
            uint8_t reserved;
        };
        uint8_t domain_len;
        uint8_t domain_name[MAX_DOMAIN_LENGTH];
    };

    std::ostream& operator<<(std::ostream& os, VTP3CommonHeader const& vch);

} /* namespace VTP3 */

#endif /* VTP3COMMONHEADER_H_ */
