//
// Created by xuss on 2016/11/20.
//

#ifndef BARCODEUPDATE_SERIALDEVICE_H
#define BARCODEUPDATE_SERIALDEVICE_H

#include "EventListener.h"
#include "Mutex.h"
#include "UpdateThread.h"
#include "Uart.h"

class SerialDevice {
public:
    SerialDevice(const char * device, int baudrate, int parity, int stop, int bits);
    /**
     * All of those method is used by JNI
     */
    int openDevice();
    int closeDevice();
    int upgrade(const char * path, const char * md5path);
    const char * getTargetVersion();
    int setEventListener( EventListener * listener);

public:
    typedef enum
    {
        EventTypeUpgradeSuccess=0x01000001,
        EventTypeUpgradeFail
    }EventType;
private:
    friend class UpdateThread;
    /**
    * All of thos method is used by UpdateThread
    */

   /**
    * upgradeImp
    * this function is always return 0
    */
    int upgradeImp();

private:
    char mPath[128];
    const int mBaudrate;
    const int mParity;
    const int mStop;
    const int mBits;
    EventListener * mListener;
private:
    UpdateThread mUpdateThread;
    Mutex mLock;
    bool mIsUpdateing;
private:
    Uart mUart;
    char mPackagePath[128];
    char mMd5Path[128];
};


#endif //BARCODEUPDATE_SERIALDEVICE_H
