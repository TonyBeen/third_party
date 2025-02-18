/*************************************************************************
    > File Name: kcp_timer.cpp
    > Author: hsz
    > Brief:
    > Created Time: Thu 07 Jul 2022 09:15:46 AM CST
 ************************************************************************/

#include "uring_timer.h"

#include <assert.h>
#include <atomic>
#include <chrono>

#include <log/log.h>

#define LOG_TAG "UringTimer"

std::atomic<uint64_t>   gUniqueIdCount{0};

int32_t TimerCompare(const heap_node_t *lhs, const heap_node_t *rhs)
{
    heap_timer_t *pTimerLeft = HEAP_NODE_TO_TIMER(lhs);
    heap_timer_t *pTimerRight = HEAP_NODE_TO_TIMER(rhs);

    return pTimerLeft->next_timeout < pTimerRight->next_timeout;
}

UringTimer::UringTimer() :
    m_timerCallback(nullptr)
{
    m_timerCtx.unique_id = ++gUniqueIdCount;
    m_timerCtx.user_data = this;
}

UringTimer::UringTimer(uint64_t ms, CallBack cb, uint32_t recycle) :
    m_timerCallback(cb)
{
    m_timerCtx.next_timeout = CurrentTime() + ms;
    m_timerCtx.recycle_time = recycle;
    m_timerCtx.unique_id = ++gUniqueIdCount;
    m_timerCtx.user_data = this;
}

UringTimer::~UringTimer()
{
    m_timerCtx.user_data = nullptr;
}

void UringTimer::update()
{
    if (m_timerCtx.recycle_time) {
        m_timerCtx.next_timeout += m_timerCtx.recycle_time;
    }
}

uint64_t UringTimer::CurrentTime()
{
    std::chrono::steady_clock::time_point tm = std::chrono::steady_clock::now();
    std::chrono::milliseconds mills = 
        std::chrono::duration_cast<std::chrono::milliseconds>(tm.time_since_epoch());
    return mills.count();
}

UringTimerManager::UringTimerManager()
{
    heap_init(&m_timerHeap, TimerCompare);
}

UringTimerManager::~UringTimerManager()
{
    heap_init(&m_timerHeap, TimerCompare);
    m_timerSet.clear();
}

uint64_t UringTimerManager::getNearTimeout()
{
    uint64_t nowMs = UringTimer::CurrentTime();

    if (m_timerHeap.root == nullptr) {
        return UINT64_MAX;
    }

    heap_timer_t *pTimer = HEAP_NODE_TO_TIMER(m_timerHeap.root);
    if (nowMs >= pTimer->next_timeout) {
        return 0;
    }

    return pTimer->next_timeout - nowMs;
}

UringTimer::SP UringTimerManager::addTimer(uint64_t ms, UringTimer::CallBack cb, uint32_t recycle)
{
    UringTimer::SP timer(new UringTimer(ms, cb, recycle));
    return addTimer(timer);
}

static void onTimer(std::weak_ptr<void> cond, std::function<void()> cb)
{
    std::shared_ptr<void> temp = cond.lock();
    if (temp) {
        cb();
    }
}

UringTimer::SP UringTimerManager::addConditionTimer(uint64_t ms, UringTimer::CallBack cb, std::weak_ptr<void> cond, uint32_t recycle)
{
    return addTimer(ms, std::bind(&onTimer, cond, cb), recycle);
}

void UringTimerManager::delTimer(uint64_t timerId)
{
    LOGD("delTimer(%lu)", timerId);
    bool isRootNode = false;
    {
        for (auto it = m_timerSet.begin(); it != m_timerSet.end(); ++it) {
            if ((*it)->getUniqueId() == timerId) {
                heap_node_t *pNode = &(*it)->m_timerCtx.node;
                if (pNode == m_timerHeap.root) {
                    isRootNode = true;
                }
                heap_remove(&m_timerHeap, pNode);
                m_timerSet.erase(it);
                break;
            }
        }
    }

    if (isRootNode) {
        onTimerInsertedAtFront();
    }
}

void UringTimerManager::listExpiredTimer(std::list<UringTimer::CallBack> &cbList)
{
    uint64_t nowMS = UringTimer::CurrentTime();

    heap_timer_t *pTimer = nullptr;
    while (m_timerHeap.root != nullptr) {
        pTimer = HEAP_NODE_TO_TIMER(m_timerHeap.root);
        if (pTimer->next_timeout > nowMS) {
            break;
        }

        heap_dequeue(&m_timerHeap);
        UringTimer *pKTimer = static_cast<UringTimer *>(pTimer->user_data);
        cbList.push_back(pKTimer->m_timerCallback);
        if (pTimer->recycle_time > 0) {
            pKTimer->update();
            heap_insert(&m_timerHeap, pKTimer->getHeapNode());
        } else {
            // 从集合中移除
            m_timerSet.erase(pKTimer->shared_from_this());
        }
    }
}

UringTimer::SP UringTimerManager::addTimer(UringTimer::SP timer)
{
    if (timer == nullptr) {
        return nullptr;
    }

    bool atFront = false;
    LOGD("addTimer(%p) %lu", timer.get(), timer->getUniqueId());
    {
        m_timerSet.insert(timer);
        heap_insert(&m_timerHeap, timer->getHeapNode());
        atFront = (m_timerHeap.root == timer->getHeapNode());
    }

    if (atFront) {
        onTimerInsertedAtFront();
    }

    return timer;
}
