#ifndef VTPBUDDY_DOMAINMGR_H
#define VTPBUDDY_DOMAINMGR_H

#include "Singleton.h"
#include "Domain.h"

class DomainMgr
{
    friend class Singleton<DomainMgr>;
    public:
        VTPDomain* GetDomainByName(const char* name);
        VTPDomain* CreateDomain(const char* name, const char* password = nullptr);

    protected:
        DomainMgr();

    private:
        std::map<std::string, VTPDomain*> m_domainMap;
};

#define sDomainMgr Singleton<DomainMgr>::getInstance()

#endif
