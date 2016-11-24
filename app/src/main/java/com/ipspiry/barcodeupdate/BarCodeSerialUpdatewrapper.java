package com.ipspiry.barcodeupdate;

import android.os.Message;
import android.util.Log;

/**
 * Created by xuss on 2016/11/24.
 */
public class BarCodeSerialUpdatewrapper {
    /**
     * Event define
     */
    /**
     * what =EventTypeUpgradeSuccess
     * arg1 = 0
     * arg2 = 0
     */
    public static int EventTypeUpgradeSuccess=0x01000001;
    /**
     *  what = EventTypeUpgradeFail
     *  arg1 = error code
     *  arg2 = 0
     */

    public static int EventTypeUpgradeFail=0x01000002;
    /**
     * what = EventTYpeUpgradeProgress
     * arg1 = (int)percent * 100
     * arg2 = 0
     */
    public static int EventTYpeUpgradeProgress=0x01000003;

    private static String TAG = "BarCodeSerialUpdatewrapper";
    private static SerialConfig gConfig = SerialConfig.getInstance();
    private static BarCodeSerialUpdate gUpdate = BarCodeSerialUpdate.getInstance();
    private static OnEventAvailableListener gListener = null;
    private static boolean gConnected= false;
    private static boolean gIsBusy = false;

    static boolean isConnected(){
        return gConnected;
    }
    static boolean isBusy(){
        return gIsBusy;
    }
    static boolean setBaudrate(int baudrate)
    {
        synchronized(BarCodeSerialUpdatewrapper.class) {
            if (gConnected) {
                Log.e(TAG, "Can not set baud rate after the device is connected");
                return false;
            } else {
                gConfig.setBaudrate(baudrate);
                Log.d(TAG, "Set baud rate("+baudrate+") sucess");
                return true;
            }
        }
    }
    static boolean setDeviceName(String path)
    {
        synchronized(BarCodeSerialUpdatewrapper.class){
            if(gConnected){
                Log.e(TAG, "Can not set device path after the device is connected");
                return false;
            }else{
                gConfig.setPath(path);
                Log.d(TAG, "Set device path("+path+") sucess");
                return true;
            }
        }
    }
    static boolean saveConfig()
    {
        gConfig.SaveConfig();
        return true;
    }
    static boolean connectTarget() throws Exception {
        synchronized(BarCodeSerialUpdatewrapper.class) {
            if(gConnected)
            {
                Log.e(TAG, "Can not connect target , it is connected");
                return false;
            }
            gUpdate.setConfig(gConfig);
            gUpdate.checkSecurity(gConfig.getPath());
            if (!gUpdate.open()) {
                Log.e(TAG, "Connect error!!!");
                gConnected =  false;
                throw new Exception();
            } else {
                Log.d(TAG, "Connect target success");
                gConnected = true;
                /**
                 * 设置内部事件转发器
                 */
                gUpdate.setOnEventAvailableListener( new OnEventAvailableListener() {
                    public void OnEventAvailable(Message msg) {
                        synchronized(BarCodeSerialUpdatewrapper.class)
                        {
                            if(msg.what <=EventTypeUpgradeFail) {
                                gIsBusy = false;
                            }
                        }
                        if(gListener != null)
                            gListener.OnEventAvailable(msg);
                    }
                });
            }
        }
        return gConnected;
    }
    static boolean disConnectTarget()throws Exception
    {
        synchronized(BarCodeSerialUpdatewrapper.class){
            if(!gConnected || gIsBusy){
                Log.d(TAG, "Can not disconnect target, nether the target is not connected nor is busy");
                return false;
            }
            if(!gUpdate.close()) {
                throw new Exception();
            }
            gConnected = false;
            gIsBusy = false;
            return true;
        }
    }
    static void setOnEventAvailableListener(OnEventAvailableListener l)
    {
        synchronized(BarCodeSerialUpdatewrapper.class)
        {
            gListener = l;
        }
    }

    static boolean upgradeTarget(String packagefile, String md5file) throws Exception
    {
        synchronized(BarCodeSerialUpdatewrapper.class){
           if(!gConnected || gIsBusy)
           {
               Log.d(TAG, "Can not upgrade target, nether the target is not connected nor is busy");
               return false;
           }
            else
           {
               Log.d(TAG, "upgrad target packagefile "+packagefile +" md5file"+md5file);
               if(!gUpdate.update(packagefile, md5file)){
                   throw new Exception();
               }
               gIsBusy = true;
           }
            return true;
        }
    }
    static String getTargetVersion()
    {
        synchronized(BarCodeSerialUpdatewrapper.class){
            if(!gConnected)
            {
                Log.d(TAG, "Can not get target version for it is disconnected");
                return null;
            }
            else
            {
                return gUpdate.getVersion();
            }
        }
    }

    static boolean checkPckage(String packagefile, String md5file)
    {
        return true;
    }

}
