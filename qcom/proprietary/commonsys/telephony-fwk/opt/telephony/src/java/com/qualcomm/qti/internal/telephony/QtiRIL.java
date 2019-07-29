/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.internal.telephony;

import static com.android.internal.telephony.RILConstants.*;
import static com.android.internal.util.Preconditions.checkNotNull;

import android.content.Context;
import android.hardware.radio.V1_0.RadioError;
import android.hardware.radio.V1_0.RadioResponseInfo;
import android.hardware.radio.V1_0.SendSmsResult;
import android.hidl.manager.V1_0.IServiceManager;
import android.hidl.manager.V1_0.IServiceNotification;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.HwBinder;
import android.os.Message;
import android.os.Parcel;
import android.os.RemoteException;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.WorkSource;
import android.telephony.Rlog;

import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.ConcurrentHashMap;

import com.android.internal.telephony.RIL;
import com.android.internal.telephony.RILRequest;
import com.android.internal.telephony.CommandException;
import com.android.internal.telephony.nano.TelephonyProto.SmsSession;
import com.android.internal.telephony.SmsResponse;

import android.hardware.radio.V1_0.SendSmsResult;
import android.hardware.radio.V1_0.CdmaSmsMessage;

import vendor.qti.hardware.radio.qtiradio.V1_0.IQtiRadio;
import vendor.qti.hardware.radio.qtiradio.V1_0.QtiRadioResponseInfo;
import vendor.qti.hardware.radio.qtiradio.V1_0.QtiRadioError;
import vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadioResponse;
import vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadioIndication;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.util.concurrent.atomic.AtomicLong;

/**
 * QtiRIL extends RIL.
 *
 * {@hide}
 */
public final class QtiRIL extends RIL {
    static final String TAG = "QTIRILJ";
    static final String[] QTI_HIDL_SERVICE_NAME = {"slot1", "slot2", "slot3"};
    QtiRadioResponse mQtiRadioResponse;
    QtiRadioIndication mQtiRadioIndication;

    vendor.qti.hardware.radio.qtiradio.V2_2.IQtiRadioResponse.Stub mClientRadioResponseCb;
    vendor.qti.hardware.radio.qtiradio.V2_2.IQtiRadioIndication.Stub mClientRadioIndicationCb;

    private IQtiRadio mQtiRadio;
    int mQtiPhoneId = 0;
    final QtiRadioProxyDeathRecipient mDeathRecipient;
    final AtomicLong mQtiRadioProxyCookie = new AtomicLong(0);
    // Notification object used to listen to the start of the IQtiRadio
    private final QtiRadioServiceNotification mServiceNotification =
        new QtiRadioServiceNotification();

    final class QtiRadioProxyDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            // Deal with service going away
            Rlog.d(TAG, "serviceDied");
            resetServiceAndRequestList();
        }
    }

    private void resetServiceAndRequestList() {
        resetProxyAndRequestList();
        mQtiRadio = null;
        mQtiRadioResponse = null;
        mQtiRadioIndication = null;
        mQtiRadioProxyCookie.incrementAndGet();
    }

    /**
     * Returns true if we are connected to IQtiRadio service, false otherwise.
     */
    private boolean isQtiRadioServiceConnected() {
        return (mQtiRadio != null);
    }

    /**
     * Class that implements the service notification which gets called once the
     * service with fully qualified name fqName has started
     */
    final class QtiRadioServiceNotification extends IServiceNotification.Stub {
        /**
         * Callback that gets called when the service has registered
         * @param fqName - Fully qualified name of the service
         * @param name - Name of the service
         * @param preexisting - if the registration is preexisting
         */
        @Override
        public void onRegistration(String fqName, String name, boolean preexisting) {
            Rlog.d(TAG, "QtiRadio interface service started " + fqName + " " + name +
                " preexisting =" + preexisting);
            if (!isQtiRadioServiceConnected()) {
                initQtiRadio();
            }
        }
    }

    /**
     * Register for notification when the com.qualcomm.qti.qtiradio@1.0::IQtiRadio is registered
     */
    private void registerForQtiRadioServiceNotification() {
        try {
            final boolean ret = IServiceManager.getService()
                    .registerForNotifications("vendor.qti.hardware.radio.qtiradio@1.0::IQtiRadio",
                    QTI_HIDL_SERVICE_NAME[mQtiPhoneId], mServiceNotification);
            if (!ret) {
                Rlog.e(TAG, "Failed to register for service start notifications");
            }
        } catch (RemoteException ex) {
            Rlog.e(TAG, "Failed to register for service start notifications. Exception " + ex);
        }
    }

    /**
     * Initialize the instance of IQtiRadio. Get the service and register the callback object
     * to be called for the responses to the solicited and unsolicited requests.
     */
    private synchronized void initQtiRadio() {
        try {
            mQtiRadio = IQtiRadio.getService(QTI_HIDL_SERVICE_NAME[mQtiPhoneId]);
            if (mQtiRadio == null) {
                Rlog.e(TAG, "initQtiRadio: mQtiRadio is null. Return");
                return;
            }
            Rlog.d(TAG, "initQtiRadio: mQtiRadio" + mQtiRadio);
            mQtiRadio.linkToDeath(mDeathRecipient,
                    mQtiRadioProxyCookie.incrementAndGet());
            mQtiRadioResponse = new QtiRadioResponse(this);
            mQtiRadioIndication = new QtiRadioIndication(mQtiPhoneId);
            mQtiRadio.setCallback(mQtiRadioResponse, mQtiRadioIndication);
        } catch (Exception ex) {
            Rlog.e(TAG, "initQtiRadio: Exception: " + ex);
            resetServiceAndRequestList();
        }
    }
    //***** Constructors

    public QtiRIL(Context context, int preferredNetworkType, int cdmaSubscription) {
        this(context, preferredNetworkType, cdmaSubscription, null);
    }

    public QtiRIL(Context context, int preferredNetworkType, int cdmaSubscription,
            Integer instanceId) {
        super(context, preferredNetworkType, cdmaSubscription, instanceId);
        mQtiPhoneId = instanceId;
        Rlog.d(TAG, "QtiRIL");
        mDeathRecipient = new QtiRadioProxyDeathRecipient();
        registerForQtiRadioServiceNotification();
    }

    /** Returns a IQtiRadio instance or null if the service is not available. */
    public IQtiRadio getQtiRadioProxy(Message result) {
        if (!mIsMobileNetworkSupported) {
            Rlog.d(TAG, "getQtiRadioProxy: Not calling getService(): wifi-only");
            if (result != null) {
                AsyncResult.forMessage(result, null,
                        CommandException.fromRilErrno(RADIO_NOT_AVAILABLE));
                result.sendToTarget();
            }
            return null;
        }

        if (mQtiRadio == null) {

            Rlog.d(TAG, "getQtiRadioProxy: mRadioProxy == null");
            if (result != null) {
                AsyncResult.forMessage(result, null,
                        CommandException.fromRilErrno(RADIO_NOT_AVAILABLE));
                result.sendToTarget();
            }
        }

        return mQtiRadio;
    }

    public void enable5g(int serial) throws RemoteException {
        vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadio radioProxy2_0 = vendor.qti.hardware
                .radio.qtiradio.V2_0.IQtiRadio.castFrom(mQtiRadio);
        if (radioProxy2_0 != null) {
            radioProxy2_0.enable5g(serial);
        } else {
            throw new RemoteException("API not available!");
        }
    }

    public void disable5g(int serial) throws RemoteException {
        vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadio radioProxy2_0 = vendor.qti.hardware
                .radio.qtiradio.V2_0.IQtiRadio.castFrom(mQtiRadio);
        if (radioProxy2_0 != null) {
            radioProxy2_0.disable5g(serial);
        } else {
            throw new RemoteException("API not available!");
        }
    }

    public void enable5gOnly(int serial) throws RemoteException {
        vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadio radioProxy2_0 = vendor.qti.hardware
                .radio.qtiradio.V2_0.IQtiRadio.castFrom(mQtiRadio);
        if (radioProxy2_0 != null) {
            radioProxy2_0.enable5gOnly(serial);
        } else {
            throw new RemoteException("API not available!");
        }
    }

    public void query5gStatus(int serial) throws RemoteException {
        vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadio radioProxy2_0 = vendor.qti.hardware
                .radio.qtiradio.V2_0.IQtiRadio.castFrom(mQtiRadio);
        if (radioProxy2_0 != null) {
            radioProxy2_0.query5gStatus(serial);
        } else {
            throw new RemoteException("API not available!");
        }
    }

    public void queryNrDcParam(int serial) throws RemoteException {
        vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadio radioProxy2_0 = vendor.qti.hardware
                .radio.qtiradio.V2_0.IQtiRadio.castFrom(mQtiRadio);
        if (radioProxy2_0 != null) {
            radioProxy2_0.queryNrDcParam(serial);
        } else {
            throw new RemoteException("API not available!");
        }
    }

    public void queryNrBearerAllocation(int serial) throws RemoteException {
        vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadio radioProxy2_0 = vendor.qti.hardware
                .radio.qtiradio.V2_0.IQtiRadio.castFrom(mQtiRadio);
        if (radioProxy2_0 != null) {
            radioProxy2_0.queryNrBearerAllocation(serial);
        } else {
            throw new RemoteException("API not available!");
        }
    }

    public void queryNrSignalStrength(int serial) throws RemoteException {
        vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadio radioProxy2_0 = vendor.qti.hardware
                .radio.qtiradio.V2_0.IQtiRadio.castFrom(mQtiRadio);
        if (radioProxy2_0 != null) {
            radioProxy2_0.queryNrSignalStrength(serial);
        } else {
            throw new RemoteException("API not available!");
        }
    }

    public void queryUpperLayerIndInfo(int serial) throws RemoteException {
        vendor.qti.hardware.radio.qtiradio.V2_1.IQtiRadio radioProxy2_1 = vendor.qti.hardware
                .radio.qtiradio.V2_1.IQtiRadio.castFrom(mQtiRadio);
        if (radioProxy2_1 != null) {
            radioProxy2_1.queryUpperLayerIndInfo(serial);
        } else {
            throw new RemoteException("API not available!");
        }
    }

    public void query5gConfigInfo(int serial) throws RemoteException {
        vendor.qti.hardware.radio.qtiradio.V2_1.IQtiRadio radioProxy2_1 = vendor.qti.hardware
                .radio.qtiradio.V2_1.IQtiRadio.castFrom(mQtiRadio);
        if (radioProxy2_1 != null) {
            radioProxy2_1.query5gConfigInfo(serial);
        } else {
            throw new RemoteException("API not available!");
        }
    }

    public void queryNrIconType(int serial) throws RemoteException {
        vendor.qti.hardware.radio.qtiradio.V2_2.IQtiRadio radioProxy2_2 = vendor.qti.hardware
                .radio.qtiradio.V2_2.IQtiRadio.castFrom(mQtiRadio);
        if (radioProxy2_2 != null) {
            radioProxy2_2.queryNrIconType(serial);
        } else {
            throw new RemoteException("API not available!");
        }
    }


    private String convertNullToEmptyString(String string) {
        return string != null ? string : "";
    }


    @Override
    public void sendCdmaSms(byte[] pdu, Message result) {
        sendCdmaSms(pdu, result, false);
    }

    @Override
    public void sendCdmaSms(byte[] pdu, Message result, boolean expectMore) {
        String feature = SystemProperties.get("persist.radio.feature");
        if (!(feature.equals("CDMA_SMS"))) {
            Rlog.d(TAG, "Feature not enabled, fall back to default sendCdmaSms");
            super.sendCdmaSms(pdu, result);
            return;
        }
        IQtiRadio radioProxy = getQtiRadioProxy(null);
        vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadio radioProxy2_0 =
                    vendor.qti.hardware.radio.qtiradio.V2_0.IQtiRadio.castFrom(radioProxy);
        if (radioProxy2_0 != null) {
            int serial = obtainRequestSerial(RIL_REQUEST_CDMA_SEND_SMS, result,
                    mRILDefaultWorkSource);

            // Do not log function arg for privacy
            Rlog.d(TAG, "["+ serial+ "] >  RIL_REQUEST_CDMA_SEND_SMS expectMore=" +expectMore);

             CdmaSmsMessage msg = new CdmaSmsMessage();
             constructCdmaSendSmsRilRequest(msg, pdu);

             try {
                 radioProxy2_0.sendCdmaSms(serial, msg, expectMore);
                 mMetrics.writeRilSendSms(mQtiPhoneId, serial, SmsSession.Event.Tech.SMS_CDMA,
                         SmsSession.Event.Format.SMS_FORMAT_3GPP2);
             } catch (RemoteException | RuntimeException e) {
                  resetServiceAndRequestList();
             }
        } else {
            Rlog.d(TAG, "fall back to default sendCdmaSms");
            super.sendCdmaSms(pdu, result);
        }
    }


    /* package */
    Message qtiGetMessageFromRequest (Object request) {
        return getMessageFromRequest (request);
    }

    /* package */
    Object qtiProcessResponse(RadioResponseInfo responseInfo) {
        return processResponse(responseInfo);
    }

    void qtiProcessResponseDone(Object rr, RadioResponseInfo responseInfo, Object ret) {
        processResponseDone(rr, responseInfo, ret);
    }

    public void setCallbacks(vendor.qti.hardware.radio.qtiradio.V2_2.IQtiRadioResponse.Stub
            qtiRadioResponse,
            vendor.qti.hardware.radio.qtiradio.V2_2.IQtiRadioIndication.Stub
            qtiRadioIndication) {
        mClientRadioResponseCb = qtiRadioResponse;
        mClientRadioIndicationCb = qtiRadioIndication;
    }

    public class QtiRadioResponse extends
            vendor.qti.hardware.radio.qtiradio.V2_2.IQtiRadioResponse.Stub {

        QtiRIL mRil;
        static final String QTI_RILJ_LOG_TAG = "QtiRadioResponse";

        public QtiRadioResponse(QtiRIL ril) {
            mRil = ril;
        }

        /**
         * Helper function to send response msg
         * @param msg Response message to be sent
         * @param ret Return object to be included in the response message
         */
        void sendMessageResponse(Message msg, Object ret) {
            if (msg != null) {
                AsyncResult.forMessage(msg, ret, null);
                msg.sendToTarget();
            }
        }

        RadioResponseInfo toRadioResponseInfo (
                QtiRadioResponseInfo qtiResponseInfo) {
            RadioResponseInfo responseInfo = new RadioResponseInfo();
            responseInfo.type = qtiResponseInfo.type;
            responseInfo.serial = qtiResponseInfo.serial;
            responseInfo.error = qtiResponseInfo.error;
            return responseInfo;
        }

        private void responseString(RadioResponseInfo responseInfo, String str) {
            Object request = mRil.qtiProcessResponse(responseInfo);
            Message result = mRil.qtiGetMessageFromRequest(request);
            if (result != null) {
                if (responseInfo.error == QtiRadioError.NONE) {
                    sendMessageResponse(result, str);
                }
                mRil.qtiProcessResponseDone(request, responseInfo, str);
            }
        }

        /**
         * @param responseInfo Response info struct containing response type, serial no. and error
         * @param atr String containing the ATR
         */
        @Override
        public void getAtrResponse(QtiRadioResponseInfo qtiResponseInfo, String atr) {
            Rlog.d(QTI_RILJ_LOG_TAG, "getAtrResponse");
            RadioResponseInfo responseInfo= toRadioResponseInfo(
                    qtiResponseInfo);
            responseString(responseInfo, atr);
        }
        @Override
        public void onEnable5gResponse(int serial, int errorCode, int status) {
            Rlog.d(TAG,"onEnable5gResponse: serial = " + serial + " errorCode = " + errorCode +
                    " " + "status = " + status);
            if (mClientRadioResponseCb != null) {
                try {
                    mClientRadioResponseCb.onEnable5gResponse(serial, errorCode, status);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onDisable5gResponse(int serial, int errorCode, int status) {
            Rlog.d(TAG, "onDisable5gResponse: serial = " + serial + " errorCode = " + errorCode +
                    " " + "status = " + status);
            if (mClientRadioResponseCb != null) {
                try {
                    mClientRadioResponseCb.onDisable5gResponse(serial, errorCode, status);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onEnable5gOnlyResponse(int serial, int errorCode, int status) {
            Rlog.d(TAG, "onEnable5gOnlyResponse: serial = " + serial + " errorCode = " + errorCode +
                    "" + " " + "status = " + status);
            if (mClientRadioResponseCb != null) {
                try {
                    mClientRadioResponseCb.onEnable5gOnlyResponse(serial, errorCode, status);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void on5gStatusResponse(int serial, int errorCode, int enabled) {
            Rlog.d(TAG, "on5gStatusResponse: serial = " + serial + " errorCode = " + errorCode +
                    " " + "enabled = " + enabled);
            if (mClientRadioResponseCb != null) {
                try {
                    mClientRadioResponseCb.on5gStatusResponse(serial, errorCode, enabled);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onNrDcParamResponse(int serial, int errorCode,
                vendor.qti.hardware.radio.qtiradio.V2_0.DcParam dcParam) {
            Rlog.d(TAG, "onNrDcParamResponse: serial = " + serial + " errorCode = "
                    + errorCode + " dcParam = " + dcParam);
            if (mClientRadioResponseCb != null) {
                try {
                    mClientRadioResponseCb.onNrDcParamResponse(serial, errorCode, dcParam);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onNrBearerAllocationResponse_2_1(int serial, int errorCode, int bearerStatus) {
            Rlog.d(TAG, "onNrBearerAllocationResponse_2_1: serial = " + serial + " errorCode = " +
                    errorCode + " bearerStatus = " + bearerStatus);
            if (mClientRadioResponseCb != null) {
                try {
                    mClientRadioResponseCb.onNrBearerAllocationResponse_2_1(serial, errorCode,
                            bearerStatus);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onNrBearerAllocationResponse(int serial, int errorCode, int bearerStatus) {
            Rlog.d(TAG, "onNrBearerAllocationResponse: serial = " + serial + " errorCode = " +
                    errorCode + " bearerStatus = " + bearerStatus);
            if (mClientRadioResponseCb != null) {
                try {
                    mClientRadioResponseCb.onNrBearerAllocationResponse(serial, errorCode,
                            bearerStatus);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onUpperLayerIndInfoResponse(int serial, int errorCode,
                vendor.qti.hardware.radio.qtiradio.V2_1.UpperLayerIndInfo uliInfo){
            Rlog.d(TAG, "UpperLayerIndInfoResponse: serial = " + serial + " errorCode = "
                    + errorCode + " UpperLayerIndInfo = " + uliInfo);
            if (mClientRadioResponseCb != null) {
                try {
                    mClientRadioResponseCb.onUpperLayerIndInfoResponse(serial, errorCode,
                            uliInfo);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void on5gConfigInfoResponse(int serial, int errorCode, int confType){
            Rlog.d(TAG, "on5gConfigInfoResponse: serial = " + serial + " errorCode = "
                    + errorCode + " ConfigType = " + confType);
            if (mClientRadioResponseCb != null) {
                try {
                    mClientRadioResponseCb.on5gConfigInfoResponse(serial, errorCode,
                            confType);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onSignalStrengthResponse(int serial, int errorCode,
                vendor.qti.hardware.radio.qtiradio.V2_0.SignalStrength signalStrength) {
            Rlog.d(TAG, "onSignalStrengthResponse: serial = " + serial + " errorCode = " +
                    errorCode + " signalStrength = " + signalStrength);
            if (mClientRadioResponseCb != null) {
                try {
                    mClientRadioResponseCb.onSignalStrengthResponse(serial, errorCode,
                            signalStrength);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onNrIconTypeResponse(int serial, int errorCode, int iconType){
            Rlog.d(TAG, "onNrIconTypeResponse: serial = " + serial + " errorCode = "
                    + errorCode + " iconType = " + iconType);
            if (mClientRadioResponseCb != null) {
                try {
                    mClientRadioResponseCb.onNrIconTypeResponse(serial, errorCode,
                            iconType);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        private void responseSms(RadioResponseInfo responseInfo, SendSmsResult sms) {
            Object request = mRil.qtiProcessResponse(responseInfo);
            if (request != null) {
                Message result = mRil.qtiGetMessageFromRequest(request);
                SmsResponse ret = new SmsResponse(sms.messageRef, sms.ackPDU, sms.errorCode);
                if (responseInfo.error == RadioError.NONE) {
                    sendMessageResponse(result, ret);
                }
                mRil.qtiProcessResponseDone(request, responseInfo, ret);
            }
        }

        /**
         * @param responseInfo Response info struct containing response type, serial no. and error
         * @param sms Sms result struct as defined by SendSmsResult in types.hal
         */
        @Override
        public void sendCdmaSmsResponse(QtiRadioResponseInfo qtiResponseInfo, SendSmsResult sms) {
            Rlog.d(TAG, "sendCdmaSmsResponse");
            RadioResponseInfo responseInfo= toRadioResponseInfo(
                    qtiResponseInfo);
            Rlog.d(QTI_RILJ_LOG_TAG, "["+ qtiResponseInfo.serial+ "] < RIL_REQUEST_CDMA_SEND_SMS ");
            responseSms(responseInfo, sms);
        }

    }

    public class QtiRadioIndication extends
            vendor.qti.hardware.radio.qtiradio.V2_2.IQtiRadioIndication.Stub {
        static final String TAG = "QtiRadioIndication";

        int mSlotId;

        public QtiRadioIndication(int slotId) {
            Rlog.d(TAG, "[" + slotId + "]Constructor: ");
            mSlotId = slotId;
        }

        @Override
        public void on5gStatusChange(int enableStatus) {
            Rlog.d(TAG, "on5gStatusChange: slotId = " + mSlotId);
            if (mClientRadioIndicationCb != null) {
                try {
                    mClientRadioIndicationCb.on5gStatusChange(enableStatus);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onNrDcParamChange(
                vendor.qti.hardware.radio.qtiradio.V2_0.DcParam dcParam) {
            Rlog.d(TAG, "onNrDcParamChange: slotId = " + mSlotId);
            if (mClientRadioIndicationCb != null) {
                try {
                    mClientRadioIndicationCb.onNrDcParamChange(dcParam);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onNrBearerAllocationChange_2_1(int bearerStatus) {
            Rlog.d(TAG, "onNrBearerAllocationChange_2_1: slotId = " + mSlotId);
            if (mClientRadioIndicationCb != null) {
                try {
                    mClientRadioIndicationCb.onNrBearerAllocationChange_2_1(bearerStatus);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onNrBearerAllocationChange(int bearerStatus) {
            Rlog.d(TAG, "onNrBearerAllocationChange: slotId = " + mSlotId);
            if (mClientRadioIndicationCb != null) {
                try {
                    mClientRadioIndicationCb.onNrBearerAllocationChange(bearerStatus);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onUpperLayerIndInfoChange(
                vendor.qti.hardware.radio.qtiradio.V2_1.UpperLayerIndInfo uliInfo){
            Rlog.d(TAG, "onUpperLayerIndInfoChange: UpperLayerIndInfo = " + uliInfo);
            if (mClientRadioIndicationCb != null) {
                try {
                    mClientRadioIndicationCb.onUpperLayerIndInfoChange(uliInfo);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void on5gConfigInfoChange(int confType){
            Rlog.d(TAG, "on5gConfigInfoChange: ConfigType = " + confType);
            if (mClientRadioIndicationCb != null) {
                try {
                    mClientRadioIndicationCb.on5gConfigInfoChange(confType);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onNrIconTypeChange(int iconType){
            Rlog.d(TAG, "onNrIconTypeChange: iconType = " + iconType);
            if (mClientRadioIndicationCb != null) {
                try {
                    mClientRadioIndicationCb.onNrIconTypeChange(iconType);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void onSignalStrengthChange(
                vendor.qti.hardware.radio.qtiradio.V2_0.SignalStrength signalStrength) {
            Rlog.d(TAG, "onSignalStrengthChange: slotId = " + mSlotId);
            if (mClientRadioIndicationCb != null) {
                try {
                    mClientRadioIndicationCb.onSignalStrengthChange(signalStrength);
                } catch (RemoteException e) {
                    e.printStackTrace();
                }
            }
        }

        @Override
        public void qtiRadioIndication(int value) {
            Rlog.d(TAG, "qtiRadioIndication: NOP!!");
        }

    }
}
