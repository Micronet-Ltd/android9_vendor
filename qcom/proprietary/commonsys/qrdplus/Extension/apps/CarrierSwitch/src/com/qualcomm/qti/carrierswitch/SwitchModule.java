/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch;

import android.content.Context;
import android.content.SharedPreferences;
import android.preference.PreferenceManager;

import com.qualcomm.qti.carrierswitch.switchaction.ApplicationSwitcher;
import com.qualcomm.qti.carrierswitch.switchaction.OverlaySwitcher;

import java.util.ArrayList;
import java.util.List;

public class SwitchModule {

    public static int CARRIER_MODE_DEFAULT = 0;
    public static int CARRIER_MODE_CHINAMOBILE = 1;
    public static int CARRIER_MODE_CHINATELECOM = 2;

    private Context mContext;

    private List<CarrierInfo> mCarrierInfos = new ArrayList<>();

    private List<ISwitcher> mSwitchers = new ArrayList<>();


    SwitchModule(Context context) {
        mContext = context;

        mCarrierInfos.add(new CarrierInfo(CARRIER_MODE_DEFAULT,
                "Default", R.string.carrier_default));
        mCarrierInfos.add(new CarrierInfo(CARRIER_MODE_CHINAMOBILE,
                "CMCC", R.string.carrier_cmcc));
        mCarrierInfos.add(new CarrierInfo(CARRIER_MODE_CHINATELECOM,
                "CT", R.string.carrier_ct));

        mSwitchers.add(new OverlaySwitcher(context));
        mSwitchers.add(new ApplicationSwitcher(context));
    }


    public List<CarrierInfo> getSupportedCarrierList() {
        return mCarrierInfos;
    }

    public boolean switchCarrier(int oldCarrier, int newCarrier) {
        LogUtil.d(this, "SwitchModule, switchCarrier oldCarrier: "
                + oldCarrier + ", newCarrier: " + newCarrier);
        if (oldCarrier == newCarrier) return true;
        LogUtil.d(this, "SwitchModule, switchCarrier 2 oldCarrier: "
                + oldCarrier + ", newCarrier: " + newCarrier);
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mContext);
        sp.edit().putInt("mode", newCarrier).apply();

        for (ISwitcher switcher: mSwitchers) {
            if (!switcher.executeNow(oldCarrier, newCarrier)) {
                LogUtil.d(this, "SwitchModule, executeNow failed");
            }
        }
        return true;
    }

    public int getCurrentCarrier() {
        return loadCarrier();
    }

    private int loadCarrier() {
        SharedPreferences sp = PreferenceManager.getDefaultSharedPreferences(mContext);
        int id = sp.getInt("mode", mContext.getResources().getInteger(R.integer.config_initMode));
        LogUtil.d(this, "getCurrentCarrier id = " + id);
        return id;
    }
}
