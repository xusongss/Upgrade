//
// Created by xuss on 2016/11/22.
//
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>//com
#include "Com.h"
#include "InspiryLog.h"
#define LOG_TAG "COM"
Com::Com(const char * name ): mName(name),mHandle(-1)
{
}
Com::Com(const char *name, int baud, int parity, int stop, int bits) :
        mName(name),
        mBaudrate(baud),
        mParity(parity),
        mBits(bits)
{
}
int Com::open() {
    LOGD(LOG_TAG, "com %s open...", mName);

    Mutex::Autolock _l(mLock);
    mHandle = ::open(mName, O_RDWR);
    if (mHandle < 0)
    {
        LOGE(LOG_TAG, "com %s open failed", mName);
        return -1;
    }
    LOGD(LOG_TAG, "com %s open success", mName);

    setConfig(Com::ComDefaultBaudrate,
                Com::ComDefaultBits,
                Com::ComDefaultStop,
                Com::ComDefaultParity);
    return 0;
}
int Com::close() {
    int ret = 0;
    LOGD(LOG_TAG, "com %s setConfig closed!!!", mName);

    Mutex::Autolock _l(mLock);
    ret = ::close(mHandle);
    mHandle = -1;
    return ret;
}

int Com::isOpen() {
    Mutex::Autolock _l(mLock);
    return mHandle != -1;
}

BYTE Com::ReadCom(int  handle, BYTE *pBuffer, DWORD len)
{
    int ret = 0;
    DWORD g_UartReadCnt = 0;

    Mutex::Autolock _l(mLock);

    while (g_UartReadCnt < len)
    {
        ret = read(handle, &pBuffer[g_UartReadCnt], 1);
        if (ret == 1)
        {
            //printf("[%d] = 0x%02x\n", g_UartReadCnt, pBuffer[g_UartReadCnt]);
            if (pBuffer[0] != 0x55)
            {
                //printf("pBuffer[0] = 0x%02x\n", pBuffer[0]);
                LOGE(LOG_TAG,"pBuffer[0] = 0x%02x\n", pBuffer[0] );
                g_UartReadCnt = 0;
                LOGD(LOG_TAG,"0 error");
                return false;
                //continue;
            }
            if (g_UartReadCnt >= 1 && pBuffer[1] != 0xaa)
            {
                g_UartReadCnt = 0;
                LOGE(LOG_TAG,"1 error");
                return false;
                //continue;
            }
            if (g_UartReadCnt >= 2 && pBuffer[2] != 0x5a)
            {
                g_UartReadCnt = 0;
                LOGE(LOG_TAG,"2 error");
                return false;
                //continue;
            }
            if (g_UartReadCnt >= 3 && pBuffer[3] != 0xa5)
            {
                g_UartReadCnt = 0;
                LOGE(LOG_TAG,"3 error");
                return false;
                //continue;
            }
            g_UartReadCnt++;
        }
        if (g_UartReadCnt == UART_PACKET_SIZE)
        {
            LOGD(LOG_TAG,"READ UART Packet Full");
            return true;
            //UART_HandleOutReport(&g_UartOutBuffer[4]);
        }
    }
}
void Com::WriteCom(int  handle, BYTE *pBuffer, DWORD len)
{
    Mutex::Autolock _l(mLock);
    int ret = 0;
    do
    {
        ret = write(handle, pBuffer, len);
    }
    while (ret != len);

    LOGD(LOG_TAG,"Write UartOK %d", ret);
}

BYTE Com::WriteComDevice(BYTE *DataBuf, int len) {
    BYTE g_TempBuf[512 + 4] = {0};
    DWORD WritenLen = 0;
    DataBuf[0] = VENDOR_OUT_ID;
    memcpy(&g_TempBuf[4], DataBuf, len);
    g_TempBuf[0] = 0XAA;
    g_TempBuf[1] = 0X55;
    g_TempBuf[2] = 0XA5;
    g_TempBuf[3] = 0X5A;
    WriteCom(mHandle, g_TempBuf, len + 4);

    usleep(500);
    return true;
}

BYTE Com::ReadComDevice(BYTE *DataBuf, int len) {
    int ret = true;
    BYTE g_TempBuf[512 + 4] = {0};
    DWORD RcvLen = len + 4;
    memset(DataBuf, 0, RcvLen);
    DataBuf[0] = VENDOR_IN_ID;

    if (ReadCom(mHandle, g_TempBuf, RcvLen) != true)
    {
        ret = false;
    }
    memcpy(DataBuf, &g_TempBuf[4], len);
    return ret;
}

int Com::setConfig(int baudrate, int bits, int stop,int parity ) {
    struct termios new_cfg;
    struct termios old_cfg;
    int speed;
    if (tcgetattr(mHandle, &old_cfg) != 0)
    {
        LOGE(LOG_TAG, "tcgetattr error!!!");
        return -1;
    }
    //设置字符大小
    new_cfg = old_cfg;
    cfmakeraw(&new_cfg);
    new_cfg.c_cflag &= ~CSIZE;

    speed = covBaudrate(baudrate);

    cfsetispeed(&new_cfg, speed);
    cfsetospeed(&new_cfg, speed);

    //设置奇偶校验位
    switch (parity)
    {
        case 1://奇校验
        {
            new_cfg.c_cflag |= PARENB;
            new_cfg.c_cflag |= PARODD;
            new_cfg.c_iflag |= (INPCK | ISTRIP);
            break;
        }
        case 2://偶校验
        {
            new_cfg.c_iflag |= (INPCK | ISTRIP);
            new_cfg.c_cflag |= PARENB;
            new_cfg.c_cflag &= ~PARODD;
            break;
        }
        case 0://无奇偶校验位
        {
            new_cfg.c_cflag &= ~PARENB;
            break;
        }
        default://无奇偶校验位
        {
            new_cfg.c_cflag &= ~PARENB;
            break;
        }
    }
    //设置停止位
    switch (stop)
    {
        case 1:
        {
            new_cfg.c_cflag &= ~CSTOPB;
            break;
        }
        case 2:
        {
            new_cfg.c_cflag |= CSTOPB;
            break;
        }
        default:
        {
            new_cfg.c_cflag &= ~CSTOPB;
            break;
        }
    }
    //设置数据位
    switch (bits)
    {
        case 5:
        {
            new_cfg.c_cflag |= CS5;
            break;
        }
        case 6:
        {
            new_cfg.c_cflag |= CS6;
            break;
        }
        case 7:
        {
            new_cfg.c_cflag |= CS7;
            break;
        }
        case 8:
        {
            new_cfg.c_cflag |= CS8;
            break;
        }
        default:
        {
            new_cfg.c_cflag |= CS8;
            break;
        }
    }
    //设置等待时间和最小接收字符
    new_cfg.c_cc[VTIME] = 0;
    new_cfg.c_cc[VMIN] = 1;

    //处理未接收字符
    tcflush(mHandle, TCIFLUSH);
    //激活新配置
    if ((tcsetattr(mHandle, TCSANOW, &new_cfg)) != 0)
    {
        LOGE(LOG_TAG, "tcsetattr error!!!");
        return -1;
    }
    this->mBaudrate = baudrate;
    this->mParity = parity;
    this->mStop = stop;
    this->mBits = bits;
    return 0;
}

int Com::getBaudrate() {
    return this->mBaudrate;
}
int Com::covBaudrate(int baudrate) {
    int speed;
    //cov波特率
    switch (baudrate)
    {
        case 110:
        {
            speed = B110;
            break;
        }
        case 300:
        {
            speed = B300;
            break;
        }
        case 600:
        {
            speed = B600;
            break;
        }
        case 1200:
        {
            speed = B1200;
            break;
        }
        case 2400:
        {
            speed = B2400;
            break;
        }
        case 4800:
        {
            speed = B4800;
            break;
        }
        case 9600:
        {
            speed = B9600;
            break;
        }
        #if 0
        case 14400:
        {
            speed = B14400;
            break;
        }
        #endif
        case 19200:
        {
            speed = B19200;
            break;
        }
        case 38400:
        {
            speed = B38400;
            break;
        }
        #if 0
        case 56000:
        {
            speed = B56000;
            break;
        }
        #endif
        case 57600:
        {
            speed = B57600;
            break;
        }
        case 115200:
        {
            speed = B115200;
            break;
        }
        #if 0
        case 128000:
        {
            speed = B128000;
            break;
        }
        case 256000:
        {
            speed = B256000;
            break;
        }
        #endif
        default:
        {
            speed = B57600;
            break;
        }
    }
    return speed;
}
