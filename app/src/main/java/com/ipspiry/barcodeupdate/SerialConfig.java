package com.ipspiry.barcodeupdate;

/**
 * Created by xuss on 2016/11/20.
 */
public class SerialConfig {
    /*
   * default SerialPort parameters
    */
    private String mPath="/dev/ttyS0";
    private int mBaudrate = 115200;
    private int mParity = 0;
    private int mStop = 1;
    private int mBits = 8;
    private static SerialConfig mInstace = null;
    public static SerialConfig getInstance(){
        if(mInstace == null)
        {
            return mInstace = new SerialConfig();
        }
        return mInstace;
    }
    /*
        Get Method
     */
    public String getPath(){
        return mPath;
    }
    public  int getBaudrate(){
        return mBaudrate;
    }
    public int getParity(){
        return mParity;
    }
    public int getStop(){
        return mStop;
    }
    public int getBits(){
        return mBits;
    }
    /*
        Set Method
     */
    public String setPath(String path){
        String old = mPath;
        mPath = path;
        return old;
    }
    public  int setBaudrate(int baudrate){
        int old = mBaudrate;
        mBaudrate = baudrate;
        return old;
    }
    public int setParity(int parity){
        int old = mParity;
        mParity = parity;
        return old;
    }
    public int setStop(int stop){
        int old = mStop;
        mStop = stop;
        return old;
    }
    public int setBits(int bits){
        int old = bits;
        mBits = bits;
        return old;
    }
    /*
        Load Parameter From BackUp File
     */
    private int loadParameters(){
        return 0;
    }
    /*
        Save Parameters To File
     */
    public int SaveConfig()
    {
        return 0;
    }
    private SerialConfig(){
        loadParameters();
    }
}
