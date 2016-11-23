//
// Created by xuss on 2016/11/22.
//

#ifndef UPGRADE_COM_H
#define UPGRADE_COM_H

#include "Mutex.h"

#define DWORD unsigned int
#define BYTE unsigned char
#define WORD unsigned long

#define UART_PACKET_SIZE 128

#define VENDOR_OUT_ID	0x05
#define VENDOR_IN_ID	0x06

class Com {
public:
    Com(const char * name);
    Com(const char * name, int baud, int parity,int stop, int bits);
public:
    int open();
    int close();
    int isOpen();
    int setConfig(int baudrate, int bits, int stop, int parity );
    int getBaudrate();
    BYTE ReadComDevice(BYTE *DataBuf, int len);
    BYTE WriteComDevice(BYTE *DataBuf, int len);
public:
    enum {
        ComDefaultBaudrate=9600,
        ComDefaultBits=8,
        ComDefaultStop=1,
        ComDefaultParity=0
    };
private:
    BYTE ReadCom(int  handle, BYTE *pBuffer, DWORD len);
    void WriteCom(int  handle, BYTE *pBuffer, DWORD len);
    int covBaudrate(int baudrate);
private:
    const char * mName;
    int mBaudrate;
    int mParity;
    int mStop;
    int mBits;
    int mHandle;
    Mutex mLock;
};


#endif //UPGRADE_COM_H
