//
// Created by xuss on 2016/11/20.
//

#include <unistd.h>
#include "UpdateImplement.h"
#include "inspiry_log.h"
#define LOG_TAG "UpdateImplement"
UpdateImplement::UpdateImplement(SerialDevice * device):
mDevice(device)
{

}
int32_t   UpdateImplement:: readyToRun()
{
    mThreadBeginRun.broadcast();
    return 0;
}
bool UpdateImplement:: threadLoop()
{
    LOGD(LOG_TAG, "UpdateImplement threadLoop")
    sleep(10);
    mDevice->upgreadEvent(SerialDevice::EventTypeUpgradeSuccess);
    return false;
}
int32_t UpdateImplement::waitBeginRun(Mutex & lock)
{
    mThreadBeginRun.wait(lock);
    return 0;
}