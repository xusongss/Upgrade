LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := barcodeupdate
LOCAL_SRC_FILES := \
	com_ipspiry_barcodeupdate_BarCodeSerialUpdate.cpp \
	Condition.cpp\
	EventListener.cpp\
	inspiry_log.cpp\
	Mutex.cpp\
	SerialDevice.cpp\
	Thread.cpp\
	UpdateImplement.cpp\
	

LOCAL_CFLAGS := -fpermissive
LOCAL_LDFLAGS := -Wl,--build-id

LOCAL_LDLIBS := -llog
include $(BUILD_SHARED_LIBRARY)
