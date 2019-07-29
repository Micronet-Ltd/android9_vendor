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
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.ScanResult;
import android.bluetooth.le.ScanRecord;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.content.ServiceConnection;

import android.os.Bundle;
import android.os.Message;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Binder;
import android.os.IBinder;
import android.os.RemoteException;

import android.util.Log;
import android.support.annotation.NonNull;
import android.app.Activity;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.Button;

import org.codeaurora.bluetooth.batestapp.IGattBroadcastService;
import org.codeaurora.bluetooth.batestapp.IGattBroadcastServiceCallback;
import org.codeaurora.bluetooth.batestapp.BroadcastAudioDevice;

import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.CountDownLatch;
import java.util.Collections;

public class BroadcastAudioDeviceListActivity extends Activity implements View.OnClickListener,
        RecyclerViewListener {

    private static final String TAG = Utils.TAG + "BroadcastAudioDeviceListActivity";
    GattBroadcastServiceClientHandler mGattBroadcastServiceClientHandler;
    boolean mIsCountdownLatchEnabled = false;
    boolean mIsAssociationInProgress = false;
    boolean mIsBRAEnabled = false;
    private RecyclerView mRvDevices;
    private AdapterDevice mAdapterDevice;
    BroadcastAudioDevice mDevice;
    private List<BroadcastAudioDevice> mList;
    private BluetoothAdapter mBtAdapter;
    private Context mCtx;
    private Button mBtnBack;
    private CountDownLatch mDeinitSignal = new CountDownLatch(1);
    private boolean mIsCleanupCompleted = false;

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
                if (state == BluetoothAdapter.STATE_TURNING_OFF) {
                    cleanup();
                    finish();
                }
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, " oncreate:");
        super.onCreate(savedInstanceState);
        setContentView(R.layout.layout_devices);
        mRvDevices = (RecyclerView) findViewById(R.id.id_lv_deviceslist);
        mRvDevices.setLayoutManager(new LinearLayoutManager(this));
        mList = Collections.synchronizedList(new ArrayList<BroadcastAudioDevice>());
        mAdapterDevice = new AdapterDevice(mList, getLayoutInflater(), this);
        mRvDevices.setAdapter(mAdapterDevice);
        mBtnBack = (Button) findViewById(R.id.id_btn_back);
        mBtnBack.setOnClickListener(this);
        mBtnBack.setEnabled(true);
        mCtx = getApplicationContext();
        mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
        registerReceiver(mReceiver, filter);

        HandlerThread thread = new HandlerThread("GattBroadcastServiceClientHandler");
        thread.start();
        Looper looper = thread.getLooper();

        mGattBroadcastServiceClientHandler = new GattBroadcastServiceClientHandler(this, looper);


    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.v(TAG, "onDestroy: mReceiver" + mReceiver + " mIsCleanupCompleted: "
                + mIsCleanupCompleted);
        if (!mIsCleanupCompleted){
            cleanup();
        }
        if (mReceiver != null) {
            unregisterReceiver(mReceiver);
        }
    }

    private void cleanup() {
        Log.v(TAG, " cleanup: mIsBRAEnabled: " + mIsBRAEnabled
                + " mGattBroadcastServiceClientHandler: " + mGattBroadcastServiceClientHandler);
        if (mGattBroadcastServiceClientHandler != null) {
            if (mIsBRAEnabled) {
                mBtnBack.setEnabled(false);
                mGattBroadcastServiceClientHandler.sendMessage(
                        mGattBroadcastServiceClientHandler.obtainMessage(
                                GattBroadcastServiceClientHandler
                                        .CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION,
                                false));
            }
            mGattBroadcastServiceClientHandler.sendMessage(
                    mGattBroadcastServiceClientHandler.obtainMessage(
                            GattBroadcastServiceClientHandler.UNREGISTER_CALLBACK));
            mIsCountdownLatchEnabled = true;
            Log.i(TAG, "Waiting for deinit to complete");
            try {
                mDeinitSignal.await();
            } catch (InterruptedException e) {
                Log.e(TAG, "Interrupt received while waitinf for de-init to complete", e);
            }

            mGattBroadcastServiceClientHandler.close();
            mGattBroadcastServiceClientHandler.removeCallbacksAndMessages(null);
            Looper looper = mGattBroadcastServiceClientHandler.getLooper();
            if (looper != null) {
                looper.quit();
                Log.i(TAG, "Quit looper");
            }
            mGattBroadcastServiceClientHandler = null;
            Log.i(TAG, "Remove Handler");
            mIsCleanupCompleted = true;
        }
    }

    @Override
    public void onClick(View view) {
        Log.v(TAG, " onClick ");
        cleanup();
        finish();
    }

    @Override
    public void onRecylerViewItemClicked(int clickedPos) {
        Log.v(TAG, "onRecylerViewItemClicked position : " + clickedPos);
        BroadcastAudioDevice device = mList.get(clickedPos);
        String msg = getString(R.string.msg_association_initiated, device.getName());
        Log.v(TAG, "onRecylerViewItemClicked device name  : " + msg);

        if (mIsAssociationInProgress) {
            Log.d(TAG, "Association is in progress ignore click");
            return;
        }

        if (mGattBroadcastServiceClientHandler != null) {
            Utils.toast(mCtx, msg);
            Log.d(TAG, " sendMessage ASSOCIATE_BCA_RECEIVER" );
            mGattBroadcastServiceClientHandler.sendMessage(
                    mGattBroadcastServiceClientHandler.obtainMessage(
                            GattBroadcastServiceClientHandler.ASSOCIATE_BCA_RECEIVER,
                            device.getBluetoothDevice()));
            mIsAssociationInProgress = true;
            mDevice = device;
        } else {
            Log.e(TAG, "mGattBroadcastServiceClientHandler is null");
        }
        Log.v(TAG, "onRecylerViewItemClicked position end ");
    }

    private synchronized void  addOrRemoveBCAReceiverDevice(BroadcastAudioDevice device,
            boolean isAdd) {

        boolean isFound = false;
        BluetoothDevice bDevice = device.getBluetoothDevice();
        Log.d(TAG, " addOrRemoveBCAReceiverDevice: isAdd: " + isAdd);
        for (BroadcastAudioDevice BADev : mList) {
            if (bDevice.equals(BADev.getBluetoothDevice())) {
                isFound = true;
                if (!isAdd) {
                    Log.d(TAG, "addOrRemoveBCAReceiverDevice Device removed :"
                            + BADev.getName());
                    mList.remove(BADev);
                }
                break;
            }
        }

        if (isAdd && !isFound) {
            Log.d(TAG, " New Device Added :" + device.getName());
            mList.add(device);
        }
    }

    private synchronized void refreshDevices() {
        Log.d(TAG, " refreshDevices size:" + mList.size());
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAdapterDevice.refreshDevices(mList);
            }
        });

    }

    public class GattBroadcastServiceClientHandler extends Handler {
        public static final int REGISTER_CALLBACK = 0;
        public static final int UNREGISTER_CALLBACK = 1;
        public static final int CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION = 2;
        public static final int ASSOCIATE_BCA_RECEIVER = 3;
        private Context mContext;
        private IGattBroadcastService mService = null;

        private final ServiceConnection mConnection = new ServiceConnection() {
            public void onServiceConnected(ComponentName className, IBinder service) {
                Log.d(TAG, "gatt Broadcast Proxy object connected");
                mService = IGattBroadcastService.Stub.asInterface(Binder.allowBlocking(service));
                mGattBroadcastServiceClientHandler.sendMessage(
                        mGattBroadcastServiceClientHandler.obtainMessage(
                                REGISTER_CALLBACK));
                mGattBroadcastServiceClientHandler.sendMessage(
                        mGattBroadcastServiceClientHandler.obtainMessage(
                                CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION, true));
            }

            public void onServiceDisconnected(ComponentName className) {
                Log.d(TAG, "gatt Broadcast  Proxy object disconnected");
                mIsBRAEnabled = false;
                mIsAssociationInProgress = false;
                mService = null;
            }
        };

        private IGattBroadcastServiceCallback.Stub mServiceCallbacks =
                new IGattBroadcastServiceCallback.Stub() {
            public void onFoundOnLostBCAReceiver(ScanResult result, boolean isFound) {
                ScanRecord record = result.getScanRecord();
                String mac = result.getDevice().getAddress();

                Log.d(TAG, "onFoundOnLostBCAReceiver: " + isFound);
                Log.d(TAG, "onFoundOnLostBCAReceiver device address: " + mac);
                Log.d(TAG, "onFoundOnLostBCAReceiver: scanRecord: " + record);

                BroadcastAudioDevice device = new BroadcastAudioDevice(result.getDevice(),
                        result.getScanRecord());

                addOrRemoveBCAReceiverDevice(device, isFound);
                refreshDevices();
            }

            public void onConfiguredBroadcastReceiverAssociation(int status) {
                Log.d(TAG, "onConfiguredBroadcastReceiverAssociation status: " + status);
                if (status == GattBroadcastService.BRA_ENABLED_SUCESSS) {
                    mIsBRAEnabled = true;
                } else if (status == GattBroadcastService.BRA_DISABLED_SUCESSS) {
                    mIsBRAEnabled = false;
                } else if (status == GattBroadcastService.BRA_ENABLED_FAILED) {
                    finish();
                } else if (status == GattBroadcastService.BRA_DISABLED_FAILED) {
                    mIsBRAEnabled = false;
                }
            }

            public void onAssociatedBCAReceiver(BluetoothDevice device, int status) {
                Log.d(TAG, "onAssociatedBCAReceiver: status " + status);
                if (mDevice == null){
                    Log.d(TAG, "onAssociatedBCAReceiver: Nothing to show on UI, mDevice is null");
                    return;
                }

                if (status == GattBroadcastService.ASSOCIATE_BCA_RECEIVER_SUCCESS) {
                    String msg = getString(R.string.msg_association_success, mDevice.getName());
                    Utils.toast(mCtx, msg);
                    List<BroadcastAudioDevice> tempList =
                            new ArrayList<BroadcastAudioDevice>(mList);
                    for (BroadcastAudioDevice BADev : tempList) {
                        if (device.equals(BADev.getBluetoothDevice())) {
                            addOrRemoveBCAReceiverDevice(BADev, false);
                        }
                    }
                    refreshDevices();
                } else if (status == GattBroadcastService.ASSOCIATE_BCA_RECEIVER_FAILED) {
                    String msg = getString(R.string.msg_association_failed, mDevice.getName());
                    Utils.toast(mCtx, msg);
                }
                mIsAssociationInProgress = false;
            }
        };

        /**
         * Create a GattBroadcastService proxy object for interacting with the local
         * Bluetooth Service which handles the GattBroadcastService Profile
         */
        private GattBroadcastServiceClientHandler(Context context, Looper looper) {
            super(looper);
            mContext = context;
            doBind();
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "Handler(): got msg=" + msg.what);

            switch (msg.what) {
                case REGISTER_CALLBACK:
                    registerCallbacks();
                    break;
                case UNREGISTER_CALLBACK:
                    deRegisterCallbacks();
                    if (mIsCountdownLatchEnabled) {
                        mDeinitSignal.countDown();
                        mIsCountdownLatchEnabled = false;
                    }
                    break;
                case CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION:
                    boolean enable = (boolean) msg.obj;
                    configureBroadcastReceiverAssociation(enable);
                    break;
                case ASSOCIATE_BCA_RECEIVER:
                    BluetoothDevice device = (BluetoothDevice) msg.obj;
                    associateBCAReceiver(device);
                    break;
                default:
                    break;
            }
        }

        boolean doBind() {
            Log.v(TAG, " doBind ");
            Intent intent = new Intent().setClass(mContext, GattBroadcastService.class);
            if (!mContext.bindServiceAsUser(intent, mConnection, 0,
                    android.os.Process.myUserHandle())) {
                Log.e(TAG, "Could not bind to Bluetooth gatt Service with intent" + intent);
                return false;
            }
            return true;
        }

        void close() {
            Log.v(TAG, " close ");
            synchronized (mConnection) {
                if (mService != null) {
                    try {
                        mContext.unbindService(mConnection);
                        mService = null;
                    } catch (Exception re) {
                        Log.e(TAG, "", re);
                    }
                }
            }
        }

        private boolean associateBCAReceiver(BluetoothDevice device) {
            Log.v(TAG, " associateBCAReceiver ");
            try {
                if (mService != null) {
                    mService.associateBCAReceiver(device);
                    return true;
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Failed to associate BCA Receiver " + e);
            }
            return false;
        }

        private boolean registerCallbacks() {
            Log.v(TAG, " registerCallbacks ");
            try {
                if (mService != null) {
                    mService.registerCallbacks(mServiceCallbacks);
                    return true;
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Failed to register Callbacks" + e);
            }
            return false;
        }

        private boolean deRegisterCallbacks() {
            Log.v(TAG, " deRegisterCallbacks ");
            try {
                if (mService != null) {
                    mService.deRegisterCallbacks(mServiceCallbacks);
                    return true;
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Failed to deRegister Callbacks" + e);
                return false;
            }
            return false;
        }

        private boolean configureBroadcastReceiverAssociation(boolean enable) {
            Log.v(TAG, " configureBroadcastReceiverAssociation ");

            try {
                if (mService != null) {
                    mService.configureBroadcastReceiverAssociation(enable);
                    return true;
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Failed to configure Broadcast Receiver Association" + e);
            }
            return false;
        }
    }
}
