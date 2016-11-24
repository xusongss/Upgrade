//
// Created by xuss on 2016/11/22.
//
#include <unistd.h>
#include "UpdateThread.h"
#include "SerialDevice.h"
#include "InspiryLog.h"

#define LOG_TAG "UpdateThread"
UpdateThread::UpdateThread(SerialDevice * device):
mDevice(device)
{

}
int32_t UpdateThread::readyToRun()
{
    mThreadBeginRun.broadcast();
    return 0;
}
int32_t UpdateThread::waitBeginRun(Mutex & lock)
{
    mThreadBeginRun.wait(lock);
    return 0;
}
bool UpdateThread::threadLoop()
{
    LOGD(LOG_TAG, "UpdateThread threadLoop");
    mDevice->upgradeImp();
    // return false indicate this threadLoop fun only run one time
    return false;
}