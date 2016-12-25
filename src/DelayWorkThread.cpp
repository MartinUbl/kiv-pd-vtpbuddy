#include "general.h"
#include "DelayWorkThread.h"
#include "Domain.h"
#include "DomainMgr.h"
#include "RuntimeGlobals.h"
#include "Versioning.h"

DelayWorker::DelayWorker()
{
    m_running = false;
}

bool DelayWorker::Init()
{
    m_running = true;

    m_wqThread = new std::thread(&DelayWorker::_Run, this);
    if (!m_wqThread)
    {
        m_running = false;
        return false;
    }

    return true;
}

void DelayWorker::Finalize()
{
    m_running = false;
    m_wqCond.notify_one();
    if (m_wqThread->joinable())
        m_wqThread->join();
}

void DelayWorker::QueueRequest(DelayedWorkType type, const char* domain, void* params, size_t paramLen)
{
    DelayWorkQueueItem* wqitem = new DelayWorkQueueItem();

    wqitem->type = type;
    wqitem->domain = domain ? domain : "";

    if (!params || paramLen == 0)
        wqitem->params = nullptr;
    else
    {
        wqitem->params = new uint8_t[paramLen];
        memcpy(wqitem->params, params, paramLen);
    }

    std::unique_lock<std::mutex> lck(m_wqMutex);
    m_workQueue.push(wqitem);
    m_wqCond.notify_one();
}

void DelayWorker::_Run()
{
    while (m_running)
    {
        std::unique_lock<std::mutex> lck(m_wqMutex);

        while (m_workQueue.empty() && m_running)
            m_wqCond.wait(lck);

        if (!m_running)
            break;

        DelayWorkQueueItem* wqitem = m_workQueue.front();
        m_workQueue.pop();

        switch (wqitem->type)
        {
            case DELAY_WORK_SAVE_DOMAIN:
                SaveDomain(wqitem);
                break;
            case DELAY_WORK_PUSH:
                PushRepository(wqitem);
                break;
            default:
                break;
        }

        delete wqitem;
        if (wqitem->params)
            delete[] wqitem->params;
    }
}

void DelayWorker::SaveDomain(DelayWorkQueueItem* wqitem)
{
    VTPDomain* dom = sDomainMgr->GetDomainByName(wqitem->domain.c_str());
    if (!dom)
        return;

    dom->SaveToFile();
}

void DelayWorker::PushRepository(DelayWorkQueueItem* wqitem)
{
    VersioningBase* vers = sRuntimeGlobals->GetVersioningTool();
    vers->Commit();
    vers->Push();
}
