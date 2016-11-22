//
// Created by xuss on 2016/11/20.
//

#ifndef BARCODEUPDATE_UPDATEIMPLEMENT_H
#define BARCODEUPDATE_UPDATEIMPLEMENT_H

#include "Thread.h"
#include "SerialDevice.h"

class UpdateImplement:public Thread {
public:
    UpdateImplement(SerialDevice * device);
    virtual int32_t    readyToRun();
    int32_t waitBeginRun(Mutex & lock);
    virtual bool        threadLoop();
private:
    SerialDevice * mDevice;
    Condition mThreadBeginRun;

};


#endif //BARCODEUPDATE_UPDATEIMPLEMENT_H
