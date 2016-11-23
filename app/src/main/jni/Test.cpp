#include "SerialDevice.h"
#include "InspiryLog.h"

//
// Created by xuss on 2016/11/23.
//
#define LOG_TAG "test"
class EventListenerTest:public EventListener
{
public:
    EventListenerTest()
    {
        mLock.lock();
    }
    void wait()
    {
        Mutex::Autolock _l(mLock);
    }
    void onEvent( int what, int arg1, int arg2)
    {
        if(what == SerialDevice::EventTypeUpgradeSuccess)
        {
            LOGD(LOG_TAG,"SerialDevice::EventTypeUpgradeSuccess");
        }
        else
        {
            LOGD(LOG_TAG,"SerialDevice::EventTypeUpgradeFail");
        }
        mLock.unlock();
    }
private:
    Mutex mLock;
};
int main()
{
    const char * version = NULL;
    EventListenerTest *mListener;
    SerialDevice * mSerialDevice;
    mListener= new EventListenerTest();
     mSerialDevice = new SerialDevice("/dev/ttyUSB0", 9600, 0,1,8);
    if(mSerialDevice == NULL)
    {
        LOGE(LOG_TAG, "mSerialDevice init error!!!");
        return 0;
    }
    if(mSerialDevice->openDevice() != 0)
    {
        LOGE(LOG_TAG, "mSerialDevice open error!!!");
    }
    mSerialDevice->setEventListener(mListener);
    if((version = mSerialDevice->getTargetVersion()) == NULL)
    {
        LOGE(LOG_TAG, "mSerialDevice getTargetVersion error!!!");
    }
    mListener->wait();
    delete mListener;
    delete mSerialDevice;


    return 0;
}

