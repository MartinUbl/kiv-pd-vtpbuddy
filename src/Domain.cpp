#include "general.h"
#include "Domain.h"
#include "VLAN.h"

VTPDomain::VTPDomain(const char* name, const char* password)
{
    m_name = name;
    m_password = password ? password : "";
    m_currentRevision = 0;
}

VLANRecord* VTPDomain::GetVLANByName(const char* name)
{
    for (auto it : m_vlans)
    {
        if (it.second->name == name)
            return it.second;
    }

    return nullptr;
}

VLANRecord* VTPDomain::GetVLANById(uint16_t id)
{
    auto it = m_vlans.find(id);
    if (it == m_vlans.end())
        return nullptr;

    return it->second;
}

void VTPDomain::AddVLAN(uint16_t id, uint8_t type, uint32_t index80210, const char* name)
{
    if (GetVLANById(id) || GetVLANByName(name))
        return;

    m_vlans[id] = new VLANRecord(type, id, index80210, name);
}

void VTPDomain::UpdateVLAN(uint16_t id, uint8_t type, uint32_t index80210, const char* name)
{
    VLANRecord* vlan = GetVLANById(id);
    if (!vlan)
        return;

    vlan->type = type;
    vlan->index80210 = index80210;
    vlan->name = name;
}

void VTPDomain::RemoveVLAN(uint16_t id)
{
    delete m_vlans[id];
    m_vlans.erase(id);
}

void VTPDomain::_ReduceVLANSet(std::set<uint16_t> const &vlanSet)
{
    std::set<uint16_t> toRemove;

    for (auto it : m_vlans)
    {
        if (vlanSet.find(it.first) == vlanSet.end())
            toRemove.insert(it.first);
    }

    for (uint16_t vlanId : toRemove)
        RemoveVLAN(vlanId);
}

void VTPDomain::HandleSummaryAdvert(SummaryAdvertPacketBody* pkt, uint8_t followers)
{
    if (pkt->revision > m_currentRevision)
    {
        //
    }
}

void VTPDomain::HandleSubsetAdvert(SubsetAdvertPacketBody* pkt, uint8_t sequence_nr, size_t frame_limit_offset)
{
    if (pkt->revision > m_currentRevision)
    {
        SubsetVLANInfoBody* cur;
        std::string name;
        std::vector<SubsetVLANInfoBody*> readVlans;
        std::set<uint16_t> vlanIds;

        size_t offset = 0;
        while (offset < frame_limit_offset)
        {
            cur = (SubsetVLANInfoBody*)((&pkt->data) + offset);
            ToHostEndianity(cur);

            name = std::string((const char*)&cur->data, (size_t)cur->name_length);
            std::cout << "VLAN: " << name.c_str() << "(" << cur->isl_vlan_id << ")" << std::endl;

            vlanIds.insert(cur->isl_vlan_id);

            offset += cur->length;
        }
    }
}

void VTPDomain::HandleAdvertRequest(AdvertRequestPacketBody* pkt)
{
    //
}
