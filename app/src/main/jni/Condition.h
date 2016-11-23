//
// Created by xuss on 2016/11/20.
//

#ifndef BARCODEUPDATE_CONDITION_H
#define BARCODEUPDATE_CONDITION_H

#include <sys/types.h>
#include <stdint.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "Mutex.h"
typedef long long nsecs_t;
/*
 * Condition variable class.  The implementation is system-dependent.
 *
 * Condition variables are paired up with mutexes.  Lock the mutex,
 * call wait(), then either re-wait() if things aren't quite what you want,
 * or unlock the mutex and continue.  All threads calling wait() must
 * use the same mutex for a given Condition.
 */
class Condition {
public:
    enum {
        PRIVATE = 0,
        SHARED = 1
    };

    enum WakeUpType {
        WAKE_UP_ONE = 0,
        WAKE_UP_ALL = 1
    };

    Condition();
    Condition(int type);
    ~Condition();
    // Wait on the condition variable.  Lock the mutex before calling.
    int32_t wait(Mutex& mutex);
    // same with relative timeout
    int32_t waitRelative(Mutex& mutex, nsecs_t reltime);
    // Signal the condition variable, allowing one thread to continue.
    void signal();
    // Signal the condition variable, allowing one or all threads to continue.
    void signal(WakeUpType type) {
        if (type == WAKE_UP_ONE) {
            signal();
        } else {
            broadcast();
        }
    }
    // Signal the condition variable, allowing all threads to continue.
    void broadcast();
private:
    pthread_cond_t mCond;
};

inline Condition::Condition() {
    pthread_cond_init(&mCond, NULL);
}
inline Condition::Condition(int type) {
    if (type == SHARED) {
        pthread_condattr_t attr;
        pthread_condattr_init(&attr);
        pthread_condattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&mCond, &attr);
        pthread_condattr_destroy(&attr);
    } else {
        pthread_cond_init(&mCond, NULL);
    }
}
inline Condition::~Condition() {
    pthread_cond_destroy(&mCond);
}
inline int32_t Condition::wait(Mutex& mutex) {
    return -pthread_cond_wait(&mCond, &mutex.mMutex);
}
inline int32_t Condition::waitRelative(Mutex& mutex,  nsecs_t reltime)
{
    struct timespec ts;
    struct timeval t;
    gettimeofday(&t, NULL);
    ts.tv_sec = t.tv_sec;
    ts.tv_nsec= t.tv_usec*1000;
    ts.tv_sec += reltime/1000000000;
    ts.tv_nsec+= reltime%1000000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec  += 1;
    }
    return -pthread_cond_timedwait(&mCond, &mutex.mMutex, &ts);
}
inline void Condition::signal() {
    pthread_cond_signal(&mCond);
}
inline void Condition::broadcast() {
    pthread_cond_broadcast(&mCond);
}
#endif //BARCODEUPDATE_CONDITION_H
