/* Copyright (c) 2016,2017 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
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
package org.codeaurora.ims;

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.android.ims.ImsConfig;
import com.android.ims.ImsException;
import com.android.ims.ImsManager;

import org.codeaurora.ims.internal.IQtiImsExt;
import org.codeaurora.ims.internal.IQtiImsExtListener;
import org.codeaurora.ims.internal.IImsMultiIdentityInterface;
import org.codeaurora.ims.QtiCallConstants;
import org.codeaurora.ims.utils.QtiImsExtUtils;

/**
 * Provides API's for IQtiImsExt Binder such as sending call transfer etc
 * This class is starting point for all the QtiImsExt actions.
 * You can acquire an instance of it by calling {@link getInstance getInstance()}
 *
 * Note: The implementation of QtiImsExtManager is not synchronized hence this is
 * not a thread safe class, Assuming all the users will call the API's from the same thread
 */
public class QtiImsExtManager {

    private static String LOG_TAG = "QtiImsExtManager";

    /**
     * Key to retrieve service form the ServiceManager
     */
    public static final String SERVICE_ID = "qti.ims.ext";

    /**
     * All the QtiImsExt actions are performed using this interface,
     * this interface/binder provides API's such as sending
     * call transfer etc
     */
    private IQtiImsExt mQtiImsExt;
    private Context mContext;

    public QtiImsExtManager(Context context) {
        mContext = context;
    }

    public void setCallForwardUncondTimer(int phoneId, int startHour, int startMinute, int endHour,
            int endMinute, int action, int condition, int serviceClass, String number,
            IQtiImsExtListener listener) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.setCallForwardUncondTimer(phoneId, startHour, startMinute, endHour,
                endMinute, action, condition, serviceClass, number, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService setCallForwardUncondTimer : " + e);
        }
    }

    public void getCallForwardUncondTimer(int phoneId, int reason, int serviceClass,
            IQtiImsExtListener listener) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.getCallForwardUncondTimer(phoneId, reason, serviceClass, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService getCallForwardUncondTimer : " + e);
        }
    }

    public void getPacketCount(int phoneId, IQtiImsExtListener listener) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.getPacketCount(phoneId, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService getPacketCount : " + e);
        }
    }

    public void getPacketErrorCount(int phoneId, IQtiImsExtListener listener)
            throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.getPacketErrorCount(phoneId, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService getPacketErrorCount : " + e);
        }
    }

    public void resumePendingCall(int phoneId, int videoState) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.resumePendingCall(phoneId, videoState);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService resumePendingCall : " + e);
        }
    }

    public void sendCallTransferRequest(int phoneId, int type, String number,
            IQtiImsExtListener listener) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.sendCallTransferRequest(phoneId, type, number, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService sendCallTransferRequest : " + e);
        }
    }

    public void sendCancelModifyCall(int phoneId, IQtiImsExtListener listener)
            throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.sendCancelModifyCall(phoneId, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService sendCancelModifyCall : " + e);
        }
    }

    public void queryVopsStatus(int phoneId, IQtiImsExtListener listener) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.queryVopsStatus(phoneId, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService queryVopsStatus : " + e);
        }
    }

    public void querySsacStatus(int phoneId, IQtiImsExtListener listener) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.querySsacStatus(phoneId, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService querySsacStatus : " + e);
        }
    }

    public void registerForParticipantStatusInfo(int phoneId, IQtiImsExtListener listener)
            throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.registerForParticipantStatusInfo(phoneId, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService registerForParticipantStatusInfo : " + e);
        }
    }

    public void updateVoltePreference(int phoneId, int preference,
            IQtiImsExtListener listener) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.updateVoltePreference(phoneId, preference, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService updateVoltePreference : " + e);
        }
    }

    public void queryVoltePreference(int phoneId,
            IQtiImsExtListener listener) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.queryVoltePreference(phoneId, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService queryVoltePreference : " + e);
        }
    }

    public void getHandoverConfig(int phoneId, IQtiImsExtListener listener) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.getHandoverConfig(phoneId, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService getHandoverConfig : " + e);
        }
    }

    public void setHandoverConfig(int phoneId, int hoConfig, IQtiImsExtListener listener)
           throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            mQtiImsExt.setHandoverConfig(phoneId, hoConfig, listener);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService setHandoverConfig : " + e);
        }
    }

    /**
     * Check if binder is available, else try to retrieve it from ServiceManager
     * if binder still doesn't exists throw {@link QtiImsException}
     */
    private IQtiImsExt obtainBinder() throws QtiImsException {
        if (mQtiImsExt == null) {
            IBinder b = ServiceManager.getService(SERVICE_ID);
            mQtiImsExt = IQtiImsExt.Stub.asInterface(b);

            if (mQtiImsExt == null) {
                throw new QtiImsException("ImsService is not running");
            }

            try {
                b.linkToDeath(()->this.handleQtiImsExtServiceDeath(), 0);
            } catch (RemoteException e) {
                Log.e(LOG_TAG, "Unable to listen for QtiImsExt service death");
            }

            return mQtiImsExt;
        }
        return mQtiImsExt;
    }

    private void handleQtiImsExtServiceDeath() {
        mQtiImsExt = null;
        Log.i(LOG_TAG, "qtiImsExtDeathListener QtiImsExt binder died");
    }

    private void checkPhoneId(int phoneId) throws QtiImsException {
        if (!SubscriptionManager.isValidPhoneId(phoneId)) {
            Log.e(LOG_TAG, "phoneId " + phoneId + " is not valid");
            throw new QtiImsException("invalid phoneId");
        }
    }

    private void checkFeatureStatus(int phoneId) throws QtiImsException {
        if (mContext == null) throw new QtiImsException("Context is null");

        try {
            if (ImsManager.getInstance(mContext, phoneId).getImsServiceState() !=
                    ImsFeature.STATE_READY) {
                Log.e(LOG_TAG, "Feature status for phoneId " + phoneId + " is not ready");
                throw new QtiImsException("Feature state is NOT_READY");
            }
        } catch (ImsException e) {
            Log.e(LOG_TAG, "Got ImsException for phoneId " + phoneId);
            throw new QtiImsException("Feature state is NOT_READY");
        }
    }

    public int setRcsAppConfig(int phoneId, int defaultSmsApp) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        int ret = ImsConfig.OperationStatusConstants.UNKNOWN;
        try {
            ret = mQtiImsExt.setRcsAppConfig(phoneId, defaultSmsApp);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService setRcsAppConfig : " + e);
        }
        return ret;
    }

    public int getRcsAppConfig(int phoneId) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        int ret = QtiImsExtUtils.QTI_IMS_SMS_APP_INVALID;

        try {
            ret = mQtiImsExt.getRcsAppConfig(phoneId);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService getRcsAppConfig : " + e);
        }

        return ret;
    }

    public boolean isImsRegistered(int phoneId) throws QtiImsException {
        final int[] subIds = SubscriptionManager.getSubId(phoneId);
        int subId = SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        if (subIds != null && subIds.length >= 1) {
            subId = subIds[0];
        }
        TelephonyManager telephonyManager =
                (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        return telephonyManager.isImsRegistered(subId);
    }

    public int setVvmAppConfig(int phoneId, int defaultVvmApp) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        int ret = ImsConfig.OperationStatusConstants.UNKNOWN;
        try {
            ret = mQtiImsExt.setVvmAppConfig(phoneId, defaultVvmApp);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService setVvmAppConfig : " + e);
        }
        return ret;
    }

    public int getVvmAppConfig(int phoneId) throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        int ret = QtiImsExtUtils.QTI_IMS_VVM_APP_INVALID;

        try {
            ret = mQtiImsExt.getVvmAppConfig(phoneId);
        } catch(RemoteException e) {
            throw new QtiImsException("Remote ImsService getVvmAppConfig : " + e);
        }

        return ret;
    }

    public static ImsMultiIdentityManager createImsMultiIdentityManager(
            int phoneId, Context context) {
        QtiImsExtManager imsExtMgr = new QtiImsExtManager(context);
        return new ImsMultiIdentityManager(phoneId, imsExtMgr);
    }

    /*package private*/
    IImsMultiIdentityInterface getMultiIdentityInterface(int phoneId)
            throws QtiImsException {
        obtainBinder();
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
        try {
            return mQtiImsExt.getMultiIdentityInterface(phoneId);
        } catch(RemoteException e) {
            throw new QtiImsException("Failed to retrieve MultiIdentityInterface : " + e);
        }
    }

    /*package private*/
    void validateInvariants(int phoneId)  throws QtiImsException {
        checkPhoneId(phoneId);
        checkFeatureStatus(phoneId);
    }
}
