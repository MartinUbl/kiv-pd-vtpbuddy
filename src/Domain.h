#ifndef VTPBUDDY_DOMAIN_H
#define VTPBUDDY_DOMAIN_H

#include "VTPStructs.h"
#include "VLAN.h"

typedef std::map<uint16_t, VLANRecord*> VLANMap;

/**
 * VTP Domain container class with related methods
 */
class VTPDomain
{
    public:
        // constructor retaining mandatory parameter(s)
        VTPDomain(const char* name, const char* password = nullptr);

        // retrieves domain name
        const char* GetName() const;

        // finds and returns VLAN by its name
        VLANRecord* GetVLANByName(const char* name);
        // finds and returns VLAN by its ID
        VLANRecord* GetVLANById(uint16_t id);
        // adds new VLAN to domain
        void AddVLAN(uint16_t id, uint8_t type, uint32_t index80210, const char* name);
        // updates existing VLAN in domain
        void UpdateVLAN(uint16_t id, uint8_t type, uint32_t index80210, const char* name);
        // removes VLAN from domain
        void RemoveVLAN(uint16_t id);

        // retrieves all VLAN map
        VLANMap const& GetVLANMap() const;

        // handle incoming summary advertisement packet
        void HandleSummaryAdvert(SummaryAdvertPacketBody* pkt, uint8_t followers);
        // handle incoming subset advertisement packet
        void HandleSubsetAdvert(SubsetAdvertPacketBody* pkt, uint8_t sequence_nr, size_t frame_limit_offset);
        // handle incoming advertisement request packet
        void HandleAdvertRequest(AdvertRequestPacketBody* pkt);

    protected:
        // reduces VLAN set - vlanSet is set of VLANs to be preserved
        void _ReduceVLANSet(std::set<uint16_t> const &vlanSet);

    private:
        // domain name
        std::string m_name;
        // domain password
        std::string m_password;
        // current configuration revision
        uint32_t m_currentRevision;
        // last sequence number of subset advert sent/received
        uint32_t m_lastSubsetSequenceNumber;

        // VLAN maps
        VLANMap m_vlans;
};

#endif
