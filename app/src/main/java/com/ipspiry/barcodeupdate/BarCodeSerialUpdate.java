package com.ipspiry.barcodeupdate;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

import java.io.File;
import java.io.IOException;
import java.lang.ref.WeakReference;

/**
 * Created by xuss on 2016/11/20.
 */
public class BarCodeSerialUpdate {
    public static int EventTypeUpgradeSuccess=0x01000001;
    public static int EventTypeUpgradeFail=0x01000002;
    private static String TAG="BarCodeSerialUpdate";
    private static BarCodeSerialUpdate mUpdate = null;
    //
    //set isCheckAuthorization to false is for Test, set it to "true" for realization
    //
    private static  boolean mIsCheckAuthorization=false;

    //
    //Do Not Modify This Field, It Is Used By Native Code!!!
    //
    private int mDeviceNativePointer;

    private EventHandler mEventHandler;
    private OnEventAvailableListener mOnEventAvailableListener;

    private class EventHandler extends Handler {
        public EventHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (mOnEventAvailableListener != null) {
                mOnEventAvailableListener.OnEventAvailable(msg);
            }
        }
    }
    public interface OnEventAvailableListener {
        public void OnEventAvailable(Message msg);
    }

    //
    //Static Method
    //
    public  static boolean checkFile(String  path, String  md5Path){
        Log.e(TAG, "checkFile Is Not Implement!!!");
        return false;
    }

    public static BarCodeSerialUpdate buildInstance(SerialConfig config){
        Log.d(TAG,"buildInstance ...");
        if(mUpdate != null) {
            Log.e(TAG, "Another Instance Is Opening, Close It Before buildInstance!!!");
            return null;
        }
        try {
            checkSecurity(config.getPath());
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "checkSecurity Error!!!");
            return null;
        }
        mUpdate = new BarCodeSerialUpdate();
        if(!mUpdate.openDevice(config.getPath(), config.getBaudrate(), config.getParity(),config.getStop(),config.getBits())) {
            Log.e(TAG, "buildInstance Fail!!!");
        }else{
            Log.d(TAG,"buildInstance successful.");
        }
        return mUpdate;
    }
    public static boolean closeInstace(BarCodeSerialUpdate instance){
        Log.v(TAG, "closeInstace");
        instance.closeDevice();
        mUpdate = null;
        return true;
    }
    private static boolean checkSecurity(String path)throws SecurityException, IOException, Exception
    {
        Log.v(TAG, "checkSecurity path:"+path);
        if(!mIsCheckAuthorization)
        {
            Log.v(TAG, "checkSecurity successful");
            return true;
        }
        File device =new File(path);
        if(!device.exists()) {
            Log.v(TAG, ""+path+" is not exists");
            throw new Exception();
        }
        if (!device.canRead() || !device.canWrite()){
            try{
                Process su;
                su = Runtime.getRuntime().exec("/system/bin/su");
                String cmd = "chmod 666 " + device.getAbsolutePath() + "\n" + "exit\n";
                su.getOutputStream().write(cmd.getBytes());
                if ((su.waitFor() != 0) || !device.canRead() || !device.canWrite()) {
                    Log.e(TAG, "checkSecurity su error!!!");
                    throw new SecurityException();
                }
            }
            catch (Exception e) {
                e.printStackTrace();
                throw new SecurityException();
            }
        }
        Log.v(TAG, "checkSecurity successful");
        return true;
    }
    /*
        Object Method
     */
    private BarCodeSerialUpdate(){
        Looper looper;
        if ((looper = Looper.myLooper()) != null) {
            Log.v(TAG, "mEventHandler inited x1");
            mEventHandler = new EventHandler(looper);
        } else if ((looper = Looper.getMainLooper()) != null) {
            mEventHandler = new EventHandler(looper);
            Log.v(TAG, "mEventHandler inited by myLooper x2");
        } else {
            Log.v(TAG, "mEventHandler null");
            mEventHandler = null;
        }
    }
    private boolean openDevice(String device, int baudrate, int parity, int stop, int bits){
        int ret = 0;
        ret = openNative( new WeakReference<BarCodeSerialUpdate>(this), device, baudrate, parity, stop, bits);
        if(ret == 0) {
            return true;
        }
        else {
            return false;
        }
    }
    private boolean closeDevice(){
        int ret = 0;
        ret = closeNative();
        if(ret == 0) {
            return true;
        }
        else {
            return false;
        }
    }
    public String  getVersion(){
        return getVersionNative();
    }
    public boolean update(String path, String  md5Path){
        int ret = updateNative(path, md5Path);
        if(ret == 0) {
            return true;
        }
        else {
            return false;
        }

    }
    public void setOnEventAvailableListener(OnEventAvailableListener l) {
        mOnEventAvailableListener = l;
    }
    //
    // Do Not Modify This Method, It Is Used By Native Code!!!
    //
    @SuppressWarnings({"UnusedDeclaration"})
    private static void postEventFromNative(int what, int arg1, int arg2,Object selfRef ){
        WeakReference weakSelf = (WeakReference)selfRef;
        BarCodeSerialUpdate update = (BarCodeSerialUpdate)weakSelf.get();
        Log.d(TAG, "postEventFromNative:Receive event:"+what);
        if (update == null) {
            return;
        }
        if (update.mEventHandler != null) {
            Message m = update.mEventHandler.obtainMessage();
            m.what = what;
            m.arg1 = arg1;
            m.arg2 = arg2;
            update.mEventHandler.sendMessage(m);
        }
    }
    /*
        Native Method
     */
    private native String getVersionNative();
    private native int updateNative(String path, String  md5Path);
    private native int openNative(Object weakSelf, String device, int baudrate, int parity, int stop, int bits);
    private native int closeNative();
    /**
     * Called at the time of the class is load
     */
    private static native void  nativeClassInit();
    static{
        System.loadLibrary("barcodeupdate");
        nativeClassInit();
    }


}
