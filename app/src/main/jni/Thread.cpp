//
// Created by xuss on 2016/11/20.
//

#include <unistd.h>
#include "Thread.h"
#include "InspiryType.h"
#include "InspiryLog.h"

#define LOG_TAG "Thread"
Thread::Thread() : mThread(-1), mLock("Thread::mLock"), mStatus(NO_ERROR),
            mExitPending(false), mRunning(false),mTid(-1)
{

}
Thread::~Thread()
{
}

int32_t Thread::readyToRun()
{
    return NO_ERROR;
}
int CreateRawThreadEtc(void* (*entryFunction)(void*) ,
                              void *userData,
                              const char* threadName,
                              int32_t threadPriority,
                              size_t threadStackSize,
                                pthread_t *threadId)
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);



    if (threadStackSize) {
        pthread_attr_setstacksize(&attr, threadStackSize);
    }

    pthread_t thread;
    int result = pthread_create(&thread, &attr,
                                entryFunction, userData);
    pthread_attr_destroy(&attr);
    if (result != 0) {
        LOGE(LOG_TAG,"androidCreateRawThreadEtc failed (entry=%p, res=%d)\n"
                      "(android threadPriority=%d)",
              entryFunction, result, threadPriority);
        return 0;
    }

    // Note that *threadID is directly available to the parent only, as it is
    // assigned after the child starts.  Use memory barrier / lock if the child
    // or other threads also need access.
    if (threadId != NULL) {
        *threadId = thread; // XXX: this is not portable
    }
    return 1;
}
int32_t Thread::run(const char* name, int32_t priority, size_t stack)
{
    Mutex::Autolock _l(mLock);

    if (mRunning) {
        // thread already started
        return INVALID_OPERATION;
    }

    // reset status and exitPending to their default value, so we can
    // try again after an error happened (either below, or in readyToRun())
    mStatus = NO_ERROR;
    mExitPending = false;
    mThread = -1;

    mRunning = true;

    bool res;

        res = CreateRawThreadEtc(_threadLoop, this, name, priority, stack, &mThread);


    if (res == false) {
        mStatus = UNKNOWN_ERROR;   // something happened!
        mRunning = false;
        mThread = -1;
        return UNKNOWN_ERROR;
    }

    // Do not refer to mStatus here: The thread is already running (may, in fact
    // already have exited with a valid mStatus result). The NO_ERROR indication
    // here merely indicates successfully starting the thread and does not
    // imply successful termination/execution.
    return NO_ERROR;

    // Exiting scope of mLock is a memory barrier and allows new thread to run
}
void* Thread::_threadLoop(void* user)
{
    Thread* const self = static_cast<Thread*>(user);
    // this is very useful for debugging with gdb
    //self->mTid = gettid();


    bool first = true;

    do {
        bool result;
        if (first) {
            first = false;
            self->mStatus = self->readyToRun();
            result = (self->mStatus == NO_ERROR);

            if (result && !self->exitPending()) {
                // Binder threads (and maybe others) rely on threadLoop
                // running at least once after a successful ::readyToRun()
                // (unless, of course, the thread has already been asked to exit
                // at that point).
                // This is because threads are essentially used like this:
                //   (new ThreadSubclass())->run();
                // The caller therefore does not retain a strong reference to
                // the thread and the thread would simply disappear after the
                // successful ::readyToRun() call instead of entering the
                // threadLoop at least once.
                result = self->threadLoop();
            }
        } else {
            result = self->threadLoop();
        }

        // establish a scope for mLock
        {
            Mutex::Autolock _l(self->mLock);
            if (result == false || self->mExitPending) {
                self->mExitPending = true;
                self->mRunning = false;
                // clear thread ID so that requestExitAndWait() does not exit if
                // called by a new thread using the same thread ID as this one.
                self->mThread = -1;
                // note that interested observers blocked in requestExitAndWait are
                // awoken by broadcast, but blocked on mLock until break exits scope
                self->mThreadExitedCondition.broadcast();
                break;
            }
        }


    } while(1);

    return (void*)0;
}
void Thread::requestExit()
{
    Mutex::Autolock _l(mLock);
    mExitPending = true;
}
int32_t Thread::requestExitAndWait()
{
    Mutex::Autolock _l(mLock);
    if (mThread == pthread_self()) {
        LOGD(LOG_TAG,
                "Thread (this=%p): don't call waitForExit() from this "
                        "Thread object's thread. It's a guaranteed deadlock!",
                this);

        return -1;
    }

    mExitPending = true;

    while (mRunning == true) {
        mThreadExitedCondition.wait(mLock);
    }
    // This next line is probably not needed any more, but is being left for
    // historical reference. Note that each interested party will clear flag.
    mExitPending = false;

    return mStatus;
}
int32_t Thread::join()
{
    Mutex::Autolock _l(mLock);
    if (mThread == pthread_self()) {
        LOGD(LOG_TAG, "Thread (this=%p): don't call join() from this "
                        "Thread object's thread. It's a guaranteed deadlock!",
                 this);

        return -1;
    }
    while (mRunning == true) {
        mThreadExitedCondition.wait(mLock);
    }

    return mStatus;
}
bool Thread::isRunning() const {
    Mutex::Autolock _l(mLock);
    return mRunning;
}
bool Thread::exitPending() const
{
    Mutex::Autolock _l(mLock);
    return mExitPending;
}

