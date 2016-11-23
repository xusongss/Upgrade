//
// Created by xuss on 2016/11/22.
//

#ifndef UPGRADE_UARTCMD_H
#define UPGRADE_UARTCMD_H


#include "Com.h"

#define CMD_TIMEOUT -1
#define CMD_OK 0

#define UPDATE_MODE	0X55
#define USER_MODE	0xaa

#define IDLE_MODE	0x00
#define WORK_MODE	0xfe
#define SET_MODE	0xf0
#define IMAGE_MODE	0x22
#define IMAGE_OPEN_MODE	0x33

class UartCmd {
public:
    UartCmd(Com * _com);
    int flashErase(DWORD addr, DWORD len);
    int flashRead(DWORD addr, BYTE *data, DWORD len);
    int flashWrite(DWORD addr, BYTE *data, DWORD len);
    int fileDel( const char *fileName);
    int fileOpen(char *fileName);
    int fileClose( char *fileName);
    int fileRead(BYTE *outBuf,DWORD len);
    int fileWrite(BYTE *inBuf, DWORD len);
    int fileSize(char *fileName, int *size);
    int setPara( BYTE inBuf);
    int setUART(int baud_rate, BYTE data_bits, BYTE stop_bits, BYTE parity);
    int setMode(BYTE Mode);
public:
    struct DeviceStatus_st
    {
        BYTE status;
        DWORD deviceID1;
        DWORD deviceID2;
        DWORD deviceID3;
        DWORD deviceID4;
    };
    int getStatus(DeviceStatus_st * status);
private:
    int handleBaudRate(BYTE *inBuf, int inLen, BYTE *outBuf, int outLen, int baudRate);
private:
    int handleCMD(BYTE *inBuf, int inLen, BYTE *outBuf, int outLen);
private:
    enum CMD_TYPE
    {
        CMD_GET_IMAGE = 0,		//摄像头的数据
        CMD_I2C_WRITE = 1,          //写I2C寄存器
        CMD_I2C_READ = 2,           //读I2C寄存器的命令结果
        CMD_SET_MODE = 3,		//设置模式
        CMD_GET_STATUS = 4,		//获取状态码
        CMD_LINUX_SHELL = 5,	//执行LINUX SHELL命令
        CMD_FILE_DEL = 6,
        CMD_FILE_OPEN = 7,
        CMD_FILE_CLOSE = 8,
        CMD_FILE_READ = 9,
        CMD_FILE_WRITE = 10,
        CMD_FILE_SIZE = 11,		//获取文件大小
        CMD_GPIO_HANDLE = 12,
        CMD_GET_CAMERA = 13,
        CMD_SET_LED = 14,
        CMD_SET_SPEAKER = 15,
        CMD_SET_NET = 16,
        CMD_SET_PRINTER = 17,
        CMD_FLASH_READ = 18,
        CMD_FLASH_WRITE = 19,
        CMD_FLASH_ERASE = 20,
        CMD_COUNT
    };
private:
    BYTE mInBuffer[512 * 8];
    BYTE mOutBuffer[512 * 8];
    Com *mCom;

};


#endif //UPGRADE_UARTCMD_H
