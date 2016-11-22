//
// Created by xuss on 2016/11/20.
//

#ifndef BARCODEUPDATE_THREAD_H
#define BARCODEUPDATE_THREAD_H
#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>
#include "Condition.h"

class Thread {
public:
    // Create a Thread object, but doesn't create or start the associated
    // thread. See the run() method.
    Thread();
    virtual ~Thread();
    // Start the thread in threadLoop() which needs to be implemented.
    virtual int32_t    run(    const char* name = 0,
                                int32_t priority = 0,
                                size_t stack = 0);
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void        requestExit();
    // Good place to do one-time initializations
    virtual int32_t    readyToRun();
    // Call requestExit() and wait until this object's thread exits.
    // BE VERY CAREFUL of deadlocks. In particular, it would be silly to call
    // this function from this object's thread. Will return WOULD_BLOCK in
    // that case.
    int32_t requestExitAndWait();
    // Wait until this object's thread exits. Returns immediately if not yet running.
    // Do not call from this object's thread; will return WOULD_BLOCK in that case.
    int32_t join();
    // Indicates whether this thread is running or not.
    bool    isRunning() const;
protected:
    // exitPending() returns true if requestExit() has been called.
    bool        exitPending() const;

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool        threadLoop() = 0;
private:
    static  void*             _threadLoop(void* user);
    // always hold mLock when reading or writing
    pthread_t     mThread;
    mutable Mutex           mLock;
    Condition       mThreadExitedCondition;
    int32_t        mStatus;
    // note that all accesses of mExitPending and mRunning need to hold mLock
    volatile bool           mExitPending;
    volatile bool           mRunning;
    pid_t           mTid;
};


#endif //BARCODEUPDATE_THREAD_H
