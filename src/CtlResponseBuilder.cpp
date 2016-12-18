#include "general.h"
#include "Config.h"
#include "Domain.h"
#include "DomainMgr.h"
#include "VLAN.h"

namespace CtlResponseBuilder
{

    std::string GetOverallStatus()
    {
        DomainMap const& dmap = sDomainMgr->GetDomainMap();

        size_t vlancount = 0;
        for (std::pair<std::string, VTPDomain*> pr : dmap)
            vlancount += pr.second->GetVLANMap().size();

        std::string statstr = "";

        statstr += "Domains: " + std::to_string(dmap.size()) + "\n";
        statstr += "VLANs: " + std::to_string(vlancount);

        return statstr;
    }

    std::string ListDomains()
    {
        std::string liststr = "";

        DomainMap const& dmap = sDomainMgr->GetDomainMap();

        size_t vlancount = 0;
        bool first = true;
        for (std::pair<std::string, VTPDomain*> pr : dmap)
        {
            liststr += (first ? "" : "\n") + pr.first;
            first = false;
        }

        return liststr;
    }

    std::string ListVLANs(const char* domain)
    {
        VTPDomain* dom = nullptr;

        DomainMap const& dmap = sDomainMgr->GetDomainMap();
        if (domain == nullptr)
        {
            if (dmap.size() == 1)
                dom = (dmap.begin())->second;
            else
                return "ERR:MISSINGPARAM";
        }
        else
        {
            auto itr = dmap.find(domain);
            if (itr == dmap.end())
                return "ERR:NOTFOUND";
            dom = itr->second;
        }

        std::string liststr = "";

        VLANMap const& vlanMap = dom->GetVLANMap();

        bool first = true;
        for (std::pair<uint16_t, VLANRecord*> vlp : vlanMap)
        {
            liststr += (first ? "" : "\n") + std::to_string(vlp.first) + " " + vlp.second->name;
            first = false;
        }

        return liststr;
    }

}
