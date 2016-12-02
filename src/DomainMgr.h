#ifndef VTPBUDDY_DOMAINMGR_H
#define VTPBUDDY_DOMAINMGR_H

#include "Singleton.h"
#include "Domain.h"

typedef std::map<std::string, VTPDomain*> DomainMap;

/**
 * Domain manager class
 */
class DomainMgr
{
    friend class Singleton<DomainMgr>;
    public:
        // retrieves stored domain by its name
        VTPDomain* GetDomainByName(const char* name);
        // creates a new domain
        VTPDomain* CreateDomain(const char* name, const char* password = nullptr);

        // retrieves map of all available domains
        DomainMap const& GetDomainMap() const;

    protected:
        // protected singleton constructor
        DomainMgr();

    private:
        // map of all stored domains
        DomainMap m_domainMap;
};

#define sDomainMgr Singleton<DomainMgr>::getInstance()

#endif
