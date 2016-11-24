//
// Created by xuss on 2016/11/22.
//
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Uart.h"
#include "InspiryLog.h"
#include "SerialDevice.h"

#define LOG_TAG "Uart"

Uart::Uart(SerialDevice * serial_device, const char *name, int baudrate, int bits, int stop,int parity):
mCom(name),
mBaudrate(baudrate),
mBits(bits),
mStop(stop),
mParity(parity),
mCmd(&mCom),
mSerialDevice(serial_device)
{
    memset(mVersion, 0, sizeof(mVersion));
    memset(mProduceName, 0, sizeof(mProduceName));
    memset(mProduceTime, 0, sizeof(mProduceTime));
}

int Uart::open() {
    int ret = 0;
    if(mLock.tryLock() != 0)
    {
        LOGE(LOG_TAG, "Uart::open try lock err!!!");
        return -1;
    }
    if(!mCom.isOpen())
    {
        ret = mCom.open();
        if(ret != 0)
        {
            goto error1;
        }
    }
    ret = mCom.setConfig(mBaudrate, mBits, mStop, mParity);

    if(ret != 0)
    {
        LOGE(LOG_TAG, "Uart setConfig error!!!");
        goto error2;
    }
    sleep(1);
    mLock.unlock();
    getVersion(mVersion, (int)sizeof(mVersion), mProduceName, (int)sizeof(mProduceName), mProduceTime, (int)sizeof(mProduceTime));
    return ret;
error2:
    mCom.close();
error1:
    mLock.unlock();
    return ret;
}

int Uart::close() {
    int ret = 0;
    if(mLock.tryLock() != 0)
    {
        LOGE(LOG_TAG, "Uart::close try lock err!!!");
        return -1;
    }
    if(mCom.isOpen())
    {
        ret = mCom.close();
    }
    mLock.unlock();
    return ret;
}

typedef struct Update_info
{
    unsigned long int Addr;
    unsigned long int Len;
    unsigned long int Lastaddr;
    char  state[5];
    char  MD5[32];
} Update_info;

int Uart::upgread(const char *file, const char *md5file) {

    FILE *pMyFile;

    char Smart_buf[512];
    char info_buf[200];

    Update_info Smart_info;
    Update_info Smart_date;
    DWORD fileAddr = 0;
    DWORD fileLen = 0;
    DWORD needLen = 0;
    DWORD writeSize = UART_PACKET_SIZE - 12;

#define SMART_NAME "/home/SmartReader"
#define SMART_ADDR "./SmartReader"
    if (mCmd.fileDel(SMART_NAME) != CMD_OK) {
        LOGD(LOG_TAG, "upgread error, fileDel %s error", SMART_NAME);
        return -1;
    }

    fileLen = getFilesize(file);
    LOGD(LOG_TAG, "file:%s fileLen:%d", file, fileLen);
    if(fileLen <= 0)
    {
        LOGE(LOG_TAG, "file:%s fileLen:%d <=0", file, fileLen);
        return -1;
    }

    pMyFile = fopen((char *) file, "rb");
    if (pMyFile == NULL) {
        LOGD(LOG_TAG, "fopen %s error(%d)", file, errno);
        return -1;
    }
    Smart_info.Addr = 0xd00000;
    Smart_info.Len = fileLen;
    Smart_info.Lastaddr = 0xd00000 + fileLen;

    memcpy(Smart_info.state, "     ", 5);
    memset(Smart_buf, 0, sizeof(Smart_buf));

    if(getFileMd5(file, Smart_info.MD5)!= 0)
    {
        return -1;
    }

    sprintf(Smart_buf, "%08d%07d%08d%5s%32s", 0xd00000, fileLen, 0xd00000 + fileLen, "     ",
            Smart_info.MD5);
    LOGD(LOG_TAG, "Smart_buf==%s", Smart_buf);
    LOGD(LOG_TAG, "sizeof(Update_info)===%d", (int)sizeof(Update_info));
    if (mCmd.flashErase(0xf30000, 60) != CMD_OK) {
        LOGE(LOG_TAG, "USB_Flash_EraseFailed addr =0x%x", 0xf30000);
        goto ERROR1;
    }
    {
        int Smart_Len = Smart_info.Len; //erase flash 0xd00000 because is too long ,split it.
        int eraseLen = 256 * 1024;

        for (int addr = 0; addr < Smart_Len;) {
            if (Smart_Len - addr >= 256 * 1024) {
                eraseLen = 256 * 1024;
            }
            else {
                eraseLen = Smart_Len - addr;
            }
            if (mCmd.flashErase(0xd00000 + addr, eraseLen) != CMD_OK) {
                LOGE(LOG_TAG, "USB_Flash_EraseFailed addr =0x%x", 0xd00000);
                goto ERROR1;
            }
            //g_EraseProcess = addr*100/Smart_Len;
            addr += eraseLen;
        }
    }
    LOGD(LOG_TAG, "Smart_buf=%s", Smart_buf);

    if (mCmd.flashWrite( 0xf30000, (BYTE *)(Smart_buf), 60) != CMD_OK)
    {
        LOGE(LOG_TAG,"USB_Flash_WriteFailed addr=0x%x" ,0xf30000);
        goto ERROR1;
    }

    LOGD(LOG_TAG, "after flash write");

    memset(Smart_buf, 0, sizeof(Smart_buf));

    if (mCmd.flashRead(0xf30000, (BYTE *)(Smart_buf), 60) < 0)
    {
        LOGE(LOG_TAG,"USB_Flash_ReadFailed addr=0x%x",0xf30000);
        goto ERROR1;
    }
    sscanf(Smart_buf, "%8ld%7ld%8ld%5s%32s", &Smart_info.Addr, &Smart_info.Len, &Smart_info.Lastaddr, Smart_info.state, Smart_info.MD5);
    memcpy(Smart_info.MD5, &Smart_buf[28], 32);
/*
    LOGD(LOG_TAG,"Addr=%d", Smart_info.Addr);
    LOGD(LOG_TAG,"Len=%d", Smart_info.Len);
    LOGD(LOG_TAG,"Lastaddr=%d", Smart_info.Lastaddr);
    LOGD(LOG_TAG,"stata=%5s", Smart_info.state);
    LOGD(LOG_TAG,"MD5=%s", Smart_info.MD5);
*/

    while(fileAddr < fileLen)
    {
        int percent = 0;
        BYTE _tmpBuf[(UART_PACKET_SIZE)];
        int RemainingSizes = fileLen - fileAddr;
        needLen = RemainingSizes >= writeSize ? writeSize: RemainingSizes;
        memset(_tmpBuf, 0, sizeof(_tmpBuf));
        fread((char *)_tmpBuf, sizeof(BYTE), needLen, pMyFile);

        if (mCmd.flashWrite(0xd00000 + fileAddr, _tmpBuf, needLen) != CMD_OK)
        {
            LOGE(LOG_TAG,"UpDatingFailed");
            goto ERROR1;
        }
        fileAddr += needLen;

        percent = fileAddr*100/fileLen;
        mSerialDevice->onEvent(SerialDevice::EventTypeUpgradeProgress, percent, 0);
    }

    fclose(pMyFile);
    return 0;
ERROR1:
    fclose(pMyFile);
    return -1;
}

int Uart::getVersion(char * pVersionBuf, int lenV,
                     char *pProduceNameBuf, int lenN,
                     char * pProduceTimeBuf, int lenT) {
    int ret = 0;
    char pBuf[128]={0};
    ret = mCmd.flashRead(0xf10000, (BYTE *)(pBuf), 24);
    if(ret != 0)
    {
        LOGE(LOG_TAG, "getVersion error(%d)", ret);
        return -1;
    }
    if((lenV) < 32 || (lenN) < 32 || (lenT)< 32)
    {
        LOGE(LOG_TAG, "getVersion error, output buf is too small!!!");
        return -1;
    }
    LOGD(LOG_TAG, "getVersion %s", pBuf);

    if (strlen(pBuf) >= 19)
    {
        sscanf(pBuf, "%[^ ] %5s%5s", pProduceTimeBuf, pProduceNameBuf, pVersionBuf);
        return 0;
    }else
    {
        LOGE(LOG_TAG, "getVersion error(strlen(pBuf) <= 19)");
        return -1;
    }
}

int Uart::getFilesize(const char *path) {

    unsigned long filesize = -1;
    struct stat statbuff;
    if (stat(path, &statbuff) < 0)
    {
        return filesize;
    }
    else
    {
        filesize = statbuff.st_size;
    }
    return filesize;
}

int Uart::getFileMd5(const char *path, char *buf) {
    FILE *stream;
    char recvBuf[256] = {0};
    char cmdBuf[256] = {0};

    memset(recvBuf, 0, 256);
    memset(cmdBuf, 0, 256);

    if (NULL == path)
        return 0;

    sprintf(cmdBuf, "md5sum %s", path);
    stream = popen(cmdBuf, "r");
    if (NULL == stream) //added for exception
    {
        LOGD(LOG_TAG, "Call popen function failed");
        return -1;
    }
    fread(recvBuf, sizeof(char), sizeof(recvBuf) - 1, stream);
    pclose(stream);

    LOGD(LOG_TAG,"getFileMd5 recvBuf=%s", recvBuf);

    if (strlen(recvBuf) >= 32)
    {
        memcpy(buf, recvBuf, 32);
    }
    else
    {
        LOGD(LOG_TAG,"getFileMd5 recvBuf < 32");
        return -1;
    }


    LOGD(LOG_TAG,"getFileMd5 md5:%s", recvBuf);
    LOGD(LOG_TAG,"getFileMd5 buf=%s", buf);
    return  0;
}

int Uart::upgradeApp(const char *file, const char *md5file) {
    int ret;
    UartCmd::DeviceStatus_st status;

    Mutex::Autolock _l(mLock);
    ret = mCmd.getStatus(&status);
    if( ret != CMD_OK)
    {
        LOGE(LOG_TAG, "upgradeApp getStatus error(%d)", ret);
        return -1;
    }
    /**
     * 如果当前处于UPDATE_MODE，为了统一处理，我们下边也要进行UPDATE_MODE模式设置，这并不冲突
     * 所以我们把下边的代码段去掉
     */
    /*
    if(status.status == UPDATE_MODE)
    {
        LOGD(LOG_TAG,  "upgradeApp UPDATE_MODE");
        LOGE(LOG_TAG,  "upgradeApp UPDATE_MODE Not Implement!!!");

    }
    else*/
    {
        LOGD(LOG_TAG, "upgradeApp set Mode  to UPDATE_MODE ");
        ret = mCmd.setMode(UPDATE_MODE);
        if(ret != CMD_OK)
        {
            LOGE(LOG_TAG, "upgradeApp setMode error(%d)", ret);
            goto ERROR1;
            return -1;
        }
        /**
         * Notice：下位机程序切换，需要等待合适时间，需要解决这个问题
         */
        sleep(3);

        int baud_rate = 115200;
        LOGD(LOG_TAG, "upgradeApp set baudrate up to %d ", baud_rate);
        ret = mCmd.setUART(baud_rate, mBits, mStop, mParity);
        if(ret != CMD_OK)
        {
            LOGE(LOG_TAG, "upgradeApp setUART error(%d)", ret);
            return -1;
        }

        ret = mCmd.getStatus(&status);
        if( ret != CMD_OK)
        {
            LOGE(LOG_TAG, "upgradeApp getStatus error(%d)", ret);
            goto ERROR1;
            return -1;
        }
        if(status.status != UPDATE_MODE)
        {
            LOGE(LOG_TAG, "upgradeApp status != UPDATE_MODE");
            goto ERROR1;
            return -1;
        }
        ret = this->upgread(file, md5file);

        if(ret != CMD_OK)
        {
            LOGE(LOG_TAG, "upgradeApp error(%d)", ret);
            goto ERROR1;
            return -1;
        }

        LOGD(LOG_TAG, "upgradeApp set baudrate back to %d ", mBaudrate);

        ret = mCmd.setUART(mBaudrate, mBits, mStop, mParity);
        sleep(1);
        if(ret != CMD_OK)
        {
            LOGE(LOG_TAG, "upgradeApp setUART error(%d)", ret);
            return -1;
        }
        LOGD(LOG_TAG, "upgradeApp set Mode back to USER_MODE ");

        ret = mCmd.setMode(USER_MODE);
        if(ret != CMD_OK)
        {
            LOGE(LOG_TAG, "upgradeApp setMode error(%d)", ret);
            return -1;
        }
        return ret;
    }
    /**
     * ERROR1 代码段主要是处理 在波特率设置以后，升级出现失败，那么我们要恢复以前的波特率
     */
ERROR1:
    LOGD(LOG_TAG, "upgradeApp set baudrate back to %d ", mBaudrate);
    ret = mCmd.setUART(mBaudrate, mBits, mStop, mParity);

    if(ret != CMD_OK)
    {
        LOGE(LOG_TAG, "upgradeApp setUART error(%d)", ret);
        return -1;
    }
    /**
     * 升级失败后，就不要设置回USER_MODE了，因为在USER_MODE模式下运行的程序已经被破坏了
     */
    /*
    LOGD(LOG_TAG, "upgradeApp set Mode back to USER_MODE ");

    ret = mCmd.setMode(USER_MODE);
    if(ret != CMD_OK)
    {
        LOGE(LOG_TAG, "upgradeApp setMode error(%d)", ret);
        return -1;
    }
     */
    return -1;

}

const char *Uart::getVersion() {
    if(strlen(mVersion) >= 0)
    {
        return this->mVersion;
    }
    return NULL;
}

const char *Uart::getProduceTime() {
    if(strlen(mProduceTime) >= 0)
    {
        return this->mProduceTime;
    }
    return NULL;
}

const char *Uart::getProduceName() {
    if(strlen(mProduceName) >= 0)
    {
        return this->mProduceName;
    }
    return NULL;
}

Uart::~Uart() {
    LOGD(LOG_TAG, "Uart Destroy is called");
    Mutex::Autolock _l(mLock);
    if(mCom.isOpen())
    {
        mCom.close();
    }
    LOGD(LOG_TAG, "Uart Destroy is called end ");
}
