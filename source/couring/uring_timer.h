/*************************************************************************
    > File Name: kcp_timer.h
    > Author: hsz
    > Brief:
    > Created Time: Thu 07 Jul 2022 09:15:42 AM CST
 ************************************************************************/

#ifndef __KCP_TIMER_H__
#define __KCP_TIMER_H__

#include <stdint.h>
#include <set>
#include <list>
#include <memory>
#include <functional>

#include <utils/utils.h>
#include <utils/mutex.h>
#include <utils/singleton.h>

#include "heap_timer.h"

class UringTimer final : public std::enable_shared_from_this<UringTimer>
{
    friend class UringTimerManager;
public:
    typedef std::shared_ptr<UringTimer> SP;
    typedef std::weak_ptr<UringTimer>   WP;
    typedef std::unique_ptr<UringTimer> Ptr;
    typedef std::function<void(void)> CallBack;

    ~UringTimer();
    UringTimer(const UringTimer& timer) = delete;
    UringTimer &operator=(const UringTimer& timer) = delete;

    uint64_t getUniqueId() const { return m_timerCtx.unique_id; }
    void cancel() { m_canceled = true; }

    static uint64_t CurrentTime();

private:
    UringTimer();
    UringTimer(uint64_t ms, CallBack cb, uint32_t recycle);

    heap_node_t *getHeapNode() { return &m_timerCtx.node; }
    uint64_t getTimeout() const { return m_timerCtx.next_timeout; }
    void setNextTime(uint64_t timeMs) { m_timerCtx.next_timeout = timeMs; }
    void setRecycleTime(uint64_t ms) { m_timerCtx.recycle_time = ms; }
    void setCallback(CallBack cb) { m_timerCallback = cb; }
    CallBack getCallback() { return m_timerCallback; }
    void update();

private:
    heap_timer_t        m_timerCtx;         // 定时器信息
    std::atomic<bool>   m_canceled;         // 定时器是否取消
    CallBack            m_timerCallback;    // 定时器回调函数
};

class UringTimerManager
{
    DISALLOW_COPY_AND_ASSIGN(UringTimerManager);
public:
    UringTimerManager();
    virtual ~UringTimerManager();

    uint64_t        getNearTimeout();
    UringTimer::SP  addTimer(uint64_t ms, UringTimer::CallBack cb, uint32_t recycle = 0);
    UringTimer::SP  addConditionTimer(uint64_t ms, UringTimer::CallBack cb, std::weak_ptr<void> cond, uint32_t recycle = 0);
    void            delTimer(uint64_t timerId);

protected:
    void            listExpiredTimer(std::list<UringTimer::CallBack> &cbList);
    UringTimer::SP  addTimer(UringTimer::SP timer);
    virtual void    onTimerInsertedAtFront() = 0;

private:
    heap_t                  m_timerHeap;    // 定时器堆
    std::set<UringTimer::SP>    m_timerSet;     // 定时器集合
};

#endif  // __KCP_TIMER_H__
