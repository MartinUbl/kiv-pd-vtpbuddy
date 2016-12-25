#ifndef VTPBUDDY_DOMAINMGR_H
#define VTPBUDDY_DOMAINMGR_H

#include "Singleton.h"
#include "Domain.h"

#include <thread>
#include <condition_variable>
#include <mutex>

typedef std::map<std::string, VTPDomain*> DomainMap;

/**
 * Domain manager class
 */
class DomainMgr
{
    friend class Singleton<DomainMgr>;
    public:
        // initializes domain manager
        bool Init();
        // finalizes domain manager
        void Finalize();

        // retrieves stored domain by its name
        VTPDomain* GetDomainByName(const char* name);
        // creates a new domain
        VTPDomain* CreateDomain(const char* name, const char* password = nullptr);

        // retrieves map of all available domains
        DomainMap const& GetDomainMap() const;

    protected:
        // protected singleton constructor
        DomainMgr();

        // thread function for periodic summary advertisement sending
        void _RunSummaryUpdater();

    private:
        // map of all stored domains
        DomainMap m_domainMap;

        // periodic summary thread
        std::thread* m_summaryThread;
        // condition variable used for timing
        std::condition_variable m_summaryCond;
        // summary mutex used
        std::mutex m_summaryMtx;
        // time of next summary advertisement
        time_t m_nextSummary;

        // is summary updater running?
        bool m_running;
};

#define sDomainMgr Singleton<DomainMgr>::getInstance()

#endif
