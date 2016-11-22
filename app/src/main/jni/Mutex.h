//
// Created by xuss on 2016/11/20.
//

#ifndef BARCODEUPDATE_MUTEX_H
#define BARCODEUPDATE_MUTEX_H


#include <stdint.h>
#include <pthread.h>
class Condition;
/*
 * Simple mutex class.  The implementation is system-dependent.
 *
 * The mutex must be unlocked by the thread that locked it.  They are not
 * recursive, i.e. the same thread can't lock it multiple times.
 */
class Mutex {
public:
    enum {
        PRIVATE = 0,
        SHARED = 1
    };

    Mutex();
    Mutex(const char* name);
    Mutex(int type, const char* name = NULL);
    ~Mutex();
    // lock or unlock the mutex
    int32_t    lock();
    void        unlock();

    // lock if possible; returns 0 on success, error otherwise
    int32_t    tryLock();

    // Manages the mutex automatically. It'll be locked when Autolock is
    // constructed and released when Autolock goes out of scope.
    class Autolock {
    public:
        inline Autolock(Mutex& mutex) : mLock(mutex)  { mLock.lock(); }
        inline Autolock(Mutex* mutex) : mLock(*mutex) { mLock.lock(); }
        inline ~Autolock() { mLock.unlock(); }
    private:
        Mutex& mLock;
    };

private:
    friend class Condition;

    // A mutex cannot be copied
    Mutex(const Mutex&);
    Mutex&      operator = (const Mutex&);

    pthread_mutex_t mMutex;
};
inline Mutex::Mutex() {
    pthread_mutex_init(&mMutex, NULL);
}
inline Mutex::Mutex(__attribute__((unused)) const char* name) {
    pthread_mutex_init(&mMutex, NULL);
}
inline Mutex::Mutex(int type, __attribute__((unused)) const char* name) {
    if (type == SHARED) {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&mMutex, &attr);
        pthread_mutexattr_destroy(&attr);
    } else {
        pthread_mutex_init(&mMutex, NULL);
    }
}
inline Mutex::~Mutex() {
    pthread_mutex_destroy(&mMutex);
}
inline int32_t Mutex::lock() {
    return -pthread_mutex_lock(&mMutex);
}
inline void Mutex::unlock() {
    pthread_mutex_unlock(&mMutex);
}
inline int32_t Mutex::tryLock() {
    return -pthread_mutex_trylock(&mMutex);
}
typedef Mutex::Autolock AutoMutex;
#endif //BARCODEUPDATE_MUTEX_H
