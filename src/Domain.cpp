#include "general.h"
#include "Domain.h"
#include "VLAN.h"
#include "Network.h"
#include "VTPStructs.h"

VTPDomain::VTPDomain(const char* name, const char* password)
{
    m_name = name;
    m_password = password ? password : "";
    m_currentRevision = 0;
    m_lastSubsetSequenceNumber = 1;
}

const char* VTPDomain::GetName() const
{
    return m_name.c_str();
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

VLANMap const& VTPDomain::GetVLANMap() const
{
    return m_vlans;
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
    {
        std::cout << "Removing VLAN " << vlanId << std::endl;
        RemoveVLAN(vlanId);
    }
}

void VTPDomain::HandleSummaryAdvert(SummaryAdvertPacketBody* pkt, uint8_t followers)
{
    if (pkt->revision > m_currentRevision)
    {
        // if there's no subset advert following this packet, ask for it using advert request
        if (followers == 0)
        {
            std::cout << "Sending advert request" << std::endl;
            PreparedPacket* reqpkt = PreparedPacket::Prepare(VTP_MSG_ADVERT_REQUEST, sizeof(AdvertRequestPacketBody), GetName());

            AdvertRequestPacketBody* bd = (AdvertRequestPacketBody*)reqpkt->data;
            bd->start_revision = pkt->revision;

            ToNetworkEndianity(bd);
            sNetwork->SendPreparedPacket(reqpkt);
        }
    }
}

void VTPDomain::HandleSubsetAdvert(SubsetAdvertPacketBody* pkt, uint8_t sequence_nr, size_t frame_limit_offset)
{
    if (pkt->revision > m_currentRevision)
    {
        SubsetVLANInfoBody* cur;
        VLANRecord* vlan;
        std::string name;
        std::vector<SubsetVLANInfoBody*> readVlans;
        std::set<uint16_t> vlanIds;

        size_t offset = 0;
        while (offset < frame_limit_offset)
        {
            cur = (SubsetVLANInfoBody*)((&pkt->data) + offset);
            ToHostEndianity(cur);

            name = std::string((const char*)&cur->data, (size_t)cur->name_length);

            vlanIds.insert(cur->isl_vlan_id);

            vlan = GetVLANById(cur->isl_vlan_id);
            if (!vlan)
            {
                AddVLAN(cur->isl_vlan_id, cur->type, cur->index80210, name.c_str());
                std::cout << "Adding VLAN: " << name.c_str() << " (" << cur->isl_vlan_id << ")" << std::endl;
            }
            else
            {
                UpdateVLAN(cur->isl_vlan_id, cur->type, cur->index80210, name.c_str());
                std::cout << "Updating VLAN: " << name.c_str() << " (" << vlan->name.c_str() << " - " << cur->isl_vlan_id << ")" << std::endl;
            }

            offset += cur->length;
        }

        _ReduceVLANSet(vlanIds);

        // update local revision number
        m_currentRevision = pkt->revision;
    }
}

void VTPDomain::HandleAdvertRequest(AdvertRequestPacketBody* pkt)
{
    //
}
