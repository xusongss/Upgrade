//
// Created by xuss on 2016/11/22.
//

#ifndef UPGRADE_UPDATETHREAD_H
#define UPGRADE_UPDATETHREAD_H


#include "Thread.h"
class SerialDevice;
class UpdateThread : public Thread{
public:
    UpdateThread(SerialDevice * device);
    virtual int32_t    readyToRun();
    int32_t waitBeginRun(Mutex & lock);
    virtual bool        threadLoop();
private:
    SerialDevice * mDevice;
    Condition mThreadBeginRun;
};


#endif //UPGRADE_UPDATETHREAD_H
