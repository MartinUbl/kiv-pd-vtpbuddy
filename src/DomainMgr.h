#ifndef VTPBUDDY_DOMAINMGR_H
#define VTPBUDDY_DOMAINMGR_H

#include "Singleton.h"
#include "Domain.h"

typedef std::map<std::string, VTPDomain*> DomainMap;

class DomainMgr
{
    friend class Singleton<DomainMgr>;
    public:
        VTPDomain* GetDomainByName(const char* name);
        VTPDomain* CreateDomain(const char* name, const char* password = nullptr);

        DomainMap const& GetDomainMap() const;

    protected:
        DomainMgr();

    private:
        DomainMap m_domainMap;
};

#define sDomainMgr Singleton<DomainMgr>::getInstance()

#endif
