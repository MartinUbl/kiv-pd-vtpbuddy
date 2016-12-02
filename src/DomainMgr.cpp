#include "general.h"
#include "DomainMgr.h"

DomainMgr::DomainMgr()
{
    //
}

VTPDomain* DomainMgr::GetDomainByName(const char* name)
{
    auto itr = m_domainMap.find(name);
    if (itr == m_domainMap.end())
        return nullptr;

    return itr->second;
}

VTPDomain* DomainMgr::CreateDomain(const char* name, const char* password)
{
    auto itr = m_domainMap.find(name);
    if (itr != m_domainMap.end())
        return nullptr;

    m_domainMap[name] = new VTPDomain(name, password);

    return m_domainMap[name];
}

DomainMap const& DomainMgr::GetDomainMap() const
{
    return m_domainMap;
}
