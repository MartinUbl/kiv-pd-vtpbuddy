#ifndef VTPBUDDY_ENCAPSULATION_H
#define VTPBUDDY_ENCAPSULATION_H

#include "EthernetStructs.h"

enum EncapsRealType
{
    ENCAPS_REAL_8023        = 0,    // 802.3 ethernet
    ENCAPS_REAL_DOT1Q       = 1,    // 802.1Q tagged
    ENCAPS_REAL_ISL         = 2,    // Inter-Switch Link
};

/**
 * Base class for every encapsulation class
 */
class Encapsulation
{
    public:
        Encapsulation(EncapsRealType encaps) : m_encaps(encaps) { };

        // appends encapsulation data to buffer; the caller shall preallocate buffer of needed size, for which he asks for using GetEncapsulationSize
        virtual void AppendEncapsData(uint8_t* target) = 0;

        // retrieves size of data to be appended by encapsulation
        virtual size_t GetEncapsulationSize() = 0;

        // sets frame length; most of headers contains frame length field; if not, this is simply ignored
        virtual void SetFrameLength(size_t len) { };

        // sets VLAN ID; some encapsulations may require vlan ID setting; if not, this is simply ignored
        virtual void SetVLANId(uint16_t vlanId) { };

    protected:
        EncapsRealType m_encaps;
};

/**
 * Ethernet (802.3) encapsulation class
 */
class EthernetEncapsulation : public Encapsulation
{
    public:
        EthernetEncapsulation();

        // Sets parameters of encapsulation (header fields)
        void SetParameters(const uint8_t* dst_mac, const uint8_t* src_mac, uint8_t dsap, uint8_t ssap, uint8_t control, const uint8_t* oui, uint16_t protocol_id);

        virtual void AppendEncapsData(uint8_t* target);
        virtual size_t GetEncapsulationSize();
        virtual void SetFrameLength(size_t len);

    protected:
        EthernetHeader m_header;
};

/**
 * 802.1Q encapsulation class (in fact extends Ethernet II scheme)
 */
class Dot1QEncapsulation : public Encapsulation
{
    public:
        Dot1QEncapsulation();

        // Sets parameters of encapsulation (header fields)
        void SetParameters(const uint8_t* dst_mac, const uint8_t* src_mac, uint8_t dsap, uint8_t ssap, uint8_t control, const uint8_t* oui, uint16_t protocol_id, uint16_t tpid, uint16_t pcp, uint16_t dei);

        virtual void AppendEncapsData(uint8_t* target);
        virtual size_t GetEncapsulationSize();
        virtual void SetFrameLength(size_t len);
        virtual void SetVLANId(uint16_t vlan_id);

    protected:
        Dot1QHeader m_header;
};

#endif
