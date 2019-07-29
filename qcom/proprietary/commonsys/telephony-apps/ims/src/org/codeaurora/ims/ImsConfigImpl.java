/*
 * Copyright (c) 2015, 2016, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.Manifest;

import android.content.Context;
import android.content.pm.PackageManager;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.telephony.ims.stub.ImsConfigImplBase;

import com.android.ims.ImsConfig;
import com.android.ims.ImsConfigListener;
import com.android.ims.internal.IImsConfig;
import com.android.internal.os.SomeArgs;
import com.qualcomm.ims.utils.Log;

public class ImsConfigImpl extends ImsConfigImplBase {
    private static final int EVENT_ACCESS_PROVISIONED_VAL = 1;
    private static final int EVENT_SET_FEATURE_VALUE = 2;

    private ImsServiceSub mServiceSub;
    private ImsSenderRxr mCi;
    private Handler mHandler;
    private Context mContext;

    // Permissions used by class.
    private final String MODIFY_PHONE_STATE = Manifest.permission.MODIFY_PHONE_STATE;
    private final String READ_PRIVILEGED_PHONE_STATE = Manifest.permission.READ_PRIVILEGED_PHONE_STATE;
    private final String READ_PHONE_STATE = Manifest.permission.READ_PHONE_STATE;

    /**
     * Creates the Ims Config interface object for a sub.
     * @param senderRxr
     */
    public ImsConfigImpl(ImsServiceSub serviceSub, ImsSenderRxr senderRxr, Context context) {
        super(context);
        mServiceSub = serviceSub;
        mCi = senderRxr;
        mContext = context;
        HandlerThread handlerThread = new HandlerThread("ImsConfigImplHandler");
        handlerThread.start();
        mHandler = new ImsConfigImplHandler(handlerThread.getLooper());
    }

    public enum SetCapabilityFailCause {
        ERROR_GENERIC,
        ERROR_SUCCESS;

        @Override
        public String toString() {
            switch(this) {
                case ERROR_GENERIC:
                    return "ERROR_GENERIC";
                case ERROR_SUCCESS:
                    return "ERROR_SUCCESS";
            }
            return "";
        }
    }

    public interface SetCapabilityValueListener {
        public void onSetCapabilityValueSuccess(int capability, int radioTech, int value);
        public void onSetCapabilityValueFailure(int capability, int radioTech,
                                                SetCapabilityFailCause reason);
    }

    /* Wrapper class to encapsulate the arguments and listener to the setFeatureValue and
     * getFeatureValue APIs
     */
    private static final class FeatureAccessWrapper {
        public int feature;
        public int network;
        public int value;
        public ImsConfigListener listener;
        public FeatureAccessWrapper(int feature, int network, int value,
                ImsConfigListener listener) {
            this.feature = feature;
            this.network = network;
            this.listener = listener;
            this.value = value;
        }
    }

    //Handler for tracking requests sent to ImsSenderRxr.
    private class ImsConfigImplHandler extends Handler {
        public ImsConfigImplHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.i(this, "Message received: what = " + msg.what);
            AsyncResult ar = (AsyncResult) msg.obj;

            switch (msg.what) {
                case EVENT_SET_FEATURE_VALUE:
                    onSetFeatureResponseDone(ar);
                    break;
                case EVENT_ACCESS_PROVISIONED_VAL:
                    onAccessProvisionedValDone(ar, msg.what);
                    break;
                default:
                    Log.e(this, "handleMessage: unhandled message");
            }
        }
    }

    private void onAccessProvisionedValDone(AsyncResult ar, int accessType) {
        SomeArgs response = (SomeArgs)ar.userObj;
        response.arg2 = getOperationStatus(ar.exception == null);
        response.arg3 = ar.result;
        synchronized(response) {
            Log.i(this, "Notifyall");
            response.notifyAll();
        }
    }

    private void onGetPacketCountDone(ImsConfigListener imsConfigListener, AsyncResult ar) {
        if (imsConfigListener != null) {
            try {
                int status = getOperationStatus(ar.exception == null);
                long result = ar.result == null ? 0 : (long)ar.result;
                //TODO - Compilation stub imsConfigListener.onGetPacketCount(status, result);
            } catch (Throwable t) {
                Log.e(this, "onGetPacketCountDone " + t);
            }
        } else {
            Log.v(this, "onGetPacketCountDone listener is null");
        }
    }

    private void onGetPacketErrorCountDone(ImsConfigListener imsConfigListener, AsyncResult ar) {
        if (imsConfigListener != null) {
            try {
                int status = getOperationStatus(ar.exception == null);
                long result = ar.result == null ? 0 : (long)ar.result;
                //TODO - Compilation stub imsConfigListener.onGetPacketErrorCount(status, result);
            } catch (Throwable t) {
                Log.e(this, "onGetPacketErrorCountDone " + t);
            }
        } else {
            Log.v(this, "onGetPacketErrorCountDone listener is null");
        }
    }

    private void onSetFeatureResponseDone(AsyncResult ar) {
        if (ar == null ) {
            Log.e(this, "getSetCapabilityValueListener :: AsyncResult is null.");
            return;
        }
        SetCapabilityValueListener listener = null;
        SomeArgs setCapArgs = null;
        // Note: Refer to setCapabilityValue to see SomeArgs argument details.
        if (ar.userObj instanceof SomeArgs) {
            setCapArgs = (SomeArgs)ar.userObj;
            if (setCapArgs.arg1 instanceof SetCapabilityValueListener) {
                listener = (SetCapabilityValueListener)setCapArgs.arg1;
            }
        }
        if (listener != null) {
            if (ar.exception == null) {
                listener.onSetCapabilityValueSuccess(setCapArgs.argi1, setCapArgs.argi2,
                        setCapArgs.argi3);
            } else {
                listener.onSetCapabilityValueFailure(setCapArgs.argi1, setCapArgs.argi2,
                        SetCapabilityFailCause.ERROR_GENERIC);
            }
        } else {
            Log.v(this, "onSetFeatureResponseDone :: listener is null");
        }
    }

    private int getOperationStatus(boolean status) {
        return status ? ImsConfig.OperationStatusConstants.SUCCESS :
                ImsConfig.OperationStatusConstants.FAILED;
    }

    private ImsConfigListener getImsConfigListener(AsyncResult ar) {
        if (ar == null ) {
            Log.e(this, "AsyncResult is null.");
        } else if (ar.userObj instanceof ImsConfigListener) {
            return (ImsConfigListener)ar.userObj;
        } else if (ar.userObj instanceof FeatureAccessWrapper &&
                ((FeatureAccessWrapper)(ar.userObj)).listener instanceof ImsConfigListener) {
            return (ImsConfigListener)((FeatureAccessWrapper)(ar.userObj)).listener;
        }

        Log.e(this, "getImsConfigListener returns null");
        return null;
    }

    private void enforceReadPhoneState(String fn) {
        if (mContext.checkCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE)
                != PackageManager.PERMISSION_GRANTED
                && mContext.checkCallingOrSelfPermission(READ_PHONE_STATE)
                != PackageManager.PERMISSION_GRANTED) {
            mContext.enforceCallingOrSelfPermission(READ_PHONE_STATE, fn);
        }
    }

    protected Object sendRequest(int requestType, int item, int event, boolean boolVal,
            int intVal, String strVal) {
        //SomeArgs semantics
        //arg1 - item
        //arg2 - result of operation - pass/fail
        //arg3 - value returned by function for get operation
        SomeArgs request = SomeArgs.obtain();
        try {
            request.arg1 = item;
            mCi.sendConfigRequest(requestType, ImsCallUtils.convertImsConfigToImsConfigItem(item),
                    boolVal, intVal, strVal, ImsConfigItem.NO_ERR,
                    mHandler.obtainMessage(event, request));

            synchronized(request) {
                while(request.arg2 == null) {
                    try {
                        request.wait();
                    } catch (InterruptedException e) {
                        //Do Nothing and wait
                    }
                }
            }
        } catch (IllegalArgumentException e) {
            request.arg2 = ImsConfig.OperationStatusConstants.UNKNOWN;
        } finally {
            return request;
        }
    }

    /**
     * Gets the currently stored value configuration value from the ImsService for {@code item}.
     *
     * @param item an integer key.
     * @return configuration value, stored in integer format or {@link #CONFIG_RESULT_UNKNOWN} if
     * unavailable.
     */
    @Override
    public int getConfigInt(int item) {
        Log.d(this, "getConfigInt :: item=" + item);
        enforceReadPhoneState("getProvisionedValue");
        if (!ImsCallUtils.isConfigRequestValid(item, ImsCallUtils.CONFIG_TYPE_INT)) {
            Log.e(this, "Invalid API request for item");
            return ImsConfigImplBase.CONFIG_RESULT_UNKNOWN;
        }
        SomeArgs result = (SomeArgs)sendRequest(MessageId.REQUEST_GET_IMS_CONFIG, item,
                EVENT_ACCESS_PROVISIONED_VAL, false, 0, null);
        int retVal = ((int)result.arg2 != ImsConfig.OperationStatusConstants.SUCCESS ?
                    ImsConfigImplBase.CONFIG_RESULT_UNKNOWN : (int)result.arg3);
        result.recycle();
        return retVal;
    }

    /**
     * Gets the currently stored value configuration value from the ImsService for {@code item}.
     *
     * @param item an integer key.
     * @return configuration value, stored in String format or {@code null} if unavailable.
     */
    @Override
    public String getConfigString(int item) {
        Log.d(this, "getConfigString :: item=" + item);
        enforceReadPhoneState("getProvisionedStringValue");
        if (!ImsCallUtils.isConfigRequestValid(item, ImsCallUtils.CONFIG_TYPE_STRING)) {
            Log.e(this, "Invalid API request for item");
            return null;
        }
        SomeArgs result = (SomeArgs)sendRequest(MessageId.REQUEST_GET_IMS_CONFIG, item,
                EVENT_ACCESS_PROVISIONED_VAL, false, 0, null);
        String retVal = ((int)result.arg2 != ImsConfig.OperationStatusConstants.SUCCESS ?
                    null : (String)result.arg3);
        result.recycle();
        return retVal;
    }

    /**
     * Sets the configuration value for this ImsService.
     *
     * @param item an integer key.
     * @param value an integer containing the configuration value.
     * @return the result of setting the configuration value, defined as either
     * {@link #CONFIG_RESULT_FAILED} or {@link #CONFIG_RESULT_SUCCESS}.
     */
    @Override
    public int setConfig(int item, int value) {
        Log.d(this, "setConfig :: item=" + item + " value=" + value);
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "setProvisionedValue");
        // Sending this request to modem is causing WFC failures
        // Refer Google bug - 80649747 for more details
        // Bug will be fixed in Android Q, until then have to retian this hack
        if (item == ImsConfig.ConfigConstants.VOICE_OVER_WIFI_ROAMING) {
            Log.w(this, "VoWiFi Roaming: value can't be changed");
            return ImsConfigImplBase.CONFIG_RESULT_FAILED;
        }
        if (!ImsCallUtils.isConfigRequestValid(item, ImsCallUtils.CONFIG_TYPE_INT)) {
            Log.e(this, "Invalid API request for item");
            return ImsConfigImplBase.CONFIG_RESULT_FAILED;
        }
        SomeArgs result = (SomeArgs)sendRequest(MessageId.REQUEST_SET_IMS_CONFIG, item,
                EVENT_ACCESS_PROVISIONED_VAL, false, value, null);
        int retVal = (int)result.arg2;
        result.recycle();
        return getImsConfigImplBaseOperationConstant(retVal);
    }

    /**
     * Sets the configuration value for this ImsService.
     *
     * @param item an integer key.
     * @param value a String containing the new configuration value.
     * @return Result of setting the configuration value, defined as either
     * {@link #CONFIG_RESULT_FAILED} or {@link #CONFIG_RESULT_SUCCESS}.
     */
    @Override
    public int setConfig(int item, String value) {
        Log.d(this, "setConfig :: item=" + item + " value=" + value);
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "setProvisionedStringValue");
        if (!ImsCallUtils.isConfigRequestValid(item, ImsCallUtils.CONFIG_TYPE_STRING)) {
            Log.e(this, "Invalid API request for item");
            return ImsConfigImplBase.CONFIG_RESULT_FAILED;
        }
        SomeArgs result = (SomeArgs)sendRequest(MessageId.REQUEST_SET_IMS_CONFIG, item,
                EVENT_ACCESS_PROVISIONED_VAL, false, 0, value);
        int retVal = (int)result.arg2;
        result.recycle();
        return getImsConfigImplBaseOperationConstant(retVal);
    }

    public void setCapabilityValue(int feature, int network, int value,
            SetCapabilityValueListener listener) {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "setFeatureValue");
        Log.d(this, "setCapabilityValue :: feature=" + feature
                + " network=" + network + " value=" + value);
        int srvType = CallDetails.CALL_TYPE_VOICE;
        if (feature == ImsConfig.FeatureConstants.FEATURE_TYPE_VIDEO_OVER_LTE) {
            srvType = CallDetails.CALL_TYPE_VT;
        }
        int enabled = ServiceStatus.STATUS_DISABLED;
        if (value == ImsConfig.FeatureValueConstants.ON) {
            enabled = ServiceStatus.STATUS_ENABLED;
        }
        int act = RadioTech.RADIO_TECH_LTE;
        if (network == TelephonyManager.NETWORK_TYPE_IWLAN) {
            act = RadioTech.RADIO_TECH_IWLAN;
        }
        if (network == TelephonyManager.NETWORK_TYPE_LTE ||
                network == TelephonyManager.NETWORK_TYPE_IWLAN) {
            Log.i(this, "SetServiceStatus = " + srvType + " " + network + " " + enabled);
            SomeArgs setCapArgs = SomeArgs.obtain();
            setCapArgs.argi1 = srvType;
            setCapArgs.argi2 = network;
            setCapArgs.argi3 = value;
            setCapArgs.arg1 = listener;
            mCi.setServiceStatus(mHandler.obtainMessage(EVENT_SET_FEATURE_VALUE, setCapArgs),
                srvType, act, enabled, 0);
        }
    }

    /**
     * Utility method to convert config operation contant values.
     *
     * @param imsConfigOpConst An ImsConfig.OperationStatusConstants value.
     * @return The corresponding 'ImsConfigImplBase.CONFIG_RESULT_' value.
     */
    private static int getImsConfigImplBaseOperationConstant(int imsConfigOpConst) {
        switch (imsConfigOpConst) {
            case ImsConfig.OperationStatusConstants.SUCCESS:
                return ImsConfigImplBase.CONFIG_RESULT_SUCCESS;
            case ImsConfigImplBase.CONFIG_RESULT_FAILED:
                return ImsConfigImplBase.CONFIG_RESULT_FAILED;
            case ImsConfig.OperationStatusConstants.UNKNOWN:
            default:
                return ImsConfigImplBase.CONFIG_RESULT_UNKNOWN;
        }
    }
}
