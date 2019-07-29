/*
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.networksetting;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.os.Binder;
import android.os.Build;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.os.UserManager;
import android.os.ServiceManager;
import android.preference.PreferenceManager;
import android.provider.Settings;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.ims.ImsManager;
import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.TelephonyIntents;
import org.codeaurora.internal.IExtTelephony;
import java.util.ArrayList;

/**
 * Service code used to assist in monitor operator card and set Enhanced 4G LTE
 * switch button status.
 */
public class Enhanced4GLteStatusService extends Service {
    //debug data
    private static final String LOG_TAG = "Enhanced4GLteStatusService";
    private static final boolean DBG = true;

    private static final int PROVISIONED = 1;
    private static final int INVALID_STATE = -1;

    private int mPhoneCount;
    private boolean[] mIccIdChanged;
    private boolean[] mIccIdReady;
    private SubscriptionManager mSubscriptionManager = null;

    private static final String SETTING_USER_PREF_DATA_SUB = "user_preferred_data_sub";
    private int mUserPrefDataSubId;
    private static SharedPreferences mSharePref;
    private static boolean mUnlocked = false;

    private static int getUiccProvisionStatus(int slotId) {
        try {
            IExtTelephony extTelephony =
                IExtTelephony.Stub.asInterface(ServiceManager.getService("extphone"));
            //get current provision state of the SIM.
            return extTelephony.getCurrentUiccCardProvisioningStatus(slotId);
        } catch (RemoteException | NullPointerException ex) {
            log("Failed to get pref, slotId: "+ slotId +" Exception: " + ex);
        }
        return INVALID_STATE;
    }

    static private boolean isUiccProvisioned(int slotId) {
        return getUiccProvisionStatus(slotId) == PROVISIONED;
    }

    private static int getUserPrefDataSubIdFromDB(Context context) {
        return android.provider.Settings.Global.getInt(context.getContentResolver(),
                SETTING_USER_PREF_DATA_SUB, SubscriptionManager.INVALID_SUBSCRIPTION_ID);
    }

    @Override
    public void onCreate() {
        log("onCreate()");
        TelephonyManager tm = (TelephonyManager) this.getSystemService(Context.TELEPHONY_SERVICE);
        mPhoneCount = tm.getPhoneCount();
        mIccIdChanged = new boolean[mPhoneCount];
        mIccIdReady = new boolean[mPhoneCount];
        mUserPrefDataSubId = getUserPrefDataSubIdFromDB(this);
        mSharePref = getDefaultSharedPreferences(this);
        for (int i = 0; i < mPhoneCount; i++) {
            mIccIdChanged[i] = false;
            mIccIdReady[i] = false;
            handleSimStateChangedEvt(this, i);
        }

        mSubscriptionManager = SubscriptionManager.from(this);
        IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(TelephonyIntents.ACTION_SIM_STATE_CHANGED);
        intentFilter.addAction(Intent.ACTION_USER_UNLOCKED);
        intentFilter.addAction(Intent.ACTION_BOOT_COMPLETED);
        if (TelephonyManager.getDefault().getPhoneCount() > 1) {
            intentFilter.addAction(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED);
        }
        registerReceiver(mBroadcastReceiver, intentFilter);
    }

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            log("onReceive intent : " + intent);
            if(TelephonyIntents.ACTION_DEFAULT_DATA_SUBSCRIPTION_CHANGED
                    .equals(intent.getAction())) {
                if (!isUiccProvisioned(PhoneConstants.SUB1)
                        || !isUiccProvisioned(PhoneConstants.SUB2)) {
                    log("SIM is not provisioned");
                    return;
                }

                int userPrefDataSubId = getUserPrefDataSubIdFromDB(context);
                if (!SubscriptionManager.isValidSubscriptionId(userPrefDataSubId)) {
                    log("userPrefDataSubId is INVALID. ");
                    return;
                }
                log("userPrefDataSubId : " + userPrefDataSubId
                        + ", old pref Data subId : " + mUserPrefDataSubId);
                if (userPrefDataSubId == mUserPrefDataSubId) {
                    log("Not user pref DDS.");
                    return;
                }
                mUserPrefDataSubId = userPrefDataSubId;
                if (isCtCard(PhoneConstants.SUB1) && isCtCard(PhoneConstants.SUB2)) {
                    log("DDS switch with 2 CT cards inserted, update the enhanced 4G LTE status");
                    for (int i = 0; i < mPhoneCount; i++) {
                        mIccIdChanged[i] = true;
                    }
                    updateEnhanced4GLteStatusForMultiSim();
                }
            } else if (TelephonyIntents.ACTION_SIM_STATE_CHANGED.equals(intent.getAction())) {
                int phoneId = intent.getIntExtra(PhoneConstants.PHONE_KEY,
                        SubscriptionManager.INVALID_PHONE_INDEX);
                String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
                log("phoneId = " + phoneId +", stateExtra = " + stateExtra);
                if (!(IccCardConstants.INTENT_VALUE_ICC_READY.equals(stateExtra) ||
                            IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra) ||
                            IccCardConstants.INTENT_VALUE_ICC_IMSI.equals(stateExtra))) {
                    log("iccid is not ready");
                    mIccIdReady[phoneId] = false;
                    return;
                }
                if (phoneId == SubscriptionManager.INVALID_PHONE_INDEX) {
                    log("this is INVALID_PHONE_INDEX");
                    return;
                }
                handleSimStateChangedEvt(context, phoneId);
            } else if (Intent.ACTION_BOOT_COMPLETED.equals(intent.getAction())
                    || Intent.ACTION_USER_UNLOCKED.equals(intent.getAction())) {
                deviceReadyToSyncData(context);
            }
        }
    };

    private void deviceReadyToSyncData(Context context) {
        if (getSystemService(UserManager.class).isUserUnlocked() && !mUnlocked) {
            mUnlocked = true;
            for (int i = 0; i < mPhoneCount; i++) {
                saveToCredentialProtectedStorage(context, i,
                        mSharePref.getString("PreIccId_" + i , null));
            }
            int defaultDataSlot = SubscriptionManager
                .getSlotIndex(SubscriptionManager.getDefaultDataSubscriptionId());
            saveDdsSlotToCs(context, defaultDataSlot);
        }
    }

    private void handleSimStateChangedEvt(Context context, int phoneId) {
        log("handleSimStateChangedEvt phoneid :" + phoneId);
        int subId[] = SubscriptionManager.getSubId(phoneId);
        if (subId != null && subId.length > 0) {
            SubscriptionInfo subInfo =
                SubscriptionManager.from(context).getActiveSubscriptionInfo(subId[0]);
            log("subInfo : " + subInfo);
            if (subInfo != null ) {
                String iccId = subInfo.getIccId();
                mIccIdReady[phoneId] = true;
                log("iccid is : " + iccId );
                if (isCardsInfoChanged(context, phoneId, iccId)){
                    mIccIdChanged[phoneId] = true;
                    saveCardIccIdInfoInSp(context, phoneId, iccId);
                    if (mPhoneCount == 1 || (mPhoneCount > 1 && (!mIccIdReady[PhoneConstants.SUB1]
                                    || !mIccIdReady[PhoneConstants.SUB2]))) {
                        updateEnhanced4gLteStatusPerSlot(phoneId);
                    }
                }
                //after hot plugin many times with same SIMs, sometimes DDS switched but not update
                //VoLTE switch status due to SIM iccid does not change. this is related to if both
                //SIMs can be ready at the same time after one round of hot-swap,
                //if just one SIM is inserted and recognised, DDS will be changed
                if (mPhoneCount > 1 && mIccIdReady[PhoneConstants.SUB1]
                        && mIccIdReady[PhoneConstants.SUB2]) {
                    updateEnhanced4GLteStatusForMultiSim();
                }
            } else {
                mIccIdReady[phoneId] = false;
            }
        }
    }

    private void updateEnhanced4gLteStatusPerSlot(int slot) {
        if (mIccIdChanged[slot] && isCtCard(slot) && hasUserModifiedSettings(slot)) {
            updateEnhanced4gLteSetting(slot, false);
        } else if (mIccIdChanged[slot]) {
            mIccIdChanged[slot] = false;
        }
    }

    private boolean hasUserModifiedSettings(int slot) {
        ImsManager mgr = ImsManager.getInstance(this, slot);
        return mgr != null && mgr.isEnhanced4gLteModeSettingEnabledByUser()
            && mgr.isNonTtyOrTtyOnVolteEnabled();
    }

    private void updateEnhanced4gLteSetting(int slot, boolean enable) {
        ImsManager mgr = ImsManager.getInstance(this, slot);
        mgr.setEnhanced4gLteModeSetting(enable);
        mIccIdChanged[slot] = false;
    }

    private void updateEnhanced4GLteStatusForMultiSim() {
        log("updateEnhanced4GLteStatusForMultiSim ");
        //CT + CT
        if(isCtCard(PhoneConstants.SUB1) && isCtCard(PhoneConstants.SUB2)) {
            log("Two CT SIM cards");
            int defaultDataSlot = SubscriptionManager
                .getSlotIndex(SubscriptionManager.getDefaultDataSubscriptionId());
            int preDdsSlot = getDdsSlot();
            log("preDdsSlot is : " + preDdsSlot);
            if(preDdsSlot == -1) {
                saveDdsToSp(this, defaultDataSlot);
            }
            if (defaultDataSlot != preDdsSlot && preDdsSlot != -1) {
                int userPerfDdsSub = getUserPrefDataSubIdFromDB(this);
                if (userPerfDdsSub != -1 &&
                        SubscriptionManager.getSlotIndex(userPerfDdsSub) != defaultDataSlot) {
                    log("system dds is different from user perf, system is " + defaultDataSlot
                            + " user perf DDS is " + SubscriptionManager
                            .getSlotIndex(userPerfDdsSub));
                }
                log("dds changed by system and reset VoLte switch");
                for (int i = 0; i < mPhoneCount;  i++) {
                    mIccIdChanged[i] = true;
                }
                saveDdsToSp(this, defaultDataSlot);
            }

            log("defaultDataSlot : " + defaultDataSlot);
            int sub = PhoneConstants.SUB1;
            //SUB1 is default data
            if (defaultDataSlot == sub) {
                if (mIccIdChanged[sub] && hasUserModifiedSettings(sub)) {
                    log("New CT card inserted in default data[1], turn off Enhanced 4G LTE.");
                     updateEnhanced4gLteSetting(sub, false);
                } else if (mIccIdChanged[sub]) {
                    mIccIdChanged[sub] = false;
                }

                sub = PhoneConstants.SUB2;
                if (!hasUserModifiedSettings(sub)) {
                    log("Daul SIM and both CT cards inserted, turn on volte for nDDS SIM2.");
                    updateEnhanced4gLteSetting(sub, true);
                } else if (mIccIdChanged[sub]) {
                    mIccIdChanged[sub] = false;
                }
                return;
            }

            //SUB2 is default data
            sub = PhoneConstants.SUB2;
            if (defaultDataSlot == sub) {
                if (mIccIdChanged[sub] && hasUserModifiedSettings(sub)) {
                    log("New CT card inserted in default data[2], turn off Enhanced 4G LTE.");
                     updateEnhanced4gLteSetting(sub, false);
                } else if (mIccIdChanged[sub]) {
                    mIccIdChanged[sub] = false;
                }

                sub = PhoneConstants.SUB1;
                if ( !hasUserModifiedSettings(sub)) {
                    log("Daul SIM and both CT cards inserted, turn on volte for nDDS SIM1.");
                    updateEnhanced4gLteSetting(sub, true);
                } else if (mIccIdChanged[sub]) {
                    mIccIdChanged[sub] = false;
                }
                return;
            }
            return;
        }
        //CT + Ohters or Others + CT
        for (int i = 0; i < mPhoneCount; i++) {
            if (mIccIdChanged[i]) {
                updateEnhanced4gLteStatusPerSlot(i);
            }
        }
    }

    private static boolean isCardsInfoChanged(Context context, int slotId, String iccId) {
        if (mSharePref == null ) {
            return false;
        }
        String iccIdInSP = mSharePref.getString("PreIccId_" + slotId , null);
        log(" slotId = " + slotId + ", icc id = " + iccId + ", icc id in sp=" + iccIdInSP);
        return !TextUtils.equals(iccId, iccIdInSP);
    }

    private static void saveCardIccIdInfoInSp(Context context, int slotId, String iccId) {
        log("saveCardIccIdInfoInSp : slotId" + slotId + ", iccid: " + iccId + " mSharePref : "
                + mSharePref);
        if (mSharePref == null ) {
            return;
        }
        mSharePref.edit().putString("PreIccId_" + slotId, iccId).commit();
        if (mUnlocked) {
            saveToCredentialProtectedStorage(context, slotId, iccId);
        }
    }

    private static void saveToCredentialProtectedStorage(Context cxt, int slotId, String iccId) {
        log("saveToCredentialProtectedStorage, slotId : " + slotId + " ,iccId : " + iccId);
        PreferenceManager.getDefaultSharedPreferences(cxt).edit()
            .putString("PreIccId_" + slotId, iccId).commit();
    }

    private static void saveDdsToSp(Context cxt, int slot) {
        log("DDS slot is : " + slot);
        if (mSharePref == null ) {
            return;
        }
        mSharePref.edit().putInt("dds_slot", slot).commit();
        if (mUnlocked) {
            saveDdsSlotToCs(cxt, slot);
        }
    }

    private static int getDdsSlot() {
        if (mSharePref == null ) {
            return -1;
        }
        return  mSharePref.getInt("dds_slot", -1);
    }

    private static void saveDdsSlotToCs(Context cxt, int slot) {
        log("DDS slot(in CS) is : " + slot);
        PreferenceManager.getDefaultSharedPreferences(cxt).edit()
            .putInt("dds_slot", slot).commit();
    }
    /**
     * used for platform build compatible
     */
    private static boolean isAtLeastN() {
        return Build.VERSION.SDK_INT >= 24;
    }

    private static SharedPreferences getDefaultSharedPreferences(Context context) {
        final Context storageContext;
        final String name = PreferenceManager.getDefaultSharedPreferencesName(context);
        if (isAtLeastN()) {
            storageContext = context.createDeviceProtectedStorageContext();
            if (!storageContext.moveSharedPreferencesFrom(context, name)) {
                log("Failed to migrate shared preferences");
            }
        } else {
            storageContext = context;
        }
        return storageContext != null ? storageContext
            .getSharedPreferences(name, Context.MODE_PRIVATE) : null;
    }

    private boolean isCtCard(int slotId) {
        log("isCtCard: " + slotId);
        if (mSharePref == null ) {
            log("mSharePref is null!");
            return false;
        }
        String iccIdInSP = mSharePref.getString("PreIccId_" + slotId , null);
        if (iccIdInSP == null ) {
            log("iccid is null");
            return false;
        }
        log("iccid in SP is : " + iccIdInSP);
        String[] ctIccIdList = getResources().getStringArray(R.array.ct_all_iccid_prefix_list);
        if (ctIccIdList != null) {
            for (String iccidPrefix : ctIccIdList) {
                if (iccIdInSP.startsWith(iccidPrefix)) {
                    log("Slot[" + slotId + "] Iccid is: " + iccIdInSP + " is CT card");
                    return true;
                }
            }
        }
        log("This is not CT SIM card!");
        return false;
    }

    /**
     * Required for service implementation.
     */
    @Override
    public void onStart(Intent intent, int startId) {
        deviceReadyToSyncData(this);
    }

    @Override
    public void onDestroy() {
        mSharePref = null;
        unregisterReceiver(mBroadcastReceiver);
    }
    /**
     * Handle the bind request.
     */
    @Override
    public IBinder onBind(Intent intent) {
        if (DBG) log("binding service implementation");
        return null;
    }

    private static void log(String msg) {
        Log.d(LOG_TAG, msg);
    }
}
