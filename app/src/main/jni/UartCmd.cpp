//
// Created by xuss on 2016/11/22.
//

#include <unistd.h>
#include <stdlib.h>
#include "UartCmd.h"
#include "InspiryLog.h"
#define LOG_TAG "UartCmd"
UartCmd::UartCmd(Com *_com) :mCom(_com){
}




int UartCmd::setMode(BYTE Mode)
{
    int cmdResult =CMD_OK;
    LOGD(LOG_TAG,"setMode %d", Mode);
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_SET_MODE;
    mInBuffer[2] = Mode;
    cmdResult = handleCMD(mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4);
    LOGD(LOG_TAG,"setMode %d ret = %d ", Mode, cmdResult);
    return cmdResult;
}
int UartCmd::setUART(int baud_rate, BYTE data_bits, BYTE stop_bits, BYTE parity) {
    int cmdResult =CMD_OK;
    LOGD(LOG_TAG, "setUART baud_rate: %d data_bits =%d  stop_bits=%d parity=%d",
          baud_rate, data_bits, stop_bits,parity);
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_SET_NET;
    mInBuffer[2] = baud_rate & 0xff;
    mInBuffer[3] = (baud_rate >> 8) & 0xff;
    mInBuffer[4] = (baud_rate >> 16) & 0xff;
    mInBuffer[5] = (baud_rate >> 24) & 0xff;
    mInBuffer[6] = data_bits;
    mInBuffer[7] = stop_bits;
    mInBuffer[8] = parity;
    cmdResult = handleBaudRate( mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4, baud_rate);
    LOGD(LOG_TAG, "setUART ret = %d", cmdResult);
    return cmdResult;
}



int UartCmd::getStatus(UartCmd::DeviceStatus_st *status) {
    int cmdResult =CMD_OK;
    LOGD(LOG_TAG,"getStatus");
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_GET_STATUS;
    cmdResult = handleCMD(mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4);

    if(cmdResult != CMD_OK)
    {
        LOGD(LOG_TAG,"getStatus error(ret = %d)!!!", cmdResult);
        return cmdResult;
    }

    status->status = mOutBuffer[3];

    char g_TempBuf[256];
    memset(g_TempBuf, 0, 256);
    g_TempBuf[0] = '0';
    g_TempBuf[1] = 'x';

    memcpy(&g_TempBuf[2], &mOutBuffer[4], 8);
    status->deviceID1 = strtoul(g_TempBuf, NULL, 16);

    memcpy(&g_TempBuf[2], &mOutBuffer[13], 8);
    status->deviceID2 = strtoul(g_TempBuf, NULL, 16);

    memcpy(&g_TempBuf[2], &mOutBuffer[22], 8);
    status->deviceID3 = strtoul(g_TempBuf, NULL, 16);

    memcpy(&g_TempBuf[2], &mOutBuffer[31], 8);
    status->deviceID4 = strtoul(g_TempBuf, NULL, 16);
    LOGD(LOG_TAG,"getStatus ret= %d", cmdResult );
    return cmdResult;
}
int UartCmd::handleCMD(BYTE *inBuf, int inLen, BYTE *outBuf, int outLen) {
    int loopCnt = 0;
    int cmdResult = CMD_TIMEOUT;
    if (mCom->WriteComDevice(inBuf, inLen) == true)
    {
        LOGD(LOG_TAG, "inBuf[1]=%d\n", inBuf[1]);
        loop:
        if (mCom->ReadComDevice(outBuf, outLen) == true)
        {
            LOGD(LOG_TAG, "outBuf[0]=%d,outBuf[1]=%d\n", outBuf[0], outBuf[1]);
            cmdResult = CMD_OK;
            if ((outBuf[0] != VENDOR_IN_ID) || (inBuf[1] != outBuf[1]))
            {
                cmdResult = CMD_TIMEOUT;
            }
        }
        else
        {
            usleep(150 * 1000);
            loopCnt++;
            LOGD(LOG_TAG, "loopCnt = %d\n", loopCnt);
            if (loopCnt < outLen)
            {
                goto loop;
            }
        }
    }
    else
    {
        cmdResult = CMD_TIMEOUT;
    }

    return cmdResult;
}

int UartCmd::handleBaudRate(BYTE *inBuf, int inLen, BYTE *outBuf, int outLen, int baudRate) {
    if(baudRate == mCom->getBaudrate()){
        LOGD(LOG_TAG,"baudRate is identical");
        LOGD(LOG_TAG,"BaudRate : %d", baudRate);
        return CMD_OK;
    }
    int loopCnt = 0;
    int cmdResult = CMD_TIMEOUT;
    if (mCom->WriteComDevice(inBuf, inLen) == true)
    {
        LOGD(LOG_TAG, "BaudRate inBuf[1]=%d", inBuf[1]);
        LOGD(LOG_TAG, "BaudRate : %d", baudRate);
        //usleep(200*1000);
        loop:
        if (mCom->ReadComDevice(outBuf, outLen) == true)
        {
            LOGD(LOG_TAG,"outBuf[0]=%d,outBuf[1]=%d\n", outBuf[0], outBuf[1]);
            cmdResult = CMD_OK;
            if ((outBuf[0] != VENDOR_IN_ID) || (inBuf[1] != outBuf[1]))
            {
                cmdResult = CMD_TIMEOUT;
            }
        }
        else
        {
            usleep(150 * 1000);
            loopCnt++;
            LOGD(LOG_TAG,"loopCnt = %d", loopCnt);
            if (loopCnt < outLen)
            {
                goto loop;
            }
        }
    }
    else
    {
        cmdResult = CMD_TIMEOUT;
    }
    if(cmdResult == CMD_OK)
    {
        LOGD(LOG_TAG,"set com baudRate = %d OK", baudRate);

        mCom->setConfig(baudRate, Com::ComDefaultBits, Com::ComDefaultStop, Com::ComDefaultParity);
        sleep(2);
    }
    else
    {
        LOGD(LOG_TAG,"set com baudRate = %d ERROR", baudRate);
    }
    return cmdResult;
}

int UartCmd::flashRead(DWORD addr, BYTE *data, DWORD len) {
    int cmdResult;
    LOGD(LOG_TAG,"flashRead == %d", len);
    if (len > (512 - 10))
        LOGD(LOG_TAG,"len > 512");
    mInBuffer[1] = CMD_FLASH_READ;
    mInBuffer[2] = addr & 0xff;
    mInBuffer[3] = (addr >> 8) & 0xff;
    mInBuffer[4] = (addr >> 16) & 0xff;
    mInBuffer[5] = (addr >> 24) & 0xff;
    mInBuffer[6] = len & 0xff;
    mInBuffer[7] = (len >> 8) & 0xff;

    cmdResult = handleCMD(mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4);

    if(cmdResult != CMD_OK)
    {
        LOGE(LOG_TAG,"flashRead error(ret=%d)!!!", cmdResult);
        return cmdResult;
    }
    LOGD(LOG_TAG,"[0]=%#x,[1]=%#x,[2]=%#x", mOutBuffer[0], mOutBuffer[1], mOutBuffer[2]);
    LOGD(LOG_TAG,"[3]=%#x,[4]=%#x,[5]=%#x", mOutBuffer[3], mOutBuffer[4], mOutBuffer[5]);
    LOGD(LOG_TAG,"[6]=%#x,[8]=%#x,[9]=%#x", mOutBuffer[6], mOutBuffer[7], mOutBuffer[8]);
    LOGD(LOG_TAG,"[9]=%#x,[10]=%#x,[11]=%#x", mOutBuffer[6], mOutBuffer[7], mOutBuffer[8]);
    LOGD(LOG_TAG,"[12]=%#x,[13]=%#x,[14]=%#x", mOutBuffer[6], mOutBuffer[7], mOutBuffer[8]);

    memcpy(data, &mOutBuffer[3], len);
    return cmdResult;
}

int UartCmd::flashWrite(DWORD addr, BYTE *data, DWORD len) {
    int cmdResult;
    LOGD(LOG_TAG,"flashWrite");
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_FLASH_WRITE;
    mInBuffer[2] = addr & 0xff;
    mInBuffer[3] = (addr >> 8) & 0xff;
    mInBuffer[4] = (addr >> 16) & 0xff;
    mInBuffer[5] = (addr >> 24) & 0xff;
    mInBuffer[6] = len & 0xff;
    mInBuffer[7] = (len >> 8) & 0xff;
    memcpy(&mInBuffer[8], data, len);
    cmdResult = handleCMD(mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4);
    if(cmdResult != CMD_OK)
    {
        LOGE(LOG_TAG,"flashWrite error(ret=%d)!!!", cmdResult);
    }
    return cmdResult;
}
int UartCmd::flashErase(DWORD addr, DWORD len) {
    int cmdResult =CMD_OK;
    LOGD(LOG_TAG, "Flash Erase addr==%d  Len==%d\n", addr, len);
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_FLASH_ERASE;
    mInBuffer[2] = addr & 0xff;
    mInBuffer[3] = (addr >> 8) & 0xff;
    mInBuffer[4] = (addr >> 16) & 0xff;
    mInBuffer[5] = (addr >> 24) & 0xff;
    mInBuffer[6] = len & 0xff;
    mInBuffer[7] = (len >> 8) & 0xff;
    mInBuffer[8] = (len >> 16) & 0xff;
    mInBuffer[9] = (len >> 24) & 0xff;
    //memcpy(&g_InBuffer[8],data,len);

    cmdResult = handleCMD( mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4);
    if(cmdResult != CMD_OK)
    {
        LOGE(LOG_TAG,"flashErase error(ret=%d)!!!", cmdResult);
    }
    LOGD(LOG_TAG, "flashErase ret = %d ", cmdResult);
    return cmdResult;
}
int UartCmd::fileDel(const char *fileName) {
    int cmdResult;
    LOGD(LOG_TAG,"fileDel");
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_FILE_DEL;
    memcpy(&mInBuffer[2], fileName, strlen(fileName));
    cmdResult = handleCMD(mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4);
    if(cmdResult != CMD_OK)
    {
        LOGE(LOG_TAG,"fileDel error(ret = %d)", cmdResult);
    }
    return cmdResult;
}

int UartCmd::fileOpen(char *fileName) {
    int cmdResult;
    LOGD(LOG_TAG,"fileOpen %s", fileName);
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_FILE_OPEN;
    memcpy(&mInBuffer[2], fileName, strlen(fileName));
    cmdResult = handleCMD(mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4);
    if(cmdResult != CMD_OK)
    {
        LOGE(LOG_TAG,"fileOpen error(ret = %d)", cmdResult);
    }
    return cmdResult;
}

int UartCmd::fileClose(char *fileName) {
    int cmdResult;
    LOGD(LOG_TAG,"fileClose %s", fileName);
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_FILE_CLOSE;

    cmdResult = handleCMD(mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4);
    if(cmdResult != CMD_OK)
    {
        LOGE(LOG_TAG,"fileClose error(ret = %d)", cmdResult);
    }
    return cmdResult;
}

int UartCmd::fileRead(BYTE *outBuf, DWORD len) {
    int cmdResult;
    LOGD(LOG_TAG,"fileRead");
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_FILE_READ;
    mInBuffer[2] = len & 0xff;
    mInBuffer[3] = (len >> 8) & 0xff;
    mInBuffer[4] = (len >> 16) & 0xff;
    mInBuffer[5] = (len >> 24) & 0xff;
    cmdResult = handleCMD(mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4);
    if(cmdResult != CMD_OK)
    {
        LOGE(LOG_TAG,"fileRead error(ret = %d)", cmdResult);
        return cmdResult;
    }
    memcpy(outBuf, &mOutBuffer[3], len);
    return cmdResult;
}

int UartCmd::fileWrite(BYTE *inBuf, DWORD len) {
    int cmdResult;
    LOGD(LOG_TAG,"fileWrite");
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_FILE_WRITE;
    mInBuffer[2] = len & 0xff;
    mInBuffer[3] = (len >> 8) & 0xff;
    mInBuffer[4] = (len >> 16) & 0xff;
    mInBuffer[5] = (len >> 24) & 0xff;
    memcpy(&mInBuffer[6], inBuf, len);

    if (len < UART_PACKET_SIZE - 4 - 6)
    {
        len = UART_PACKET_SIZE - 4 - 6;
    }
    cmdResult = handleCMD(mInBuffer, len + 6, mOutBuffer, UART_PACKET_SIZE - 4);
    if(cmdResult != CMD_OK)
    {
        LOGE(LOG_TAG,"fileWrite error(ret = %d)", cmdResult);
    }
    return cmdResult;
}

int UartCmd::fileSize(char *fileName, int *size) {
    int cmdResult;
    LOGD(LOG_TAG,"fileSize %s" , fileName);
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_FILE_SIZE;
    memcpy(&mInBuffer[2], fileName, strlen(fileName));
    cmdResult = handleCMD(mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4);
    if (cmdResult != CMD_OK)
    {
        LOGE(LOG_TAG,"fileSize error(ret = %d)", cmdResult);
        return cmdResult;
    }
    *size = mOutBuffer[3] + (mOutBuffer[4] << 8) + (mOutBuffer[5] << 16) + (mOutBuffer[6] << 24);
    return cmdResult;
}

int UartCmd::setPara(BYTE inBuf) {
    int cmdResult;
    memset(mInBuffer, 0, sizeof(mInBuffer));
    memset(mOutBuffer, 0, sizeof(mOutBuffer));
    mInBuffer[1] = CMD_GET_CAMERA;
    mInBuffer[2] = inBuf;
    cmdResult =( mInBuffer, UART_PACKET_SIZE - 4, mOutBuffer, UART_PACKET_SIZE - 4);
    if (cmdResult != CMD_OK)
    {
        LOGE(LOG_TAG,"setPara error(ret = %d)", cmdResult);
    }
    return cmdResult;
}
