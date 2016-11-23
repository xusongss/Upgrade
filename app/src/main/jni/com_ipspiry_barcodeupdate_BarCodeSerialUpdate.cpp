//
// Created by xuss on 2016/11/20.
//
#define LOG_TAG "BarCodeSerialUpdateNative"
#include <android/log.h>
#include <jni.h>
#include <string.h>
#include <pthread.h>
#include "com_ipspiry_barcodeupdate_BarCodeSerialUpdate.h"
#include "inspiry_log.h"
#include "SerialDevice.h"
#include "EventListener.h"

typedef struct  {
    jfieldID mDeviceNativePointer;
    jmethodID postEvent;
}fields_t;
static fields_t g_field;
const char * BarCodeSerialUpdate_mDeviceNativePointer_Jni_Id = "mDeviceNativePointer";

static JavaVM *gVM=NULL;
static JavaVM* getJavaVM()
{
    return gVM;
}
static void setJavaVM(JavaVM * vm)
{
    gVM = vm;
}
class BarCodeSerialUpdateEventListener :public EventListener
{
public:
    BarCodeSerialUpdateEventListener(JNIEnv* env, jobject weakThiz, jclass clazz);
    ~BarCodeSerialUpdateEventListener();
    virtual void onEvent(int what, int arg1, int arg2);
private:
    static JNIEnv* getJNIEnv(bool* needsDetach);
    static void detachJNI();
private:
    jobject mWeakThiz;
    jclass mClazz;
};
BarCodeSerialUpdateEventListener::BarCodeSerialUpdateEventListener(JNIEnv* env, jobject weakThiz, jclass clazz) :
        mWeakThiz(env->NewGlobalRef(weakThiz)),
        mClazz((jclass)env->NewGlobalRef(clazz))
{}
void BarCodeSerialUpdateEventListener::onEvent( int what, int arg1, int arg2)
{
    bool needsDetach = false;
    JNIEnv* env = getJNIEnv(&needsDetach);
    if (env != NULL) {
        LOGD(LOG_TAG,"onEvent event is posted");
        env->CallStaticVoidMethod(mClazz, g_field.postEvent,  what, arg1, arg2,mWeakThiz);
    } else {
        LOGE(LOG_TAG,"onEvent event will not posted");
    }
    if (needsDetach) {
        detachJNI();
    }
}
JNIEnv* BarCodeSerialUpdateEventListener::getJNIEnv(bool* needsDetach) {
    *needsDetach = false;
    JavaVM* vm = getJavaVM();
    JNIEnv*env = NULL;
    if(vm->GetEnv((void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        env = NULL;
    }

    if (env == NULL) {
        JavaVMAttachArgs args = {JNI_VERSION_1_4, NULL, NULL};
        JavaVM* vm = getJavaVM();
        int result = vm->AttachCurrentThread(&env, (void*) &args);
        if (result != JNI_OK) {
            LOGE(LOG_TAG,"thread attach failed: %#x", result);
            return NULL;
        }
        *needsDetach = true;
    }

    if(env==NULL)
    {
        LOGE(LOG_TAG,"getJNIEnv env is NULL!!!");
    }
    return env;
}
void BarCodeSerialUpdateEventListener::detachJNI() {

    JavaVM* vm = getJavaVM();
    int result = vm->DetachCurrentThread();
    if (result != JNI_OK) {
        LOGE(LOG_TAG,"thread detach failed: %#x", result);
    }

}
BarCodeSerialUpdateEventListener::~BarCodeSerialUpdateEventListener()
{
    bool needsDetach = false;
    JNIEnv* env = getJNIEnv(&needsDetach);
    if (env != NULL) {
        env->DeleteGlobalRef(mWeakThiz);
        env->DeleteGlobalRef(mClazz);
    } else {
        LOGE(LOG_TAG,"leaking JNI object references");
    }
    if (needsDetach) {
        detachJNI();
    }
}
/*
 * nativeClassInit
 */
JNIEXPORT void JNICALL Java_com_ipspiry_barcodeupdate_BarCodeSerialUpdate_nativeClassInit
  (JNIEnv *env, jclass classzz)
  {
    g_field.mDeviceNativePointer = env->GetFieldID(classzz, BarCodeSerialUpdate_mDeviceNativePointer_Jni_Id, "J");
    if(g_field.mDeviceNativePointer == NULL)
    {
      LOGE(LOG_TAG, "can't find com/ipspiry/barcodeupdate/BarCodeSerialUpdate.%s",
              BarCodeSerialUpdate_mDeviceNativePointer_Jni_Id);
    }
      g_field.postEvent = env->GetStaticMethodID(classzz, "postEventFromNative",
                                                "(IIILjava/lang/Object;)V");
      if (g_field.postEvent == NULL) {
          LOGE(LOG_TAG,"can't find com/ipspiry/barcodeupdate/BarCodeSerialUpdate.postEventFromNative");
      }
      env->GetJavaVM(&gVM); //保存到全局变量中JVM

  }
static void BarCodeSerialUpdate_setDeviceNativePointer(JNIEnv* env, jobject thizz, SerialDevice * pdevice)
{
    env->SetLongField(thizz, g_field.mDeviceNativePointer, (long)pdevice);
}
/**
 * openNative
 */
JNIEXPORT jint JNICALL Java_com_ipspiry_barcodeupdate_BarCodeSerialUpdate_openNative
        (JNIEnv *env, jobject thizz, jobject weakThiz, jstring path, jint baudrate, jint parity, jint stop, jint bits)
{

    SerialDevice * pdevice = NULL;
    const char * deviceName = path != NULL ? env->GetStringUTFChars(path, NULL):"/dev/ttyS0";
    pdevice = new SerialDevice(deviceName, baudrate, parity, stop, bits);
    if(pdevice == NULL)
    {
        LOGE(LOG_TAG, "openNative: pdevice is NULL!!!");
    }
    BarCodeSerialUpdate_setDeviceNativePointer(env, thizz, pdevice);

    if(pdevice->openDevice() != 0)
    {
        return 1;
    }
    jclass clazz = env->GetObjectClass(thizz);
    if (clazz == NULL) {
        return 1;
    }
    pdevice->setEventListener(new BarCodeSerialUpdateEventListener(env, weakThiz, clazz));

    return 0;
}
/*
 * closeNative
 */
JNIEXPORT jint JNICALL Java_com_ipspiry_barcodeupdate_BarCodeSerialUpdate_closeNative
        (JNIEnv *env, jobject thizz)
{
    SerialDevice * pdevice = NULL;
    pdevice = (SerialDevice*)env->GetLongField(thizz, g_field.mDeviceNativePointer);

    if(pdevice == NULL)
    {
        LOGE(LOG_TAG, "closeNative: pdevice is NULL");
        return -1;
    }
    if(pdevice->closeDevice() == 0)
    {
        delete  pdevice;
        LOGD(LOG_TAG, "closeNative: success");
        env->SetLongField(thizz, g_field.mDeviceNativePointer, 0);
        return 0;
    }
    else {
        LOGE(LOG_TAG, "closeNative: pdevice error");
        return -1;
    }
}
/*
 * updateNative
 */
JNIEXPORT jint JNICALL Java_com_ipspiry_barcodeupdate_BarCodeSerialUpdate_updateNative
        (JNIEnv *env, jobject thizz, jstring path, jstring md5Path)
{
    int ret = 0;
    SerialDevice * pdevice = NULL;
    pdevice = (SerialDevice*)env->GetLongField(thizz, g_field.mDeviceNativePointer);

    if(pdevice == NULL)
    {
        LOGE(LOG_TAG, "closeNative: pdevice is NULL");
        return 1;//false
    }
    const char * packagePath= path!=NULL?env->GetStringUTFChars(path, NULL):NULL;
    const char * md5checkPath= md5Path!=NULL?env->GetStringUTFChars(md5Path, NULL):NULL;

    if(packagePath == NULL || md5checkPath == NULL)
    {
        LOGE(LOG_TAG, "updateNative: packagePath or md5checkPath is null!!!");
        return 1;//false
    }

    return pdevice->upgrade(packagePath, md5checkPath);
}
JNIEXPORT jstring JNICALL Java_com_ipspiry_barcodeupdate_BarCodeSerialUpdate_getVersionNative
        (JNIEnv *env, jobject thizz)
{
    SerialDevice * pdevice = NULL;
    pdevice = (SerialDevice*)env->GetLongField(thizz, g_field.mDeviceNativePointer);

    if(pdevice == NULL)
    {
        LOGE(LOG_TAG, "closeNative: pdevice is NULL");
        return NULL;
    }
    const char * version = pdevice->getTargetVersion();
    if(version != NULL)
    {
        return env->NewStringUTF(version);
    }
    else
    {
        return NULL;
    }
}
