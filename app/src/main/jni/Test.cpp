#include <unistd.h>
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
            mLock.unlock();
        }
        else if(what == SerialDevice::EventTypeUpgradeFail)
        {
            LOGD(LOG_TAG,"SerialDevice::EventTypeUpgradeFail");
            mLock.unlock();
        }
        else if(what == SerialDevice::EventTypeUpgradeProgress)
        {
            LOGD(LOG_TAG, "upgrade progess =%d%%", arg1);
        }

    }
private:
    Mutex mLock;
};
class TestThread :public  Thread
{
public:
    TestThread(const char * name);
    virtual bool        threadLoop();
private:
    const char * mName;
};
TestThread::TestThread(const char *name)
:mName(name)
{

}
bool TestThread::threadLoop() {
    const char * version = NULL;
    EventListenerTest *mListener;
    SerialDevice * mSerialDevice;
    mListener= new EventListenerTest();
    mSerialDevice = new SerialDevice("/dev/ttyUSB0", 9600, 0,1,8);
    LOGD(LOG_TAG, "%s   Upgrade test!", mName);
    if(mSerialDevice == NULL)
    {
        LOGE(LOG_TAG, "%s   mSerialDevice init error!!!", mName);
        return 0;
    }
    if(mSerialDevice->openDevice() != 0)
    {
        LOGE(LOG_TAG, "%s   mSerialDevice open error!!!",mName);
        return 0;
    }
    mSerialDevice->setEventListener(mListener);
    if((version = mSerialDevice->getTargetVersion()) == NULL)
    {
        LOGE(LOG_TAG, "%s   mSerialDevice getTargetVersion error!!!", mName);
    }
    else
    {
        LOGD(LOG_TAG, "%s   get version %s",mName, version);
    }
    mSerialDevice->upgrade("/home/xuss/SmartReader", "./SmartReader");
    mListener->wait();
    delete mListener;
    delete mSerialDevice;
    return false;
}
int main() {


    int i = 100;
    char  testname[128];
    while (i--) {
        sprintf(testname, "Test-%d", 100 -i);
        TestThread mTest1(testname);
        mTest1.run();
        mTest1.join();
        sleep(20);
    }
/*
    TestThread mTest2("mTest2");
    mTest1.run();
    TestThread mTest3("mTest3");
    mTest1.run();
*/


    return 0;
}

