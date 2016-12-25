#ifndef VTPBUDDY_DELAYWORKTHREAD_H
#define VTPBUDDY_DELAYWORKTHREAD_H

#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>

#include "Singleton.h"

// enumerator of known delayed work types
enum DelayedWorkType
{
    DELAY_WORK_NONE         = 0,
    DELAY_WORK_SAVE_DOMAIN  = 1,
    DELAY_WORK_PUSH         = 2,
    MAX_DELAY_WORK
};

// structure of delayed queue item
struct DelayWorkQueueItem
{
    // work type (see DelayedWorkType enum)
    DelayedWorkType type;
    // VTP domain which requested work
    std::string domain;
    // additional parameters (optional)
    uint8_t* params;
};

/**
 * Delay worker class - maintains delayed queued work
 */
class DelayWorker
{
    friend class Singleton<DelayWorker>;
    public:
        // initializes delay worker
        bool Init();
        // finalizes delay worker
        void Finalize();

        // queues new work
        void QueueRequest(DelayedWorkType type, const char* domain, void* params = nullptr, size_t paramLen = 0);

    protected:
        // private singleton constructor
        DelayWorker();

        // main thread routine
        void _Run();

        // -- Worker delayed routines
        // saves domain to file
        void SaveDomain(DelayWorkQueueItem* wqitem);
        // pushes saved files to repository
        void PushRepository(DelayWorkQueueItem* wqitem);
        // -- End of worker delayed routines

    private:
        // work queue
        std::queue<DelayWorkQueueItem*> m_workQueue;

        // main thread
        std::thread* m_wqThread;

        // queue condition variable
        std::condition_variable m_wqCond;
        // queue mutex
        std::mutex m_wqMutex;

        // is thread running?
        bool m_running;
};

#define sDelayWorker Singleton<DelayWorker>::getInstance()

#endif
