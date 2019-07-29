/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.IBinder;
import android.os.RemoteException;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.RadioButton;
import android.widget.RadioGroup;
import android.widget.Toast;

import java.util.List;


public class MainActivity extends Activity {
    private RadioGroup mCarrierListGroup;
    private ICarrierSwitchDeamonService mCarrierSwitchDeamonService;
    private int mCurrentId = 0;

    private ServiceConnection mServiceConnection = new ServiceConnection() {

        public void onServiceConnected(ComponentName className, IBinder service) {
            LogUtil.d(MainActivity.this, "onServiceConnected");
            mCarrierSwitchDeamonService =
                    ICarrierSwitchDeamonService.Stub.asInterface(service);
            updateRadioGroup();
        }

        public void onServiceDisconnected(ComponentName className) {
            mCarrierSwitchDeamonService = null;
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        LogUtil.d(this, "onCreate");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        initVew();
        bindService();

    }

    private void initVew() {
        mCarrierListGroup = findViewById(R.id.carrier_list_radio);
    }

    private void updateRadioGroup() {
        if (mCarrierSwitchDeamonService == null) return;
        try {
            List<CarrierInfo> carriers =
                    mCarrierSwitchDeamonService.getSupportedCarrierList();
            LogUtil.d(this, "updateRadioGroup = " + carriers);
            //mCarrierListGroup.removeAllViews();
            for (final CarrierInfo carrier : carriers) {
                RadioButton btn = (RadioButton) getLayoutInflater().inflate(
                        R.layout.radio_button, null, true);
                btn.setId(carrier.id);
                btn.setText(getResources().getString(carrier.resId));
                btn.setOnClickListener(new View.OnClickListener() {
                    @Override
                    public void onClick(View view) {
                        LogUtil.d(MainActivity.this, "RadioButton onChecked: " + carrier.id);
                        if (ActivityManager.isUserAMonkey()) {
                            return;
                        }
                        final int carrierId = carrier.id;
                        new AsyncTask<Void, Void, Boolean>() {
                            @Override
                            protected Boolean doInBackground(Void[] objects) {
                                boolean result = false;
                                try {
                                    if (mCarrierSwitchDeamonService != null) {
                                        result = mCarrierSwitchDeamonService.switchCarrier(
                                                mCurrentId, carrierId);
                                        mCurrentId = carrierId;
                                    }
                                } catch (RemoteException e) {
                                    e.printStackTrace();
                                }
                                return result;
                            }

                            @Override
                            protected void onPostExecute(Boolean result) {
                                Toast.makeText(MainActivity.this,
                                        result ? R.string.switch_success : R.string.switch_failed,
                                        Toast.LENGTH_LONG).show();
                            }
                        }.execute();
                    }
                });
                mCarrierListGroup.addView(btn,
                        LinearLayout.LayoutParams.MATCH_PARENT,
                        LinearLayout.LayoutParams.WRAP_CONTENT);
            }
            mCurrentId = mCarrierSwitchDeamonService.getCurrentCarrier();
            mCarrierListGroup.check(mCurrentId);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    private void bindService() {
        if (mCarrierSwitchDeamonService == null) {
            Intent intent = new Intent(this, CarrierSwitchDeamonService.class);
            bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);
        }
    }
}
