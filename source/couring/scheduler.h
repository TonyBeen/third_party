/*************************************************************************
    > File Name: scheduler.h
    > Author: hsz
    > Brief:
    > Created Time: Sun 27 Feb 2022 05:41:28 PM CST
 ************************************************************************/

#ifndef __COURING_SCHEDULER_H__
#define __COURING_SCHEDULER_H__

#include "fiber.h"

#include <memory>
#include <vector>
#include <list>
#include <atomic>

#include <utils/thread.h>
#include <utils/string8.h>
#include <utils/mutex.h>

namespace eular {
class Scheduler
{
public:
    typedef std::shared_ptr<Scheduler> SP;

    /**
     * @brief 协程调度器
     *
     * @param name 调度器名字
     */
    Scheduler(const eular::String8 &name = "");
    virtual ~Scheduler();

    const String8 &name() const { return mName; }
    static Scheduler* GetScheduler();

    void start();
    void stop();

    /**
     * @brief 调度函数
     * 
     * @param fc 协程或回调函数
     * @param th 线程ID
     */
    template<class FiberOrCb>
    void schedule(FiberOrCb fc)
    {
        scheduleNoLock(fc);
    }

    template<class Iterator>
    void schedule(Iterator begin, Iterator end)
    {
        while (begin != end) {
            scheduleNoLock(&*begin);
            ++begin;
        }
    }

private:
    /**
     * @brief 协程信息结构体, 绑定哪个线程
     */
    struct FiberBindThread {
        Fiber::SP fiberPtr;         // 协程智能指针对象
        std::function<void()> cb;   // 协程执行函数

        FiberBindThread() = default;
        FiberBindThread(Fiber::SP sp) : fiberPtr(sp) {}
        FiberBindThread(Fiber::SP *sp) { fiberPtr.swap(*sp); }
        FiberBindThread(std::function<void()> f) : cb(f) {}
        FiberBindThread(std::function<void()> *f) { cb.swap(*f); }

        void reset()
        {
            fiberPtr = nullptr;
            cb = nullptr;
        }
    };

    template<class FiberOrCb>
    void scheduleNoLock(FiberOrCb fc) {
        FiberBindThread ft(fc);
        if (ft.fiberPtr || ft.cb) {
            mFiberQueue.push_back(ft);
        }
    }

protected:
    void run();
    void setThis();

    /**
     * @brief 线程空闲时执行idle协程
     */
    virtual void idle();

    virtual bool stopping();

protected:
    bool                        mStopping;      // 是否停止

private:
    eular::String8              mName;          // 调度器名字
    Fiber::SP                   mRootFiber;     // userCaller为true时有效
    std::list<FiberBindThread>  mFiberQueue;    // 待执行的协程队列
};

} // namespace eular

#endif // __COURING_SCHEDULER_H__
