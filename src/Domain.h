#ifndef VTPBUDDY_DOMAIN_H
#define VTPBUDDY_DOMAIN_H

#include "VTPStructs.h"
#include "VLAN.h"

typedef std::map<uint16_t, VLANRecord*> VLANMap;

class VTPDomain
{
    public:
        VTPDomain(const char* name, const char* password = nullptr);

        const char* GetName() const;

        VLANRecord* GetVLANByName(const char* name);
        VLANRecord* GetVLANById(uint16_t id);
        void AddVLAN(uint16_t id, uint8_t type, uint32_t index80210, const char* name);
        void UpdateVLAN(uint16_t id, uint8_t type, uint32_t index80210, const char* name);
        void RemoveVLAN(uint16_t id);

        VLANMap const& GetVLANMap() const;

        void HandleSummaryAdvert(SummaryAdvertPacketBody* pkt, uint8_t followers);
        void HandleSubsetAdvert(SubsetAdvertPacketBody* pkt, uint8_t sequence_nr, size_t frame_limit_offset);
        void HandleAdvertRequest(AdvertRequestPacketBody* pkt);

    protected:
        void _ReduceVLANSet(std::set<uint16_t> const &vlanSet);

    private:
        std::string m_name;
        std::string m_password;
        uint32_t m_currentRevision;
        uint32_t m_lastSubsetSequenceNumber;

        VLANMap m_vlans;
};

#endif
