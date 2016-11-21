package com.ipspiry.barcodeupdate;

import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

public class MainActivity extends AppCompatActivity {
    private static String TAG="MainActivity";
    private SerialConfig mConfig;
    private BarCodeSerialUpdate mUpdate;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mConfig = SerialConfig.getInstance();
        mUpdate = BarCodeSerialUpdate.buildInstance(mConfig);
        assert (mUpdate !=null);
        mUpdate.setOnEventAvailableListener(new BarCodeSerialUpdate.OnEventAvailableListener() {
            public void OnEventAvailable(Message msg){
                Log.d(TAG, "MainActivity:Receive event:"+msg.what);
            }
        });
        mUpdate.getVersion();
        mUpdate.update("/data/update.zip", "/data/md5.zip");

    }
}
