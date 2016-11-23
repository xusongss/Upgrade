//
// Created by xuss on 2016/11/20.
//
#define LOG_TAG "SerialDevice"
#include <string.h>
#include "SerialDevice.h"
#include "inspiry_log.h"
#include "UpdateThread.h"
#include <pthread.h>
#include <unistd.h>
SerialDevice::SerialDevice(const char *device, int baudrate, int parity, int stop, int bits):
mBaudrate(baudrate),
mParity(parity),
mStop(stop),
mBits(bits),
mListener(NULL),
mUpdateThread(this),
mIsUpdateing(false),
mUart(device, baudrate, bits, stop, parity)
{
    memset((void*)mPath, 0, sizeof(mPath));
    strncpy(mPath, device, sizeof(mPath)-1);
    LOGD(LOG_TAG,"SerialDevice construction is called device:%s", mPath);
}
int SerialDevice::openDevice()
{
    LOGD(LOG_TAG, "openDevice is called ");
    return mUart.open();
};
int SerialDevice::closeDevice()
{
    LOGD(LOG_TAG, "closeDevice is called");
    return mUart.close();
};
int SerialDevice::upgrade(const char * path, const char * md5path)
{
    LOGD(LOG_TAG, "upgrade: path=%s md5Path=%s", path, md5path);
    if(mLock.tryLock()!= 0 || mIsUpdateing || mUpdateThread.isRunning())
    {
        return 0;
    }
    strcpy(mPackagePath, path);
    strcpy(mMd5Path, md5path);
    mUpdateThread.run();
    mUpdateThread.waitBeginRun(mLock);
    mIsUpdateing = true;
    mLock.unlock();
    return 0;
}
const char * SerialDevice::getTargetVersion()
{
    LOGD(LOG_TAG, "getTargetVersion is called");
    return mUart.getVersion();

}
int SerialDevice::setEventListener(EventListener * listener)
{
    Mutex::Autolock _l(mLock);
    LOGD(LOG_TAG, "setEventListener is called");
    mListener = listener;
}
int SerialDevice::upgradeImp()
{
    int ret = 0;
    int event = EventTypeUpgradeFail;
    Mutex::Autolock _l(mLock);
    ret = mUart.upgradeApp(mPath, mMd5Path);
    event = ret == 0 ? EventTypeUpgradeSuccess:EventTypeUpgradeFail;

    if (mListener)
    {
        mListener->onEvent(event, 0, 0);
    }
    mIsUpdateing = false;
    return 0;
}


