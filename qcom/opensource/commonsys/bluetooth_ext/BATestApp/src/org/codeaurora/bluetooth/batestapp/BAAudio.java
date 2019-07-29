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

import android.app.Service;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothBATransmitter;
import android.bluetooth.BluetoothBAEncryptionKey;
import android.bluetooth.BluetoothBAStreamServiceRecord;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.bluetooth.BluetoothProfile;
import android.util.Log;
import android.os.IBinder;

import java.util.Map;
import java.util.HashMap;

import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;
import android.media.AudioRecord;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;

import java.io.IOException;

import android.media.MediaRecorder;
import android.media.AudioManager;
import android.media.AudioManager.OnAudioFocusChangeListener;

import android.media.audiofx.AcousticEchoCanceler;
import android.media.audiofx.AutomaticGainControl;
import android.media.audiofx.NoiseSuppressor;

// class to handle all interactions with Audio part of Broadcast Audio Transmitter.
public class BAAudio {

    public static final String BASERVICE_STATE_CHANGED = "android.bluetooth.bat.service";
    public static final String EXTRA_BA_STATE = "android.bluetooth.extra.ba.state";
    public static final String EXTRA_CONN_STATE = "android.bluetooth.extra.conn.state";
    private static final String TAG = Utils.TAG + "BAAudio";
    private static final int SAMPLE_RATE = 16000;
    private static final int CHANNEL_CONFIG = AudioFormat.CHANNEL_CONFIGURATION_MONO;
    private static final int AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;
    private static final int RECORD_BUF_SIZE = AudioRecord.getMinBufferSize(SAMPLE_RATE,
            CHANNEL_CONFIG, AUDIO_FORMAT);
    private final static int MSG_STOP_RECORD_PLAY = 1;
    private final static int MSG_START_RECORD_PLAY = 2;
    private final static int MSG_AUDIO_FOCUS_CHANGE = 3;
    private static BluetoothBATransmitter sBATprofile = null;
    private static boolean sIsBAReady = false;
    private static BluetoothBAStreamServiceRecord mServiceRecord;
    private static boolean sIsPlaying = false;
    private BluetoothAdapter mAdapter;
    private BAAudioReceiver mReceiver;
    private Context mContext;
    private int mCurrBATState;
    private BluetoothBAEncryptionKey mCurrEncKey;
    private int mCurrDiv;
    private int mCurrStreamId;
    private AudioRecord mAudioRecord = null;
    private AudioTrack mAudioTrack = null;
    private int mCurrAudioFocusState = AudioManager.AUDIOFOCUS_LOSS;
    private AudioManager mAudioManager;
    private BAMsgHandler mHandler;
    private StreamingThread mStrThread;
    private OnAudioFocusChangeListener mAudioFocusListener = new OnAudioFocusChangeListener() {
        public void onAudioFocusChange(int focusVal) {
            Log.d(TAG, "focusChangs val = " + focusVal);
            if (mHandler != null)
                mHandler.obtainMessage(MSG_AUDIO_FOCUS_CHANGE, focusVal, 0).sendToTarget();
        }
    };

    public BAAudio(Context context) {
        Log.d(TAG, " BAAudio constructor");
        mContext = context;
        mReceiver = new BAAudioReceiver();
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        IntentFilter filter = new IntentFilter(BluetoothBATransmitter.ACTION_BAT_STATE_CHANGED);
        filter.addAction(BluetoothBATransmitter.ACTION_BAT_ENCRYPTION_KEY_CHANGED);
        filter.addAction(BluetoothBATransmitter.ACTION_BAT_DIV_CHANGED);
        filter.addAction(BluetoothBATransmitter.ACTION_BAT_STREAMING_ID_CHANGED);
        mContext.registerReceiver(mReceiver, filter);
        if (mAdapter != null) {
            mAdapter.getProfileProxy(mContext, new BATServiceListener(),
                    BluetoothProfile.BA_TRANSMITTER);
        }
        mCurrEncKey = null;
        mServiceRecord = null;
        mCurrDiv = BluetoothBATransmitter.INVALID_DIV;
        mCurrStreamId = 0;
        mCurrBATState = BluetoothBATransmitter.STATE_DISABLED;

        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        HandlerThread thread = new HandlerThread("BAaudioHandler");
        thread.start();
        Looper looper = thread.getLooper();
        mHandler = new BAMsgHandler(looper);

    }

    public void broadcastServiceConnection(boolean isConnected, int state) {
        Log.d(TAG, " Sending Intent service conn = " + isConnected + " ba_state = " + state);
        Intent intent = new Intent(BASERVICE_STATE_CHANGED);
        intent.putExtra(EXTRA_CONN_STATE, isConnected);
        intent.putExtra(EXTRA_BA_STATE, state);
        mContext.sendBroadcast(intent);
    }

    public void cleanup() {
        Log.v(TAG," cleanup ");
        if ((mCurrAudioFocusState == AudioManager.AUDIOFOCUS_GAIN) ||
                (mCurrAudioFocusState == AudioManager.AUDIOFOCUS_LOSS_TRANSIENT)) {
            mAudioManager.abandonAudioFocus(mAudioFocusListener);
        }
        if (mContext != null) {
            mContext.unregisterReceiver(mReceiver);
        }
    }

    public boolean enableBA() {
        Log.d(TAG, " enableBA currState = " + mCurrBATState);
        if (sBATprofile == null) {
            Log.d(TAG, " profile null, return");
            return false;
        }
        if ((mCurrBATState == BluetoothBATransmitter.STATE_PAUSED) ||
                (mCurrBATState == BluetoothBATransmitter.STATE_PLAYING)) {
            return false;
        }
        return sBATprofile.setBATState(BluetoothBATransmitter.ENABLE_BA_TRANSMITTER);
    }

    public boolean disableBA() {
        Log.d(TAG, " disableBA currState = " + mCurrBATState);
        if (sBATprofile == null) {
            Log.d(TAG, " profile null, return");
            return false;
        }
        if (mCurrBATState == BluetoothBATransmitter.STATE_DISABLED) {
            return false;
        }
        return sBATprofile.setBATState(BluetoothBATransmitter.DISABLE_BA_TRANSMITTER);
    }

    public int getBATState() {
        Log.d(TAG, " getBATState currState = " + mCurrBATState);
        if (sBATprofile != null) {
            mCurrBATState = sBATprofile.getBATState();
        }
        Log.d(TAG, " getBATState returning  = " + mCurrBATState);
        return mCurrBATState;
    }

    public BluetoothBAEncryptionKey getEncKey() {
        Log.d(TAG, " getEncKey currState = " + mCurrBATState);
        if (sBATprofile != null) {
            mCurrEncKey = sBATprofile.getEncryptionKey();
        }
        for (int i = 0; i < BluetoothBAEncryptionKey.ENCRYPTION_KEY_LENGTH; i++) {
            if (mCurrEncKey != null)
                Log.d(TAG, " EncrycptionKey[ " + i + "] = " + mCurrEncKey.getEncryptionKey()[i]);
        }
        return mCurrEncKey;
    }

    public int getStreamId() {
        Log.d(TAG, " getStreamId mCurrStreamId = " + mCurrStreamId);
        if (sBATprofile != null) {
            mCurrStreamId = (int) sBATprofile.getStreamId();
        }
        Log.d(TAG, " getStreamid returning  = " + mCurrStreamId);
        return mCurrStreamId;
    }

    public int getDIV() {
        Log.d(TAG, " getDIV mCurrDiv = " + mCurrDiv);
        if (sBATprofile != null) {
            mCurrDiv = sBATprofile.getDIV();
        }
        Log.d(TAG, " getDIV returning  = " + mCurrDiv);
        return mCurrDiv;
    }

    public boolean refreshEncryptionKey() {
        Log.d(TAG, " refreshEncryptionKey mCurrBATState = " + mCurrBATState);
        if (sBATprofile == null) {
            return false;
        }
        return sBATprofile.refreshEncryptionKey();
    }

    public BluetoothBAStreamServiceRecord getBAServiceRecord() {
        Log.d(TAG, " getBAServiceRecord mCurrBATState = " + mCurrBATState);
        if (sBATprofile == null) {
            Log.d(TAG, " Profile not up, return null ");
            return null;
        }
        try {
            mServiceRecord = sBATprofile.getBAServiceRecord();
            Long[] streamIDs = mServiceRecord.getStreamIds();
            Log.d(TAG, " streamIDs =  " + streamIDs.length + "streamId = " + streamIDs[0]);
            Map<Integer, Long> mServiceRecordData = mServiceRecord.getServiceRecord(streamIDs[0]);
            for (Map.Entry<Integer, Long> entry : mServiceRecordData.entrySet()) {
                Log.d(TAG, " Key< " + entry.getKey() + " >" + " value <" + entry.getValue() + ">");
            }
        } catch (Exception e) {
            Log.d(TAG, " Exception occured ");
            return null;
        }
        return mServiceRecord;
    }

    private synchronized void initAudioRecordSink() {
        mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.VOICE_COMMUNICATION, SAMPLE_RATE,
                CHANNEL_CONFIG, AUDIO_FORMAT, RECORD_BUF_SIZE);
        Log.d(TAG," mAudioRecord initialized = " + mAudioRecord.getState());
        if (AutomaticGainControl.isAvailable()) {
            AutomaticGainControl agc = AutomaticGainControl.create(mAudioRecord.getAudioSessionId
                    ());
            if (agc != null) {
                Log.d(TAG, "AGC is " + (agc.getEnabled() ? "enabled" : "disabled"));
                agc.setEnabled(true);
                Log.d(TAG, "AGC is " + (agc.getEnabled() ? "enabled" : "disabled"
                                        + " after trying to" + " enable"));
            }
        } else {
            Log.d(TAG, "AGC is unavailable");
        }

        if (NoiseSuppressor.isAvailable()) {
            NoiseSuppressor ns = NoiseSuppressor.create(mAudioRecord.getAudioSessionId());
            if (ns != null) {
                Log.d(TAG, "NS is " + (ns.getEnabled() ? "enabled" : "disabled"));
                ns.setEnabled(true);
                Log.d(TAG, "NS is " + (ns.getEnabled() ? "enabled" : "disabled"
                                       + " after trying to " + "disable"));
            }
        } else {
            Log.d(TAG, "NS is unavailable");
        }
        if (AcousticEchoCanceler.isAvailable()) {

            AcousticEchoCanceler aec = AcousticEchoCanceler.create(
                mAudioRecord.getAudioSessionId());
            if (aec != null) {
                Log.d(TAG, "AEC is " + (aec.getEnabled() ? "enabled" : "disabled"));
                aec.setEnabled(true);
                Log.d(TAG, "AEC is " + (aec.getEnabled() ? "enabled" : "disabled" + " after trying to" +
                        " disable"));
            }
        } else {
            Log.d(TAG, "aec is unavailable");
        }
        mAudioTrack = new AudioTrack(AudioManager.STREAM_MUSIC, SAMPLE_RATE, CHANNEL_CONFIG,
                AUDIO_FORMAT,
                RECORD_BUF_SIZE, AudioTrack.MODE_STREAM);
    }

    public boolean startRecordAndPlay() {
        if (mHandler != null) {
            if (mCurrAudioFocusState == AudioManager.AUDIOFOCUS_LOSS_TRANSIENT) {
                Log.d(TAG, " Transient Loss , don't start now ");
                return false;
            }
            mHandler.obtainMessage(MSG_START_RECORD_PLAY).sendToTarget();
            return true;
        }
        return false;
    }

    public void stopRecordAndPlay() {
        if (mHandler != null) {
            if (mStrThread == null) {
                Log.d(TAG, " Streaming already stopped, bail out ");
                return;
            }
            mHandler.obtainMessage(MSG_STOP_RECORD_PLAY).sendToTarget();
        }
    }

    class BATServiceListener implements BluetoothProfile.ServiceListener {
        @Override
        public void onServiceConnected(int profile, BluetoothProfile proxy) {
            Log.d(TAG, " onServiceConnected profile = " + profile);
            sBATprofile = (BluetoothBATransmitter) proxy;
            sIsBAReady = true;
            mCurrBATState = sBATprofile.getBATState();
            broadcastServiceConnection(true, mCurrBATState);
        }

        @Override
        public void onServiceDisconnected(int profile) {
            Log.d(TAG, " onServiceDisconnected profile = " + profile);
            sIsBAReady = false;
            mCurrBATState = BluetoothBATransmitter.STATE_DISABLED;
            sBATprofile = null;
            broadcastServiceConnection(false, mCurrBATState);
        }
    }

    private class BAAudioReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            int extraVal = 0;
            Log.d(TAG, action);
            if (action.equals(BluetoothBATransmitter.ACTION_BAT_STATE_CHANGED)) {
                extraVal = intent.getIntExtra(BluetoothBATransmitter.EXTRA_STATE,
                        -1);
                if (extraVal != -1)
                    mCurrBATState = extraVal;
                Log.d(TAG, " ACTION_BAT_STATE_CHANGED mCurrBATState = " + mCurrBATState + " " +
                        "extraVal = "
                        + extraVal);
            }
            if (action.equals(BluetoothBATransmitter.ACTION_BAT_ENCRYPTION_KEY_CHANGED)) {
                mCurrEncKey = (BluetoothBAEncryptionKey) intent.getParcelableExtra
                        (BluetoothBATransmitter.EXTRA_ECNRYPTION_KEY);
                Log.d(TAG, " ACTION_BAT_ENCRYPTION_KEY_CHANGED ");
            }
            if (action.equals(BluetoothBATransmitter.ACTION_BAT_DIV_CHANGED)) {
                extraVal = intent.getIntExtra(BluetoothBATransmitter.EXTRA_DIV_VALUE,
                        -1);
                if (extraVal != -1)
                    mCurrDiv = extraVal;
                Log.d(TAG, " ACTION_BAT_DIV_CHANGED mCurrDiv = " + mCurrDiv + "extraVal = " +
                        extraVal);
            }
            if (action.equals(BluetoothBATransmitter.ACTION_BAT_STREAMING_ID_CHANGED)) {
                extraVal = intent.getIntExtra(BluetoothBATransmitter.EXTRA_STREAM_ID,
                        -1);
                if (extraVal != -1)
                    mCurrStreamId = extraVal;
                Log.d(TAG, " ACTION_BAT_STREAMING_ID_CHANGED mCurrStreamId = "
                        + mCurrStreamId + " extraVal = " + extraVal);
            }
        }
    }

    private class StreamingThread extends Thread {
        public void run() {
            if ((mAudioRecord == null) || (mAudioTrack == null)) {
                initAudioRecordSink();
                Log.d(TAG, "mAudioRecord and mAudioTrack object created");
            }
            Log.d(TAG, " StreamThread Starts + ");
            mAudioTrack.setPlaybackRate(SAMPLE_RATE);
            byte[] buffer = new byte[RECORD_BUF_SIZE];
            mAudioRecord.startRecording();
            Log.d(TAG, "Audio Recording started");
            mAudioTrack.play();
            Log.d(TAG, "Audio Playing started");
            while (!Thread.interrupted()) {
                try {
                    int sizee = mAudioRecord.read(buffer, 0, RECORD_BUF_SIZE);
                    if (sizee != 0)
                        Log.d(TAG, "Reading first");
                    else
                        Log.d(TAG, "Something wrong");

                    mAudioTrack.write(buffer, 0, buffer.length);
                } catch (Throwable t) {
                    Log.d(TAG, "Read write failed- ERROR");
                    t.printStackTrace();
                    break;
                }
            }
            Log.d(TAG, " StreamThread Stops - ");
        }
    }

    private final class BAMsgHandler extends Handler {
        private BAMsgHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.v(TAG, " handleMessage what :"+msg.what);
            switch (msg.what) {

                case MSG_START_RECORD_PLAY:
                    Log.d(TAG, " Current Audio Focus = " + mCurrAudioFocusState);
                    if (mCurrAudioFocusState == AudioManager.AUDIOFOCUS_LOSS_TRANSIENT)
                        Log.d(TAG, " Transient Loss occused, call must be in progress, don't " +
                                "start now ");
                    int focusGranted = mAudioManager.requestAudioFocus(mAudioFocusListener,
                            AudioManager.STREAM_MUSIC,
                            AudioManager.AUDIOFOCUS_GAIN);
                    Log.d(TAG, " Focus Granted = " + focusGranted);
                    if (focusGranted == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
                        if (mStrThread == null) {
                            mStrThread = new StreamingThread();
                            mStrThread.start();
                        }
                    }
                    break;

                case MSG_STOP_RECORD_PLAY:
                    if (mStrThread == null) {
                        Log.d(TAG, " Streaming thread null ");
                        break;
                    }
                    if (!mStrThread.isAlive()) {
                        Log.d(TAG, " Streaming thread not active ");
                        break;
                    }
                    mStrThread.interrupt();
                    try {
                        mStrThread.join();
                    } catch (InterruptedException e) {
                        Log.d(TAG, " Thread Interrupted while joining ");
                        e.printStackTrace();
                    }
                    if (mAudioRecord != null) {
                        mAudioRecord.stop();
                        mAudioRecord.release();
                        mAudioRecord = null;
                    }
                    if (mAudioTrack != null) {
                        mAudioTrack.stop();
                        mAudioTrack.release();
                        mAudioTrack = null;
                    }
                    mStrThread = null;
                    if (mCurrAudioFocusState == AudioManager.AUDIOFOCUS_GAIN)
                        mAudioManager.abandonAudioFocus(mAudioFocusListener);
                    break;
                case MSG_AUDIO_FOCUS_CHANGE:
                    mCurrAudioFocusState = msg.arg1;
                    switch (mCurrAudioFocusState) {
                        case AudioManager.AUDIOFOCUS_GAIN:
                            if (mStrThread == null) {
                                mStrThread = new StreamingThread();
                                mStrThread.start();
                            }
                            break;
                        case AudioManager.AUDIOFOCUS_GAIN_TRANSIENT:
                            break;
                        case AudioManager.AUDIOFOCUS_LOSS:
                            // abandon audio focus in this case
                            mAudioManager.abandonAudioFocus(mAudioFocusListener);
                            mHandler.obtainMessage(MSG_STOP_RECORD_PLAY).sendToTarget();
                            break;
                        case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                            mHandler.obtainMessage(MSG_STOP_RECORD_PLAY).sendToTarget();
                            break;
                    }
                    break;
            }
        }
    }
}
