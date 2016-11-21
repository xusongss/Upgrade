//
// Created by xuss on 2016/11/20.
//
#define LOG_TAG "SerialDevice"
#include <string.h>
#include "SerialDevice.h"
#include "inspiry_log.h"
#include "UpdateImplement.h"
#include <pthread.h>
#include <unistd.h>
SerialDevice::SerialDevice(const char *device, int baudrate, int parity, int stop, int bits):
mBaudrate(baudrate),
mParity(parity),
mStop(stop),
mBits(bits),
mListener(NULL),
mIsUpdate(false)
{
    memset((void*)mPath, 0, sizeof(mPath));
    strncpy(mPath, device, sizeof(mPath)-1);
    mImplement = new UpdateImplement(this);
    LOGD(LOG_TAG,"SerialDevice construction is called device:%s", mPath);
}
int SerialDevice::openDevice()
{
    LOGD(LOG_TAG, "openDevice is called ");
    return 0;
};
int SerialDevice::closeDevice()
{
    LOGD(LOG_TAG, "closeDevice is called");
    return 0;
};
int SerialDevice::upgrade(const char * path, const char * md5path)
{
    LOGD(LOG_TAG, "upgrade: path=%s md5Path=%s", path, md5path);
    /*
    pthread_t id;
    pthread_create(&id, NULL, threadLoop, (void*)this);
     */
    if(mLock.tryLock()!= 0 || mIsUpdate || mImplement->isRunning())
    {
        return 1;
    }
    mImplement->run();
    mImplement->waitBeginRun(mLock);
    /*
    if(mListener != NULL)
    {
        LOGD(LOG_TAG, "upgrade: post event");
        mListener->onEvent(SerialDevice::EventTypeUpgradeSuccess, 0, 0);
    }
     */
    mIsUpdate = true;
    mLock.unlock();
    return 0;
}
const char * SerialDevice::getTargetVersion()
{
    LOGD(LOG_TAG, "getTargetVersion is called");
    return "1.0.2";
}
int SerialDevice::setEventListener(EventListener * listener)
{
    LOGD(LOG_TAG, "setEventListener is called");
    mListener = listener;
}
int SerialDevice::upgreadEvent(int status) {
    Mutex::Autolock _l(mLock);
    sleep(10);
    if (mListener)
    {
        mListener->onEvent(status, 0, 0);
    }
    mIsUpdate = false;
    return 0;
}

