LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := barcodeupdate
LOCAL_SRC_FILES := \
	Test.cpp\
	EventListener.cpp\
	UpdateThread.cpp\
	Thread.cpp\
	InspiryLog.cpp\
	com_ipspiry_barcodeupdate_BarCodeSerialUpdate.cpp\
	Mutex.cpp\
	UartCmd.cpp\
	Condition.cpp\
	Uart.cpp\
	SerialDevice.cpp\
	Com.cpp\
	inspiry_log.cpp\
	

LOCAL_CFLAGS := -fpermissive
LOCAL_LDFLAGS := -Wl,--build-id
LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := update
LOCAL_SRC_FILES := \
	Test.cpp\
	EventListener.cpp\
	UpdateThread.cpp\
	Thread.cpp\
	InspiryLog.cpp\
	com_ipspiry_barcodeupdate_BarCodeSerialUpdate.cpp\
	Mutex.cpp\
	UartCmd.cpp\
	Condition.cpp\
	Uart.cpp\
	SerialDevice.cpp\
	Com.cpp\
	inspiry_log.cpp\
	

LOCAL_CFLAGS := -fpermissive
LOCAL_LDFLAGS := -Wl,--build-id
LOCAL_LDLIBS := -llog
include $(BUILD_EXECUTABLE)


