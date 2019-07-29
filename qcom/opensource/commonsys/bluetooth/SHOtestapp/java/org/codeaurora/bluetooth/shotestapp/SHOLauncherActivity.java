/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package org.codeaurora.bluetooth.shotestapp;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ToggleButton;
import android.app.Activity;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothProfile;
import android.util.Log;
import android.bluetooth.BluetoothProfile.ServiceListener;
import android.view.View.OnClickListener;
import android.widget.Toast;
import java.util.List;
import android.os.Handler;

public class SHOLauncherActivity extends Activity implements View.OnClickListener {

    private static final String TAG = "SHOTestapp";
    private BluetoothAdapter mAdapter;
    private int dev1_state;
    private int dev2_state;
    private Button mBtnDevice1;
    private Button mBtnDevice2;
    private Context mContext;
    private BluetoothA2dp mA2dp = null;

    private BluetoothProfile.ServiceListener mProfileListener =
         new BluetoothProfile.ServiceListener() {
     public void onServiceConnected(int profile, BluetoothProfile proxy) {
         Log.d(TAG,"onServiceConnected");
         if (profile == BluetoothProfile.A2DP) {
             Log.d(TAG,"onServiceConnected: A2dp");
             mA2dp = (BluetoothA2dp) proxy;
         }
         getdevicelistarray();
     }
     public void onServiceDisconnected(int profile) {
         Log.d(TAG,"onServiceDisconnected");
         if (profile == BluetoothProfile.A2DP) {
             Log.d(TAG,"onServiceDisconnected: A2dp");
             mA2dp = null;
        }
     }
   };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG,"onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.layout_main);
        dev1_state = 0;
        dev2_state = 0;
        mBtnDevice1 = (Button) findViewById(R.id.id_btn_hs1);
        mBtnDevice1.setEnabled(false);
        mBtnDevice2 = (Button) findViewById(R.id.id_btn_hs2);
        mBtnDevice2.setEnabled(false);
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mAdapter.getProfileProxy(getApplicationContext(), mProfileListener, BluetoothProfile.A2DP);
        IntentFilter filter = new IntentFilter(BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        registerReceiver(mReceiver, filter);
    }


private void getdevicelistarray() {
if(mA2dp != null){
   List<BluetoothDevice> deviceList = mA2dp.getConnectedDevices();
    if (!deviceList.isEmpty()) {
       Log.d(TAG,"Device List updated");
            for (BluetoothDevice dev : deviceList)
              if(device_not_in_list(dev))
                 addDevice(dev);
            }
    }
else
    Log.d(TAG,"mA2dp NULL");
}

public boolean device_not_in_list(BluetoothDevice device){
       String data = (device != null) ? device.getName() : null;
       String data_button1 = (mBtnDevice1.getTag() != null) ? ((BluetoothDevice) mBtnDevice1.getTag()).getName() : null;
       String data_button2 = (mBtnDevice2.getTag() != null) ? ((BluetoothDevice) mBtnDevice2.getTag()).getName() : null;
       if(data.equals(data_button1)  ||  data.equals(data_button2))
        return false;
       else
        return true;
}

@Override
   protected void onResume() {
      super.onResume();
      Log.d(TAG, "The onResume() event");
      getdevicelistarray();
   }


    @Override
    protected void onDestroy() {
        Log.d(TAG,"onDestroy");
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    @Override
    public void onClick(View view) {
        if (view == mBtnDevice1) {
           Log.d(TAG,"Click detected1");
           mA2dp.selectStream(((BluetoothDevice) mBtnDevice1.getTag()));
           mBtnDevice1.setEnabled(false);
           new Handler().postDelayed(new Runnable() {
        @Override
         public void run() {
            mBtnDevice2.setEnabled(true);
           }
           }, 1500);
        }
        if (view == mBtnDevice2) {
          Log.d(TAG,"Click detected2");
         mA2dp.selectStream(((BluetoothDevice) mBtnDevice2.getTag()));
         mBtnDevice2.setEnabled(false);
         new Handler().postDelayed(new Runnable() {
        @Override
         public void run() {
            mBtnDevice1.setEnabled(true);
           }
           }, 1500);
        }
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            if (action.equals(BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED)) {
                Log.d(TAG,"Received ACTION_CONNECTION_STATE_CHANGED");
                int state = intent.getIntExtra(BluetoothA2dp.EXTRA_STATE, -1);
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                if (state == BluetoothA2dp.STATE_DISCONNECTED) {
                   Log.d(TAG,"A2dp Disconnected");
                    if (device != null)  removeDevice(device);
                }
                getdevicelistarray();
            }
                if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                  Log.d(TAG,"Received ACTION_STATE_CHANGED");
               int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter.ERROR);
               if (!((BluetoothAdapter.STATE_ON == state) || (BluetoothAdapter.STATE_TURNING_ON == state))){
                  mBtnDevice1.setEnabled(false);
                  mBtnDevice1.setText("HS1");
                  mBtnDevice1.setTag(null);
                  dev1_state=0;
                  mBtnDevice2.setEnabled(false);
                  mBtnDevice2.setText("HS2");
                  mBtnDevice2.setTag(null);
                  dev2_state=0;
               }
            }
        }
    };

    private void addDevice(BluetoothDevice device) {
        String data = (device != null) ? device.getName() : null;
        BluetoothDevice attribute =device;
        if (dev1_state==0) {
            mBtnDevice1.setEnabled(true);
            mBtnDevice1.setOnClickListener(this);
            mBtnDevice1.setText(data);
            mBtnDevice1.setTag(attribute);
            dev1_state=1;
             Log.d(TAG,"Name displayed1");
        }
        else if (dev2_state==0) {
            mBtnDevice2.setEnabled(true);
            mBtnDevice2.setOnClickListener(this);
            mBtnDevice2.setText(data);
            mBtnDevice2.setTag(attribute);
            dev2_state=1;
             Log.d(TAG,"Name displayed2");
        }
        else
            Toast.makeText(getApplicationContext(), "Already 2 devices connected!",Toast.LENGTH_LONG).show();
    }

    private void removeDevice(BluetoothDevice device) {
       String data = (device != null) ? device.getName() : null;
       String data_button1 = (mBtnDevice1.getTag() != null) ? ((BluetoothDevice) mBtnDevice1.getTag()).getName() : null;
       String data_button2 = (mBtnDevice2.getTag() != null) ? ((BluetoothDevice) mBtnDevice2.getTag()).getName() : null;
       if((dev1_state==1) && data.equals(data_button1) && (device!=null)) {
            mBtnDevice1.setEnabled(false);
            mBtnDevice1.setText("HS1");
            dev1_state=0;
            mBtnDevice1.setTag(null);
       }
       else if((dev2_state==1) && data.equals(data_button2) && (device!=null)) {
            mBtnDevice2.setEnabled(false);
            mBtnDevice2.setText("HS2");
            dev2_state=0;
            mBtnDevice2.setTag(null);
       }
       else
         Toast.makeText(getApplicationContext(), "Error removing device!",Toast.LENGTH_LONG).show();
    }
}




