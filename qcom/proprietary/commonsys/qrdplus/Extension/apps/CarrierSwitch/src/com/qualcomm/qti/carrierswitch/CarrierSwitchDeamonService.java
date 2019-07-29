/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.PowerManager;

import java.lang.ref.WeakReference;
import java.util.List;

public class CarrierSwitchDeamonService extends Service {

    private SwitchModule mSwitchModule;

    @Override
    public void onCreate() {
        LogUtil.d(this, "onCreate");
        super.onCreate();
        mSwitchModule = new SwitchModule(this);
    }

    @Override
    public IBinder onBind(Intent intent) {
        LogUtil.d(this, "onBind");
        return mBinder;
    }

    public int getCurrentCarrier() {
        return mSwitchModule.getCurrentCarrier();
    }

    public List<CarrierInfo> getSupportedCarrierList() {

        return mSwitchModule.getSupportedCarrierList();
    }

    public boolean switchCarrier(int oldCarrier, int newCarrier) {
        LogUtil.d(this, "switchCarrier, oldCarrier: "
                + oldCarrier + ", newCarrier: " + newCarrier);
        boolean ret = mSwitchModule.switchCarrier(oldCarrier, newCarrier);

        if (ret) {
            PowerManager power = (PowerManager) getSystemService(Context.POWER_SERVICE);
            power.reboot(null);
        }

        return ret;
    }

    private final IBinder mBinder = new ServiceStub(this);
    static class ServiceStub extends ICarrierSwitchDeamonService.Stub {
        WeakReference<CarrierSwitchDeamonService> mService;

        ServiceStub(CarrierSwitchDeamonService service) {
            mService = new WeakReference<>(service);
        }

        public int getCurrentCarrier() {
            return mService.get().getCurrentCarrier();
        }
        public List<CarrierInfo> getSupportedCarrierList() {
            return mService.get().getSupportedCarrierList();

        }
        public boolean switchCarrier(int oldCarrier, int newCarrier) {
            return mService.get().switchCarrier(oldCarrier, newCarrier);
        }
    }

}
