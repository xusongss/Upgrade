//
// Created by xuss on 2016/11/20.
//

#ifndef BARCODEUPDATE_SERIALDEVICE_H
#define BARCODEUPDATE_SERIALDEVICE_H

#include "EventListener.h"
#include "Mutex.h"
class UpdateImplement;
class SerialDevice {
public:
    SerialDevice(const char * device, int baudrate, int parity, int stop, int bits);
    int openDevice();
    int closeDevice();
    int upgrade(const char * path, const char * md5path);
    int setEventListener( EventListener * listener);
    const char * getTargetVersion();
public:
    typedef enum
    {
        EventTypeUpgradeSuccess=0x01000001,
        EventTypeUpgradeFail
    }EventType;
private:
    friend class UpdateImplement;
    int upgreadEvent(int status);
private:
    char mPath[128];
    const int mBaudrate;
    const int mParity;
    const int mStop;
    const int mBits;
    EventListener * mListener;
private:
    UpdateImplement *mImplement;
    Mutex mLock;
    bool mIsUpdate;
};


#endif //BARCODEUPDATE_SERIALDEVICE_H
