package com.ipspiry.barcodeupdate;

import android.os.Message;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;

public class MainActivity extends AppCompatActivity {
    private static String TAG="MainActivity";
    private BarCodeSerialUpdatewrapper updatewrapper;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }
    private void initUpgradeModule()
    {
        updatewrapper.setOnEventAvailableListener(new OnEventAvailableListener() {
            public void OnEventAvailable(Message msg){
                Log.d(TAG, "MainActivity:Receive event:"+msg.what);
                if(msg.what == BarCodeSerialUpdatewrapper.EventTypeUpgradeFail ||
                        msg.what == BarCodeSerialUpdatewrapper.EventTypeUpgradeSuccess)
                {
                    try {
                        updatewrapper.disConnectTarget();
                    } catch (Exception e) {
                        e.printStackTrace();
                    }
                }
            }
        });

    }
}
