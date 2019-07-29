/*
* Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.smartassistant.service;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.soundtrigger.SoundTrigger;
import android.media.soundtrigger.SoundTriggerManager;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Log;
import android.widget.Toast;

import com.qualcomm.qti.smartassistant.R;
import com.qualcomm.qti.smartassistant.SmartAssistantActivity;
import com.qualcomm.qti.smartassistant.SmartAssistantApplication;
import com.qualcomm.qti.smartassistant.asr.CandidateInfo;
import com.qualcomm.qti.smartassistant.asr.IVoiceSdkListener;
import com.qualcomm.qti.smartassistant.asr.VoiceSdkHelper;
import com.qualcomm.qti.smartassistant.asr.baidu.AbstractDuerAudioRecorder;
import com.qualcomm.qti.smartassistant.asr.baidu.DuerAudioRecordImpl;
import com.qualcomm.qti.smartassistant.recordings.RecordingMgr;
import com.qualcomm.qti.smartassistant.detection.ISecondStageDetection;
import com.qualcomm.qti.smartassistant.detection.baidu.SnowboyDetection;
import com.qualcomm.qti.smartassistant.service.device.ContactsUploader;
import com.qualcomm.qti.smartassistant.sva.session.IRecognitionEvent;
import com.qualcomm.qti.smartassistant.sva.session.IWakeupSession;
import com.qualcomm.qti.smartassistant.sva.controller.ExtendedSmMgr;
import com.qualcomm.qti.smartassistant.sva.data.IExtendedSmModel;
import com.qualcomm.qti.smartassistant.sva.session.RecognitionEvent;
import com.qualcomm.qti.smartassistant.sva.session.WakeupSessionImpl;
import com.qualcomm.qti.smartassistant.utils.LogUtils;
import com.qualcomm.qti.smartassistant.utils.SharedPreferenceUtils;
import com.qualcomm.qti.smartassistant.utils.FileUtils;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import com.baidu.duer.dcs.util.NetWorkUtil;

public class SmartAssistantService extends Service{
    public static final String STARTED_BY_ASSISTANT_SERVICE = "Started_By_Assistant_Service";
    private static final int MSG_ESTABLISH_SVA_SESSION = 0x3001;
    private static final int MSG_TERMINATE_SVA_SESSION = 0x3002;
    private static final int MSG_RESTART_SVA_RECOGNITION = 0x3005;
    private static final String TAG = "SmartAssistantService";
    private static final String XIAODUXIAODU_PDK_NAME = "XiaoduXiaodu.uim";
    private static final String XIAODUXIAODU_UDM_NAME = "XiaoduXiaodu.udm";

    private ArrayList<Messenger> mClients = new ArrayList<>();
    private ArrayList<Context> mClientContexts = new ArrayList<>();

    private final Handler mReceiveHandler = new Handler(){
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what){
                case Messages.REGISTER_CLIENT_REQUEST:
                    LogUtils.v(TAG, "MSG_REGISTER_CLIENT" );
                    if (!mClients.contains(msg.replyTo)) {
                        mClients.add(msg.replyTo);
                        Log.v(TAG, "adding client = " + msg.replyTo +" clients size = "
                                + mClients.size());
                    }

                    Context registerContext = (Context) msg.obj;
                    if (!mClientContexts.contains(registerContext)
                            && registerContext instanceof SmartAssistantActivity){
                        mClientContexts.add((Context) msg.obj);
                        Log.v(TAG, "add client context= " + msg.obj);
                    }
                    break;
                case Messages.UNREGISTER_CLIENT_REQUEST:
                    LogUtils.v(TAG, "MSG_UNREGISTER_CLIENT" );
                    if (mClients.remove(msg.replyTo)){
                        Log.v(TAG, "remove client = " + msg.replyTo);
                    }

                    Context unRegisterContext = (Context) msg.obj;
                    if (mClientContexts.contains(unRegisterContext)
                            && unRegisterContext instanceof SmartAssistantActivity){
                        Log.v(TAG, "remove client context= " + msg.obj);
                        handleReleaseVoiceSdk();
                        mClientContexts.remove(unRegisterContext);
                    }
                    break;
                case Messages.START_RECOGNITION_REQUEST:
                    Log.v(TAG, "MSG_START_RECOGNITION" );
                    mAsyncEventsHandler.sendEmptyMessage(MSG_ESTABLISH_SVA_SESSION);
                    handleReleaseVoiceSdk();
                    handleInitializeVoiceSdk();
                    break;
                case Messages.STOP_RECOGNITION_REQUEST:
                    Log.v(TAG, "MSG_STOP_RECOGNITION" );
                    mAsyncEventsHandler.sendEmptyMessage(MSG_TERMINATE_SVA_SESSION);
                    handleReleaseVoiceSdk();
                    break;
            }
        }
    };

    // A reference for remote Service message receiving
    private final Messenger mReceiveMessenger = new Messenger(mReceiveHandler);

    private Handler mAsyncEventsHandler;
    private HandlerThread myHandlerThread;
    private ContactsUploader mContactsUploader;
    private VoiceSdkHelper mVoiceSdk;
    private ScreenWakeupHelper mScreenWakeup;
    private boolean mVoiceSdkInitialized = false;
    private String mCurrentSoundModelName = XIAODUXIAODU_PDK_NAME;
    private Runnable mVoicePostExecuteTask;
    private ExtendedSmMgr mExtendedSmMgr;
    private IWakeupSession mWakeupSession;
    private RecordingMgr mRecordingMgr;
    private ISecondStageDetection mSecondDetection;
    private AbstractDuerAudioRecorder mAsrAudioRecorder;

    private IVoiceSdkListener mVoiceSdkListener = new IVoiceSdkListener() {

        @Override
        public void onRenderVoiceInputText(String text) {
            sendMessageToClients(Messages.RENDER_VOICE_INPUT_TEXT_RESPONSE, text);
        }

        @Override
        public void onHtmlPayload(String url) {
            sendMessageToClients(Messages.LOAD_HTML_URL_RESPONSE, url);
        }

        @Override
        public void onPhoneCallByName(List<CandidateInfo> candidateInfos) {
            sendMessageToClients(Messages.CALL_BY_CONTACTS_RESPONSE, candidateInfos);
        }

        @Override
        public void onSendSmsByName(List<CandidateInfo> candidateInfos, String message) {
            Map<String, Object> smsParams =  new HashMap<>();
            smsParams.put("candidate",candidateInfos);
            smsParams.put("message",message);
            sendMessageToClients(Messages.SEND_SMS_BY_CONTACTS_RESPONSE, smsParams);
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();
        LogUtils.d(TAG," onCreate ");

        initializeAsyncEventsHandler();

        FileUtils.createDirIfNotExists(FileUtils.APP_PATH);
        FileUtils.createDirIfNotExists(FileUtils.LOOK_AHEAD_VOICE_COMMANDS_PATH);
        FileUtils.createDirIfNotExists(FileUtils.NORMAL_VOICE_COMMANDS_PATH);

        SmartAssistantApplication.getInstance().getAssetsFileMgr()
                .copyAssetsIfNotExists(getApplicationContext(), FileUtils.APP_PATH);

        mExtendedSmMgr = SmartAssistantApplication.getInstance().getExtendedSmMgr();
        mExtendedSmMgr.initSoundModels(getApplicationContext());

        mWakeupSession = new WakeupSessionImpl(getApplicationContext());

        mRecordingMgr = new RecordingMgr();
        mSecondDetection  = new SnowboyDetection(getApplicationContext(),
                mRecordingMgr);
        initSecondDetectionListener();
        mAsrAudioRecorder = new DuerAudioRecordImpl(mRecordingMgr);
        initAsrRecordingListener();
        mVoiceSdk =  new VoiceSdkHelper(getApplicationContext(), mAsrAudioRecorder);
        mVoiceSdk.addVoiceSdkListener(mVoiceSdkListener);

        if (SharedPreferenceUtils.getAssistantEnabled(getApplicationContext())){
            mAsyncEventsHandler.sendEmptyMessage(MSG_ESTABLISH_SVA_SESSION);
        }

        mScreenWakeup = new ScreenWakeupHelper(getApplicationContext());
        mContactsUploader = new ContactsUploader(getApplicationContext(), mVoiceSdk);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        String action = intent != null ? intent.getAction() : "";
        if (IWakeupSession.ACTION_RECOGNITION.equals(action)) {
            String smName = intent.getStringExtra(IWakeupSession.KEY_EXTRA_SM_NAME);
            int messageType = intent.getIntExtra(SoundTriggerManager.EXTRA_MESSAGE_TYPE,
                    SoundTriggerManager.FLAG_MESSAGE_TYPE_UNKNOWN);
            LogUtils.d(TAG, "onStartCommand: smName = " + smName
                    + " messageType = " + messageType);
            switch (messageType) {
                case SoundTriggerManager.FLAG_MESSAGE_TYPE_RECOGNITION_EVENT:
                    onReceivedRecognitionEvent(intent);
                    break;
                case SoundTriggerManager.FLAG_MESSAGE_TYPE_RECOGNITION_ERROR:
                    onReceivedRecognitionError(intent);
                    break;
                case SoundTriggerManager.FLAG_MESSAGE_TYPE_RECOGNITION_PAUSED:
                    onReceivedRecognitionPaused(intent);
                    break;
                case SoundTriggerManager.FLAG_MESSAGE_TYPE_RECOGNITION_RESUMED:
                    onReceivedRecognitionResumed(intent);
                    break;
            }
        }
        return START_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mReceiveMessenger.getBinder();
    }

    @Override
    public void onDestroy(){
        if (myHandlerThread != null){
            myHandlerThread.quitSafely();
        }
        mContactsUploader.release();

        //maybe need to release in async handler.
        mWakeupSession.releaseAllSvaSessions();

        mVoiceSdk.removeVoiceSdkListener(mVoiceSdkListener);
        handleReleaseVoiceSdk();

        super.onDestroy();
    }

    private void initializeAsyncEventsHandler(){
        myHandlerThread = new HandlerThread( "sva-recognition") ;
        myHandlerThread.start();
        mAsyncEventsHandler = new Handler( myHandlerThread.getLooper() ){
            @Override
            public void handleMessage(Message msg) {
                switch (msg.what){
                    case MSG_ESTABLISH_SVA_SESSION:
                        handleEstablishSvaSession();
                        break;
                    case MSG_TERMINATE_SVA_SESSION:
                        handleTerminateSvaSession();
                        break;
                    case MSG_RESTART_SVA_RECOGNITION:
                        handleRestartRecognition();
                        break;
                }
            }
        };
    }

    private void onReceivedRecognitionEvent(Intent intent){
        LogUtils.d(TAG, "onReceivedRecognitionEvent: enter");

        SoundTrigger.KeyphraseRecognitionEvent event = intent.getParcelableExtra(
                SoundTriggerManager.EXTRA_RECOGNITION_EVENT);
        LogUtils.d(TAG, "onReceivedRecognitionEvent: event = " + event);

        final IRecognitionEvent recognitionEvent = new RecognitionEvent(event);

        if (IRecognitionEvent.State.SUCCESS == recognitionEvent.getState()) {
            if (recognitionEvent.getCaptureAvailable()){
                startSecondDetectionOrRecognition(recognitionEvent);
                if (SharedPreferenceUtils.getIsSecondDetectionEnabled(getApplicationContext())){
                    SharedPreferenceUtils.increaseFirstDetectionCount();
                }
            }else {
                new WakeupTonePlayer(getApplicationContext()).play( new WakeupTonePlayer
                        .OnCompletionListener() {
                    @Override
                    public void onCompletion() {
                        startSecondDetectionOrRecognition(recognitionEvent);
                    }
                });
            }
        } else {
            sendRestartRecognitionRequest("sva exception status = "
                    +recognitionEvent.getState());
            LogUtils.d(TAG," SVA Wakeup exception == "+recognitionEvent.getState());
        }
    }

    private void onReceivedRecognitionError(Intent intent){
        LogUtils.d(TAG, "onReceivedRecognitionError: enter");
    }

    private void onReceivedRecognitionPaused(Intent intent){
        LogUtils.d(TAG, "onReceivedRecognitionPaused: enter");
    }

    private void onReceivedRecognitionResumed(Intent intent){
        LogUtils.d(TAG, "onReceivedRecognitionResumed: enter");
    }

    private void handleInitializeVoiceSdk(){
        if (!mVoiceSdkInitialized ) {
            mVoiceSdk.initializeSdk();
            mContactsUploader.updateContacts();
            mVoiceSdkInitialized = true;
            if (mVoicePostExecuteTask != null){
                mVoicePostExecuteTask.run();
                mVoicePostExecuteTask = null;
            }
        }
    }

    private void handleReleaseVoiceSdk(){
        if (mVoiceSdkInitialized) {
            mRecordingMgr.stopRecord();
            mSecondDetection.stopSecondDetection();
            mVoiceSdk.stopRecognition();
            mVoiceSdk.releaseSdk();
            mVoiceSdkInitialized = false;
            mVoicePostExecuteTask = null;
        }
    }

    private void handleEstablishSvaSession(){
        final int ret = establishSvaSession();
        sendMessageToClients(Messages.START_RECOGNITION_RESPONSE, ret);
    }

    private void handleTerminateSvaSession(){
        int ret = mWakeupSession.terminateSvaSession(mCurrentSoundModelName);
        sendMessageToClients(Messages.STOP_RECOGNITION_RESPONSE, ret);
    }

    private void handleRestartRecognition(){
        if (!mWakeupSession.isRecognitionActive(mCurrentSoundModelName)) {
            mWakeupSession.restartRecognition(mCurrentSoundModelName);
        } else {
            LogUtils.d(TAG, "handleRestartRecognition do nothing for recognition active");
        }
    }

    /**
     * Restart recognition conditions
     * 1.Asr audio record released and then restart recognition.
     * 2.when receive exception recognition event, restart recognition.
     */
    private void sendRestartRecognitionRequest(String reason){
        LogUtils.e(TAG, "restart recognition reason = "+ reason);
        mAsyncEventsHandler.sendMessage(Message.obtain(null, MSG_RESTART_SVA_RECOGNITION));
    }

    private int establishSvaSession(){
        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(getTargetSmName());
        if (extendedSmModel != null
                && IExtendedSmModel.SessionStatus.STARTED == extendedSmModel.getSessionStatus()){
            return IWakeupSession.RC_OK;
        }else {
            mCurrentSoundModelName = getTargetSmName();
            return mWakeupSession.establishSvaSession(getTargetSmName());
        }
    }

    private void startSecondDetectionOrRecognition(final IRecognitionEvent recognitionEvent){
        if (mSecondDetection.isEnabled() && recognitionEvent.getCaptureAvailable()){
            mSecondDetection.startSecondDetection(recognitionEvent);
            mRecordingMgr.startRecord(recognitionEvent);
        }else {
            if (NetWorkUtil.isNetworkConnected(getApplicationContext())){
                startRecognition();
                mRecordingMgr.startRecord(recognitionEvent);
            }else {
                Toast.makeText(getApplicationContext(), getApplicationContext().getResources().
                        getString(R.string.err_net_msg), Toast.LENGTH_SHORT).show();
                sendRestartRecognitionRequest("network exception");
            }
            startMainActivity();
            mScreenWakeup.wakeup();
        }
    }

    /**
     * must start voice recognition after sdk initialized
     */
    private void startRecognition(){
        if (mVoiceSdkInitialized) {
            mVoiceSdk.startRecognition();
        } else {
            mVoicePostExecuteTask = new Runnable() {
                @Override
                public void run() {
                    mVoiceSdk.startRecognition();
                }
            };
        }
    }

    private void initAsrRecordingListener(){
        mAsrAudioRecorder.setAsrRecordingListener(
                new AbstractDuerAudioRecorder.IAsrRecordingListener() {
            @Override
            public void onAsrRecordingStarted() {
                sendMessageToClients(Messages.START_VOICE_INPUT_RESPONSE, null);
            }

            @Override
            public void onAsrRecordingStopped() {
                sendMessageToClients(Messages.STOP_VOICE_INPUT_RESPONSE, null);
                sendRestartRecognitionRequest("asr voice stop");
            }
        });
    }

    private void initSecondDetectionListener(){
        mSecondDetection.addSecondStageDetectionListener(
                new ISecondStageDetection.IDetectionListener() {
            @Override
            public void onHotwordDetected() {
                if (NetWorkUtil.isNetworkConnected(getApplicationContext())){
                    startRecognition();
                }else {
                    Toast.makeText(getApplicationContext(), getApplicationContext().getResources().
                            getString(R.string.err_net_msg), Toast.LENGTH_SHORT).show();
                    mRecordingMgr.stopRecord();
                }
                startMainActivity();
                mScreenWakeup.wakeup();
            }

            @Override
            public void onHotwordUndetected() {
                mRecordingMgr.stopRecord();
            }
        });
    }

    private String getTargetSmName(){
        boolean isUserVerificationEnabled =
                SharedPreferenceUtils.getIsUserVerificationEnabled(getApplicationContext())
                && FileUtils.isFileExist(FileUtils.getSMFilePath(XIAODUXIAODU_UDM_NAME));
        return isUserVerificationEnabled ? XIAODUXIAODU_UDM_NAME : XIAODUXIAODU_PDK_NAME;
    }

    private void startMainActivity(){
        Intent intent = new Intent(getApplicationContext(), SmartAssistantActivity.class);
        intent.putExtra(STARTED_BY_ASSISTANT_SERVICE, true);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        startActivity(intent);
    }

    // Send a message with obj data to all registered clients
    private void sendMessageToClients(int what, Object obj) {
        Iterator<Messenger> iterator = mClients.iterator();
        while (iterator.hasNext()){
            Messenger messenger = iterator.next();
            try {
                Message msg = Message.obtain(null, what, obj);
                messenger.send(msg);
            }catch (RemoteException e){
                iterator.remove();
            }
        }
    }

    private Context getActivityContext() {
        final int size = mClientContexts.size();
        return size> 0 ? mClientContexts.get(size -1): null;
    }
}
