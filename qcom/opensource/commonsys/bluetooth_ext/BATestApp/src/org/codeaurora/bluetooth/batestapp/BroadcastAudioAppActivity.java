/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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

package org.codeaurora.bluetooth.batestapp;

import android.Manifest;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.support.annotation.NonNull;
import android.support.v4.app.ActivityCompat;
import android.util.Log;

import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ToggleButton;
import android.app.Activity;
import org.codeaurora.bluetooth.batestapp.BAAudio;
import android.bluetooth.BluetoothBATransmitter;


public class BroadcastAudioAppActivity extends Activity implements
        CompoundButton.OnCheckedChangeListener, View.OnClickListener {

    //private ToggleButton mBtnBtEnable;
    private ToggleButton mBAEnable;
    private ToggleButton mMegaPhoneEnable;
    private BluetoothAdapter mBtAdapter;
    private String mMsg;
    private static final String TAG = Utils.TAG +"BroadcastAudioAppActivity";
    private final int REQUEST_ENABLE_BT = 1001;
    private final int PERMISSIONS_REQUEST_BLUETOOTH = 1002;
    private Button mBtnBRAEnable;
    private Button mBtnChangeEnc;
    BAAudio mBAAudiobj =  null;
    private boolean isStateChangePending = false;
    private int expectedState = BluetoothBATransmitter.STATE_DISABLED;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate ");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.layout_main);
        mBAEnable = (ToggleButton) findViewById(R.id.id_switch_bt_enable);
        mBAEnable.setOnCheckedChangeListener(this);
        mMegaPhoneEnable =  (ToggleButton) findViewById(R.id.id_switch_mp);
        mMegaPhoneEnable.setOnCheckedChangeListener(this);
        mBtnChangeEnc = (Button) findViewById(R.id.id_btn_enc_change);
        mBtnChangeEnc.setOnClickListener(this);

        mBtnBRAEnable = (Button) findViewById(R.id.id_btn_enable_bra);
        mBtnBRAEnable.setOnClickListener(this);
        mBtnBRAEnable.setEnabled(false);

        mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBtAdapter == null) {
            Log.d(TAG, " Device does not support Bluetooth");
            return;
        }

        boolean isBtEnabled = mBtAdapter.isEnabled();
        if (isBtEnabled) {
            startService(new Intent(BroadcastAudioAppActivity.this, GattBroadcastService.class));
        }

        IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(BluetoothBATransmitter.ACTION_BAT_STATE_CHANGED);
        filter.addAction(BAAudio.BASERVICE_STATE_CHANGED);
        registerReceiver(mReceiver, filter);

        Log.d(TAG, " Creating BAAudio ");
        mBAAudiobj = new BAAudio(getApplicationContext());
        Log.d(TAG, " LauncherActivity  constructor - ");
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, " onDestroy ");
        if (mBAAudiobj != null) {
            mBAAudiobj.cleanup();
        }
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    @Override
    public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {
        Log.d(TAG, " onCheckChanged isChecked = " + isChecked + " button = " + compoundButton
                                                 +" stateChangePending " + isStateChangePending);
            if(mBtAdapter == null) return;
            if (!mBtAdapter.isEnabled()) return;
            if (mBAAudiobj == null) return;

            if (compoundButton == mBAEnable) {
                if (isStateChangePending) return;
                if (isChecked) {
                    if(mBAAudiobj.getBATState() == BluetoothBATransmitter.STATE_DISABLED) {
                        isStateChangePending = true;
                        expectedState = BluetoothBATransmitter.STATE_PAUSED;
                        mBAEnable.setEnabled(false);
                        mBAAudiobj.enableBA();
                    }
                } else {
                   if(mBAAudiobj.getBATState() != BluetoothBATransmitter.STATE_DISABLED) {
                       isStateChangePending = true;
                       expectedState = BluetoothBATransmitter.STATE_DISABLED;
                       mBAEnable.setEnabled(false);
                       mBAAudiobj.disableBA();
                   }
                }
            }
            if (compoundButton == mMegaPhoneEnable) {
                if (isChecked) {
                    boolean started = mBAAudiobj.startRecordAndPlay();
                    setMPButtonText(started);
                } else {
                   mBAAudiobj.stopRecordAndPlay();
                   setMPButtonText(false);
                }
            }
    }

    @Override
    public void onClick(View view) {
        Log.d(TAG, " onClick  view :"+view);
        if (view == mBtnBRAEnable) {
            startActivity(new Intent(BroadcastAudioAppActivity.this,
                    BroadcastAudioDeviceListActivity.class));
        }
        if (view == mBtnChangeEnc) {
            if (mBAAudiobj == null) return;
            mBAAudiobj.refreshEncryptionKey();
        }
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            Log.d(TAG, " Action " + action);

            if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter
                        .ERROR);
                int prevState = intent.getIntExtra(BluetoothAdapter.EXTRA_PREVIOUS_STATE,
                        BluetoothAdapter.ERROR);
                //setButtonText(state);
                if (state == BluetoothAdapter.STATE_TURNING_OFF
                    || state == BluetoothAdapter.STATE_OFF) {
                    setBRAButtonState(BluetoothBATransmitter.STATE_DISABLED);
                }
            }

            if (action.equals(BluetoothBATransmitter.ACTION_BAT_STATE_CHANGED)) {
                final int state = intent.getIntExtra(BluetoothBATransmitter.EXTRA_STATE,
                        -1);
                if(state != -1) {
                    setBRAButtonState(state);
                    Log.d(TAG, " stateChangePending " + isStateChangePending + " expectedState = "
                            +expectedState + " state " + state);
                    if (isStateChangePending) {
                        isStateChangePending = false;
                        mBAEnable.setEnabled(true);
                        if (expectedState != state) {
                            setBAButtonText(state);
                        }
                    } else {
                        // this must have come from state change in stack
                        setBAButtonText(state);
                    }
                }
            }
            if (action.equals(BAAudio.BASERVICE_STATE_CHANGED)) {
                final boolean serviceState = intent.getBooleanExtra(BAAudio.EXTRA_CONN_STATE, false);
                final int baState = intent.getIntExtra(BAAudio.EXTRA_BA_STATE, -1);
                Log.d(TAG, " Service state changed servicState = "
                        + serviceState + " baState = " + baState);
                setBAButtonText(baState);
            }
        }
    };

    private void setMPButtonText(boolean state) {
        Log.d(TAG," setMPButtonText state = " + state);
        if(state) {
            mMegaPhoneEnable.setChecked(true);
        } else {
            mMegaPhoneEnable.setChecked(false);
        }
    }

    private void setBAButtonText(int state) {
        boolean currentCheckStatus = mBAEnable.isChecked();
        Log.d(TAG," setBAButtonText state = " + state + " isChecked() = " + currentCheckStatus);
        if(state == BluetoothBATransmitter.STATE_DISABLED) {
            if (currentCheckStatus) {
                // button was cheked and we are making it false, need to set it
                Log.d(TAG, " setting button false ");
                mBAEnable.setChecked(false);
            }
        }
        else {
            if (!currentCheckStatus) {
               Log.d(TAG, " setting button true ");
               mBAEnable.setChecked(true);
            }
        }
    }

    private void setBRAButtonState(int state) {
        Log.d(TAG, "setBRAButtonState state : "+state);
        if(state == BluetoothBATransmitter.STATE_DISABLED) {
            mBtnBRAEnable.setEnabled(false);
        } else {
            mBtnBRAEnable.setEnabled(true);
        }
    }
}
