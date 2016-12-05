#include "general.h"
#include "Enums.h"
#include "Domain.h"
#include "VLAN.h"
#include "Network.h"
#include "VTPStructs.h"
#include "Config.h"
#include "ConfigurationGenerator.h"
#include "RuntimeGlobals.h"
#include <arpa/inet.h>

#include <openssl/md5.h>

VTPDomain::VTPDomain(const char* name, const char* password)
{
    m_name = name;
    m_password = password ? password : "";
    m_currentRevision = 0;
    m_lastSubsetSequenceNumber = 1;
}

void VTPDomain::Startup()
{
    if (sConfig->GetConfigIntValue(CONF_MODE) == OM_CLIENT)
        SendAdvertRequest(0);
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

void VTPDomain::AddVLAN(uint16_t id, uint8_t type, uint8_t status, uint16_t mtu, uint32_t index80210, const char* name)
{
    if (GetVLANById(id) || GetVLANByName(name))
        return;

    m_vlans[id] = new VLANRecord(type, status, id, mtu, index80210, name);
}

void VTPDomain::UpdateVLAN(uint16_t id, uint8_t type, uint8_t status, uint16_t mtu, uint32_t index80210, const char* name)
{
    VLANRecord* vlan = GetVLANById(id);
    if (!vlan)
        return;

    vlan->type = type;
    vlan->status = status;
    vlan->index80210 = index80210;
    vlan->mtu = mtu;
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

    // find VLANs to be removed
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
    for (int i = 0; i < 16; i++)
        printf("%.2X ", pkt->md5_digest[i]);
    std::cout << std::endl;

    uint8_t myDigest[VTP_MD5_LENGTH + 1];
    // this is wrong, VTP node appends domain, password, version, ... into hash
    // TODO: figure it out
    //MD5((unsigned char*)m_password.c_str(), m_password.length(), m_passwordHash);

    // if received revision is higher than our revision..
    if (pkt->revision > m_currentRevision)
    {
        // if there's no subset advert following this packet, ask for it using advert request
        if (followers == 0)
        {
            SendAdvertRequest(pkt->revision);
        }
    }
}

void VTPDomain::HandleSubsetAdvert(SubsetAdvertPacketBody* pkt, uint8_t sequence_nr, size_t frame_limit_offset)
{
    // if received revision is higher than our revision, read contents and update our VLAN database
    if (pkt->revision > m_currentRevision)
    {
        SubsetVLANInfoBody* cur;
        VLANRecord* vlan;
        std::string name;
        std::vector<SubsetVLANInfoBody*> readVlans;
        std::set<uint16_t> vlanIds;

        // iterate while we are still within limits
        size_t offset = 0;
        while (offset < frame_limit_offset)
        {
            // "cut" next chunk and convert to host endianity
            cur = (SubsetVLANInfoBody*)((&pkt->data) + offset);
            ToHostEndianity(cur);

            // extract VLAN name
            name = std::string((const char*)&cur->data, (size_t)cur->name_length);

            int name_padded = ((int)(cur->name_length / 4) + (cur->name_length%4 ? 1 : 0)) * 4;

            vlanIds.insert(cur->isl_vlan_id);

            // add or update VLAN
            vlan = GetVLANById(cur->isl_vlan_id);
            if (!vlan)
            {
                AddVLAN(cur->isl_vlan_id, cur->type, cur->status, cur->mtu_size, cur->index80210, name.c_str());
                std::cout << "Adding VLAN: " << name.c_str() << " (" << cur->isl_vlan_id << ")" << std::endl;

                vlan = GetVLANById(cur->isl_vlan_id);
            }
            else
            {
                UpdateVLAN(cur->isl_vlan_id, cur->type, cur->status, cur->mtu_size, cur->index80210, name.c_str());
                std::cout << "Updating VLAN: " << name.c_str() << " (" << vlan->name.c_str() << " - " << cur->isl_vlan_id << ")" << std::endl;
            }

            vlan->features.clear();
            for (size_t g = offset + sizeof(SubsetVLANInfoBody) - 1 + name_padded; g < offset + cur->length; g += 4)
            {
                uint8_t col_id = (*(&pkt->data + g));
                uint8_t par = (*(&pkt->data + g + 1));
                uint16_t val = ntohs((*(uint16_t*)(&pkt->data + g + 2)));

                if (col_id == VLAN_FEAT_NONE || col_id >= MAX_VLAN_FEATURE)
                    std::cerr << "Unknown VLAN feature " << (int)col_id << " (par: " << (int)par << ") with value " << (int)val << " received" << std::endl;
                else
                    vlan->features[col_id] = val;
            }

            offset += cur->length;
        }

        // remove VLANs not present in subset advert packet
        _ReduceVLANSet(vlanIds);

        // update local revision number
        m_currentRevision = pkt->revision;

        // save to file now
        // TODO: make this periodic?
        SaveToFile();
    }
}

void VTPDomain::HandleAdvertRequest(AdvertRequestPacketBody* pkt)
{
    //
}

void VTPDomain::SendAdvertRequest(uint32_t start_revision)
{
    std::cout << "Sending advert request" << std::endl;

    PreparedPacket* reqpkt = PreparedPacket::Prepare(VTP_MSG_ADVERT_REQUEST, sizeof(AdvertRequestPacketBody), GetName());

    AdvertRequestPacketBody* bd = (AdvertRequestPacketBody*)reqpkt->data;
    bd->start_revision = start_revision;

    ToNetworkEndianity(bd);
    sNetwork->SendPreparedPacket(reqpkt);
}

void VTPDomain::SaveToFile()
{
    bool exists = true;
    // TODO: include system paths
    struct stat info;
    if (stat("data", &info) != 0)
        exists = false;
    else if (!(info.st_mode & S_IFDIR))
        exists = false;
        
    if (!exists)
    {
        if (mkdir("data", 0777) != 0)
        {
            std::cerr << "Could not open or create data directory!" << std::endl;
            return;
        }
    }
    
    // ofstream scope
    {
        std::ofstream of("data/" + m_name);
        if (!of.is_open())
        {
            std::cerr << "Could not open data file for domain " << m_name.c_str() << "!" << std::endl;
            return;
        }

        of << "# Configuration for domain " << m_name.c_str() << std::endl;

        of << std::endl;

        of << "#!META:DOMAIN=" << m_name.c_str() << std::endl;
        of << "#!META:REVISION=" << std::to_string(m_currentRevision) << std::endl;

        of << std::endl;
        
        VLANRecord* rec;
        ConfigurationGenerator* cfgen = sRuntimeGlobals->GetConfigurationGenerator();
        for (auto vl : m_vlans)
        {
            rec = vl.second;
            
            _SaveVLANToFile(of, cfgen, rec);

            of << std::endl;
        }
    }
}

void VTPDomain::_SaveVLANToFile(std::ofstream &ofs, ConfigurationGenerator* generator, VLANRecord* vlan)
{
    std::string trg;

    // Append mandatory info

    if (!generator->BlockStart(trg, vlan->id, vlan->name.c_str()))
        return;
    ofs << trg << std::endl;

    if (!generator->VLANType(trg, vlan->type))
        return;
    ofs << trg << std::endl;

    if (!generator->ID80210(trg, vlan->index80210))
        return;
    ofs << trg << std::endl;

    if (!generator->VLANState(trg, vlan->status))
        return;
    ofs << trg << std::endl;

    if (!generator->MTUSize(trg, vlan->mtu))
        return;
    ofs << trg << std::endl;

    // Append features, if present

    if (vlan->features.find(VLAN_FEAT_RING_NO) != vlan->features.end())
    {
        if (!generator->RingNumber(trg, vlan->features[VLAN_FEAT_RING_NO]))
            return;
        ofs << trg << std::endl;
    }

    if (vlan->features.find(VLAN_FEAT_BRIDGE_NO) != vlan->features.end())
    {
        if (!generator->BridgeNumber(trg, vlan->features[VLAN_FEAT_BRIDGE_NO]))
            return;
        ofs << trg << std::endl;
    }

    if (vlan->features.find(VLAN_FEAT_STP) != vlan->features.end())
    {
        if (!generator->STPMode(trg, vlan->features[VLAN_FEAT_STP]))
            return;
        ofs << trg << std::endl;
    }

    if (vlan->features.find(VLAN_FEAT_PARENT) != vlan->features.end())
    {
        if (!generator->ParentVLAN(trg, vlan->features[VLAN_FEAT_PARENT]))
            return;
        ofs << trg << std::endl;
    }

    if (vlan->features.find(VLAN_FEAT_TRANS1) != vlan->features.end())
    {
        if (!generator->VLANTrans1(trg, vlan->features[VLAN_FEAT_TRANS1]))
            return;
        ofs << trg << std::endl;
    }

    if (vlan->features.find(VLAN_FEAT_TRANS2) != vlan->features.end())
    {
        if (!generator->VLANTrans2(trg, vlan->features[VLAN_FEAT_TRANS2]))
            return;
        ofs << trg << std::endl;
    }

    if (vlan->features.find(VLAN_FEAT_BRIDGE_MODE) != vlan->features.end())
    {
        if (!generator->BridgeMode(trg, vlan->features[VLAN_FEAT_BRIDGE_MODE]))
            return;
        ofs << trg << std::endl;
    }

    if (vlan->features.find(VLAN_FEAT_ARE_HOPS) != vlan->features.end())
    {
        if (!generator->AREHops(trg, vlan->features[VLAN_FEAT_ARE_HOPS]))
            return;
        ofs << trg << std::endl;
    }

    if (vlan->features.find(VLAN_FEAT_STE_HOPS) != vlan->features.end())
    {
        if (!generator->STEHops(trg, vlan->features[VLAN_FEAT_STE_HOPS]))
            return;
        ofs << trg << std::endl;
    }
}

void VTPDomain::LoadFromFile()
{
    //
}
