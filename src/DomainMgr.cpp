#include "general.h"
#include "DomainMgr.h"
#include "Config.h"

DomainMgr::DomainMgr()
{
    m_running = false;
}

bool DomainMgr::Init()
{
    m_running = true;
    m_nextSummary = time(nullptr) + sConfig->GetConfigIntValue(CONF_SUMMARY_PERIOD);
    m_summaryThread = new std::thread(&DomainMgr::_RunSummaryUpdater, this);
    if (!m_summaryThread)
    {
        m_running = false;
        return false;
    }

    return true;
}

void DomainMgr::Finalize()
{
    m_running = false;
    m_summaryCond.notify_all();
    if (m_summaryThread->joinable())
        m_summaryThread->join();
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

void DomainMgr::_RunSummaryUpdater()
{
    while (m_running)
    {
        std::unique_lock<std::mutex> lck(m_summaryMtx);

        // use condition variable for interruptable sleep
        while (time(nullptr) < m_nextSummary && m_running)
            m_summaryCond.wait_for(lck, std::chrono::seconds(m_nextSummary - time(nullptr)));

        if (!m_running)
            break;

        // send "empty" summary adverts by every domain
        for (std::pair<std::string, VTPDomain*> dompair : m_domainMap)
            dompair.second->SendSummaryAdvert(0);

        m_nextSummary = time(nullptr) + sConfig->GetConfigIntValue(CONF_SUMMARY_PERIOD);
    }
}
