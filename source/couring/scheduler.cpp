/*************************************************************************
    > File Name: scheduler.cpp
    > Author: hsz
    > Brief:
    > Created Time: Sun 27 Feb 2022 05:41:32 PM CST
 ************************************************************************/

#include "scheduler.h"
#include "hook.h"

#include <utils/utils.h>
#include <log/log.h>

#define LOG_TAG "scheduler"

namespace eular {

static thread_local Scheduler *gScheduler = nullptr;    // 线程调度器

Scheduler::Scheduler(const eular::String8 &name) :
    mStopping(true),
    mName(name)
{
    Fiber::GetThis();

    gScheduler = this;
    mRootFiber.reset(new Fiber(std::bind(&Scheduler::run, this)));
    Thread::SetName(name);

    LOGD("root thread id %ld", gettid());
}

Scheduler::~Scheduler()
{
    LOG_ASSERT(mStopping, "You should call stop before deconstruction");
    if (gScheduler == this) {
        gScheduler = nullptr;
    }
}

Scheduler *Scheduler::GetScheduler()
{
    return gScheduler;
}

void Scheduler::start()
{
    Fiber::GetThis()->resume();
}

void Scheduler::stop()
{
    mStopping = true;
}

void Scheduler::run()
{
    LOGI("Scheduler::run() in %s:%d", Thread::GetName().c_str(), gettid());
    setThis();

    Fiber::SP idleFiber(new Fiber(std::bind(&Scheduler::idle, this)));
    Fiber::SP cbFiber(nullptr);

    FiberBindThread ft;
    while (true) {
        ft.reset();
        bool needTickle = false;
        bool isActive = false;
        {
            auto it = mFiberQueue.begin();
            while (it != mFiberQueue.end()) {
                LOG_ASSERT(it->fiberPtr || it->cb, "task can not be null");
                ft = *it;
                mFiberQueue.erase(it++);
                isActive = true;
                break;
            }
            needTickle |= it != mFiberQueue.end();
        }

        if (ft.fiberPtr && (ft.fiberPtr->getState() != Fiber::EXEC && ft.fiberPtr->getState() != Fiber::EXCEPT)) {
            ft.fiberPtr->resume();
            if (ft.fiberPtr->getState() == Fiber::READY) {  // 用户主动设置协程状态为REDAY
                schedule(ft.fiberPtr);
            } else if (ft.fiberPtr->getState() != Fiber::TERM &&
                       ft.fiberPtr->getState() != Fiber::EXCEPT) {
                ft.fiberPtr->mState = Fiber::HOLD;
            }
            ft.reset();
        } else if (ft.cb) {
            if (cbFiber) {
                cbFiber->reset(ft.cb);
            } else {
                cbFiber.reset(new Fiber(ft.cb));
                LOG_ASSERT(cbFiber != nullptr, "");
            }
            ft.reset();
            cbFiber->resume();
            if (cbFiber->getState() == Fiber::READY) {
                schedule(cbFiber);
                cbFiber.reset();
            } else if (cbFiber->getState() == Fiber::EXCEPT || cbFiber->getState() == Fiber::TERM) {
                cbFiber->reset(nullptr);
            } else {
                cbFiber->mState = Fiber::HOLD;
                cbFiber.reset();
            }
        } else {
            if (isActive) {
                continue;
            }

            idleFiber->resume();
            if (idleFiber->getState() == Fiber::TERM || idleFiber->getState() == Fiber::EXCEPT) {
                LOGI("idle fiebr exit");
                break;
            }
        }
    }
}

void Scheduler::setThis()
{
    gScheduler = this;
}

void Scheduler::idle()
{
    while (!stopping()) {
        LOGI("%s() fiber id: %lu", __func__, Fiber::GetFiberID());
        Fiber::Yeild2Hold();
    }
}

bool Scheduler::stopping()
{
    return mStopping && mFiberQueue.empty();
}

} // namespace eular
