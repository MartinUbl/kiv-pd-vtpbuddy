#include "general.h"
#include "Enums.h"
#include "Domain.h"
#include "VLAN.h"
#include "Network.h"
#include "VTPStructs.h"
#include "Config.h"
#include "ConfigurationGenerator.h"
#include "ConfigurationLoader.h"
#include "RuntimeGlobals.h"
#include "Shared.h"
#include "Versioning.h"
#include <arpa/inet.h>

#include <iomanip>
#include <openssl/md5.h>

VTPDomain::VTPDomain(const char* name, const char* password)
{
    m_name = name;
    m_password = password ? password : "";
    m_currentRevision = 0;
    m_lastSubsetSequenceNumber = 1;

    memset(m_lastChecksum, 0, 16);
    memset(m_lastUpdateTimestamp, 0, 12);
    memset(m_updaterIdentity, 0, 4);
}

void VTPDomain::Startup()
{
    // attempt to load from file - if available
    LoadFromFile();

    // if we are in client mode, let's send advertisement request so the other devices would send us their configuration
    if (sConfig->GetConfigIntValue(CONF_MODE) == OM_CLIENT)
        SendAdvertRequest(m_currentRevision);
    SendSummaryAdvert(0);
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
        RemoveVLAN(vlanId);
}

void VTPDomain::HandleSummaryAdvert(SummaryAdvertPacketBody* pkt, uint8_t followers)
{
    uint8_t myDigest[VTP_MD5_LENGTH + 1];
    // TODO: figure out MD5 digest rules for summary advertisement frame
    // this is wrong, VTP node appends domain, password, version, ... into hash
    //MD5((unsigned char*)m_password.c_str(), m_password.length(), m_passwordHash);

    // if received revision is higher than our revision..
    if (pkt->revision > m_currentRevision)
    {
        // if there's no subset advert following this packet, ask for it using advert request
        if (followers == 0)
            SendAdvertRequest(pkt->revision);
        else
        {
            // summary advert always comes right before subset advert(s), so we can save MD5 digest here

            memcpy(m_lastChecksum, pkt->md5_digest, 16);
            memcpy(m_lastUpdateTimestamp, pkt->update_timestamp, 12);
            memcpy(m_updaterIdentity, &pkt->updater_id, 4);
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
                vlan = GetVLANById(cur->isl_vlan_id);
            }
            else
                UpdateVLAN(cur->isl_vlan_id, cur->type, cur->status, cur->mtu_size, cur->index80210, name.c_str());

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

        // commit and push changes
        // TODO: delegate push to a thread, synchronize with further pushes
        VersioningBase* vers = sRuntimeGlobals->GetVersioningTool();
        vers->Commit();
        vers->Push();
    }
}

void VTPDomain::HandleAdvertRequest(AdvertRequestPacketBody* pkt)
{
    if (m_currentRevision >= pkt->start_revision)
    {
        // prepare subset adverts, so we would know, how many of them will follow after summary advert
        std::vector<PreparedPacket*> subsetAdverts;
        PrepareAllSubsetAdverts(subsetAdverts);

        SendSummaryAdvert(subsetAdverts.size());

        SendPreparedSubsetAdverts(subsetAdverts);
    }
}

void VTPDomain::SendSummaryAdvert(uint8_t followers)
{
    PreparedPacket* reqpkt = PreparedPacket::Prepare(VTP_MSG_SUMMARY_ADVERT, sizeof(SummaryAdvertPacketBody) + 8, GetName());

    SummaryAdvertPacketBody* bd = (SummaryAdvertPacketBody*)reqpkt->data;
    bd->revision = m_currentRevision;
    memcpy(bd->md5_digest, m_lastChecksum, 16);
    memcpy(bd->update_timestamp, m_lastUpdateTimestamp, 12);
    memcpy(&bd->updater_id, m_updaterIdentity, 4);

    // TODO: figure out, what is meant by following fields present in Summary Advert frames sent by Cisco devices
    // this dump appears to be same no matter what changes are made to VLAN settings; may be related to interface
    // configuration, STP configuration, etc.
    // begin of mysterious byte dump
    uint8_t* trgt = reqpkt->data + sizeof(SummaryAdvertPacketBody);
    trgt[0] = 0;
    trgt[1] = 0;
    trgt[2] = 0;
    trgt[3] = 1;
    trgt[4] = 6;
    trgt[5] = 1;
    trgt[6] = 0;
    trgt[7] = 2;
    // end of mysterious byte dump

    reqpkt->header.followers = followers;

    ToNetworkEndianity(bd);
    sNetwork->SendPreparedPacket(reqpkt);
}

void VTPDomain::SendAdvertRequest(uint32_t start_revision)
{
    PreparedPacket* reqpkt = PreparedPacket::Prepare(VTP_MSG_ADVERT_REQUEST, sizeof(AdvertRequestPacketBody), GetName());

    AdvertRequestPacketBody* bd = (AdvertRequestPacketBody*)reqpkt->data;
    bd->start_revision = start_revision;

    ToNetworkEndianity(bd);
    sNetwork->SendPreparedPacket(reqpkt);
}

void VTPDomain::PrepareAllSubsetAdverts(std::vector<PreparedPacket*> &target)
{
    const size_t maxLen = 1200; // TODO: find proper limit; configurable?
    const size_t overhead = sizeof(SubsetAdvertPacketBody) - 1;

    if (m_vlans.size() == 0)
        return;

    uint8_t sequence_nr = 1;

    VLANMap::iterator it = m_vlans.begin();
    std::vector<uint8_t> vibBuffer;
    while (it != m_vlans.end())
    {
        std::vector<uint8_t> buffer;

        if (!vibBuffer.empty())
        {
            buffer.insert(buffer.end(), vibBuffer.begin(), vibBuffer.end());
            vibBuffer.clear();
        }

        while (buffer.size() <= maxLen + overhead && it != m_vlans.end())
        {
            FillVLANInfoBlock(it->second, vibBuffer);

            it++;

            if (buffer.size() + vibBuffer.size() > maxLen + overhead)
                break;

            buffer.insert(buffer.end(), vibBuffer.begin(), vibBuffer.end());
            vibBuffer.clear();
        }

        PreparedPacket* reqpkt = PreparedPacket::Prepare(VTP_MSG_SUBSET_ADVERT, buffer.size() + overhead, GetName());

        reqpkt->header.sequence_nr = (sequence_nr++);

        SubsetAdvertPacketBody* bd = (SubsetAdvertPacketBody*)reqpkt->data;
        bd->revision = m_currentRevision;
        memcpy(&bd->data, &buffer[0], buffer.size());
        ToNetworkEndianity(bd);
        target.push_back(reqpkt);

        buffer.clear();
    }
}

void VTPDomain::SendPreparedSubsetAdverts(std::vector<PreparedPacket*> &target)
{
    for (PreparedPacket* pkt : target)
        sNetwork->SendPreparedPacket(pkt);
    target.clear();
}

#define VEC_ADD_1B(x) { target.push_back(x); }
#define VEC_ADD_2B(x) { target.push_back((x >> 8) & 0xFF); target.push_back(x & 0xFF); }
#define VEC_ADD_4B(x) { target.push_back((x >> 24) & 0xFF); target.push_back((x >> 16) & 0xFF); target.push_back((x >> 8) & 0xFF); target.push_back(x & 0xFF); }

void VTPDomain::FillVLANInfoBlock(VLANRecord* vlan, std::vector<uint8_t> &target)
{
    size_t i;

    // size placeholder
    VEC_ADD_1B(0);

    VEC_ADD_1B(vlan->status);
    VEC_ADD_1B(vlan->type);
    VEC_ADD_1B(vlan->name.size());

    VEC_ADD_2B(vlan->id);
    VEC_ADD_2B(vlan->mtu);

    VEC_ADD_4B(vlan->index80210);

    // insert name
    for (i = 0; i < vlan->name.size(); i++)
        VEC_ADD_1B(vlan->name[i]);

    // pad to multiple of 4
    if ((vlan->name.size() % 4) != 0)
    {
        size_t rem = 3 - (vlan->name.size() % 4);
        for (i = 0; i <= rem; i++)
            VEC_ADD_1B(0);
    }

    // append all features
    for (std::pair<uint16_t, uint16_t> ftr : vlan->features)
    {
        VEC_ADD_1B(ftr.first);
        VEC_ADD_1B(0x01);           // TODO: verify this value for sure, it appears to be constant and correct
        VEC_ADD_2B(ftr.second);
    }

    target[0] = target.size();
}

void VTPDomain::SaveToFile()
{
    std::string datapath = SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION));

    bool exists = true;

    struct stat info;
    if (stat(datapath.c_str(), &info) != 0)
        exists = false;
    else if (!(info.st_mode & S_IFDIR))
        exists = false;

    if (!exists)
    {
        if (mkdir(datapath.c_str(), 0777) != 0)
        {
            std::cerr << "Could not open or create data directory!" << std::endl;
            return;
        }
    }
    
    // ofstream scope
    {
        std::ofstream of(datapath + m_name);
        if (!of.is_open())
        {
            std::cerr << "Could not open data file for domain " << m_name.c_str() << "!" << std::endl;
            return;
        }

        ConfigurationGenerator* cfgen = sRuntimeGlobals->GetConfigurationGenerator();

        // append metadata to output file

        of << "#!META:CONFTYPE=" << std::to_string(cfgen->GetType()) << std::endl;
        of << "#!META:DOMAIN=" << m_name.c_str() << std::endl;
        of << "#!META:REVISION=" << std::to_string(m_currentRevision) << std::endl;
        of << "#!META:TIMESTAMP=" << std::string((const char*)m_lastUpdateTimestamp, (size_t)12) << std::endl;
        of << "#!META:UPDATER=" << std::to_string(m_updaterIdentity[3]) << "." << std::to_string(m_updaterIdentity[2]) << "." << std::to_string(m_updaterIdentity[1]) << "." << std::to_string(m_updaterIdentity[0]) << std::endl;
        of << "#!META:CHECKSUM=";
        {
            std::ios::fmtflags f(of.flags());

            of << std::hex << std::setfill('0') << std::setw(2);
            for (size_t i = 0; i < 16; i++)
                of << (uint32_t)m_lastChecksum[i];

            of.flags(f);
        }
        of << std::endl;

        of << std::endl; // the header is always delimited by empty line
        
        VLANRecord* rec;
        for (auto vl : m_vlans)
        {
            rec = vl.second;

            // wrap VLAN configuration to starting and ending tags

            of << "#!VLAN:START" << std::endl;
            _SaveVLANToFile(of, cfgen, rec);
            of << "#!VLAN:END" << std::endl;

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
    std::string datapath = SanitizePath(sConfig->GetConfigStringValue(CONF_DATA_LOCATION));

    // ifstream scope
    {
        std::ifstream ifs(datapath + m_name);
        if (!ifs.is_open())
        {
            // no domain data stored yet
            return;
        }

        std::string line;
        std::string metaval, metaname;
        size_t confType = 0xFFFF;

        size_t metadataBitmap = 0;

        // at first, load metadata

        while (ifs.good())
        {
            std::getline(ifs, line);
            line = str_trim(line);

            // empty line delimits header
            if (line.length() == 0)
                break;

            if (line.substr(0, 7) == "#!META:")
            {
                metaval = line.substr(7);
                size_t eqpos = metaval.find_first_of('=');
                if (eqpos == std::string::npos)
                {
                    std::cerr << "Invalid meta definition in domain file: " << metaval.c_str() << std::endl;
                    continue;
                }

                metaname = metaval.substr(0, eqpos);
                metaval = metaval.substr(eqpos + 1);

                try
                {
                    if (metaname == "DOMAIN")
                    {
                        if (m_name != metaval)
                            std::cerr << "Domain name mismatch, file named '" << m_name.c_str() << "' contains domain data for '" << metaval.c_str() << "'";

                        metadataBitmap |= (1 << 0);
                    }
                    else if (metaname == "REVISION")
                    {
                        m_currentRevision = parseLong_ex(metaval.c_str());

                        metadataBitmap |= (1 << 1);
                    }
                    else if (metaname == "UPDATER")
                    {
                        parseIPv4_ex(metaval.c_str(), m_updaterIdentity);

                        metadataBitmap |= (1 << 2);
                    }
                    else if (metaname == "CHECKSUM")
                    {
                        if (metaval.length() < 16)
                            throw std::invalid_argument(metaval);

                        size_t tg = 0;
                        for (size_t il = 0; il < 16; il++)
                            m_lastChecksum[il] = (uint8_t)std::stoll(metaval.substr(il*2, 2), &tg, 16);

                        metadataBitmap |= (1 << 3);
                    }
                    else if (metaname == "TIMESTAMP")
                    {
                        if (metaval.length() < 12)
                            throw std::invalid_argument(metaval);
                        memcpy(m_lastUpdateTimestamp, metaval.c_str(), 12);

                        metadataBitmap |= (1 << 4);
                    }
                    else if (metaname == "CONFTYPE")
                    {
                        confType = parseLong_ex(metaval.c_str());

                        metadataBitmap |= (1 << 5);
                    }
                }
                catch (std::invalid_argument &ex)
                {
                    std::cerr << "Invalid meta " << metaname.c_str() << " value: " << metaval.c_str() << std::endl;
                }
            }
            else
            {
                std::cerr << "Invalid line in header file: " << line.c_str() << std::endl;
            }
        }

        // just integrity check - 1 << 6 is first unused bit, -1 due to check for all lower bits to be ON
        // this is probably not critical
        if (metadataBitmap != ((1 << 6) - 1))
            std::cerr << "Possible metadata corruption (bmap " << metadataBitmap << ", expected " << ((1 << 6) - 1) << "), configuration may not be loaded properly" << std::endl;

        VLANRecord* rec;

        ConfigurationLoader* cldr = nullptr;

        // use loader mentioned in metadata
        switch (confType)
        {
            case CONFIGURATION_TYPE_CONFIG:
                cldr = new CMConfigurationLoader();
                break;
            case CONFIGURATION_TYPE_VLANDB:
                cldr = new VDConfigurationLoader();
                break;
            default:
                std::cerr << "Unknown configuration type detected (" << confType << "), could not load domain configuration" << std::endl;
                return;
        }

        rec = new VLANRecord;
        while (cldr->LoadVLAN(ifs, rec))
        {
            // do not store incorrect records
            if (rec->id == 0 || rec->name.length() == 0)
            {
                rec->ResetRecordState();
                continue;
            }

            m_vlans[rec->id] = rec;

            rec = new VLANRecord;
        }
        delete rec; // one record is always allocated excessively
    }
}
