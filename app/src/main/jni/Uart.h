//
// Created by xuss on 2016/11/22.
//

#ifndef UPGRADE_UART_H
#define UPGRADE_UART_H


#include "Com.h"
#include "UartCmd.h"
class SerialDevice;
class Uart {
public:
    Uart(SerialDevice * serial_device, const char * device_name, int baudrate, int bits, int stop,int parity );
    ~Uart();
    int open();
    int close();
    int upgradeApp(const char * file, const char * md5file);
    const char * getVersion();
    const char * getProduceName();
    const char * getProduceTime();
private:
    int getVersion(char * pVersionBuf, int lenV, char *pProduceNameBuf, int lenN, char * pProduceTimeBuf, int lenT);
    int upgread(const char * file, const char * md5file);
    int getFilesize(const char * path);
    int getFileMd5(const char * path, char * buf);
private:
    SerialDevice * mSerialDevice;
    UartCmd mCmd;
    Com mCom;
    Mutex mLock;
    char mVersion[64];
    char mProduceName[64];
    char mProduceTime[64];
    const int mBaudrate;
    const int mParity;
    const int mStop;
    const int mBits;
};


#endif //UPGRADE_UART_H
