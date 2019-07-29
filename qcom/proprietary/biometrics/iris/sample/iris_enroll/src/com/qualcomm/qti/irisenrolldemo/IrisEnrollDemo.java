/*
 * Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.qualcomm.qti.irisenrolldemo;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.DialogFragment;
import android.app.Fragment;
import android.app.FragmentTransaction;
import android.content.Context;
import android.content.DialogInterface;
import android.graphics.Color;
import android.graphics.PorterDuff;

import com.qualcomm.qti.iris.Iris;
import com.qualcomm.qti.iris.IrisManager;
import com.qualcomm.qti.iris.IrisOperationStatus;
import com.qualcomm.qti.iris.IrisError;

import android.media.AudioManager;
import android.media.ToneGenerator;
import android.os.Bundle;
import android.os.CancellationSignal;
import android.os.PowerManager;
import android.os.Vibrator;
import android.util.Log;
import android.util.Size;
import android.view.Gravity;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class IrisEnrollDemo extends Activity implements SurfaceHolder.Callback {

    private static final String TAG = "IrisMainActivity";

    /**
     * Authentication status
     */
    private enum Status {
        STOPPED, ENROLLING, STARTING, NO_MATCH, MODIFYING
    }

    private Status mCurrentStatus;

    /**
     * Current status or states of the app
     */
    private TextView mTextViewInfo;

    public static IrisManager mIrisManager;
    CancellationSignal mCancelSignal;

    // Surface view related
    private IrisManager.IrisSurfaceView mSurfaceView;
    private Surface mSurface;
    private View mParent;
    private int mWidth = 1920, mHeight = 1080;

    // Tone generator, used to create sound for verification
    private ToneGenerator mToneGen = new ToneGenerator(AudioManager.STREAM_MUSIC, 100);

    // Enrollment list view
    private ListView mEnrollListView;
    private ArrayAdapter<String> mEnrollListAdapter = null;
    private final ArrayList<String> mEnrollList = new ArrayList<>();
    private int mEnrollmentProgress = -1;
    private byte[] mToken = new byte[69];

    // Round progress bar for enrollment
    private ProgressBar mEnrollLeftProgressBar;
    private ProgressBar mEnrollRightProgressBar;

    private Toast mToast;
    private long mStartTime = 0;
    private ProgressBar mTimerBar;

    private ImageButton mBtnEnroll, mBtnCancel;
    private iPassEnrollmentCallback mEnrollmentCallback;

    private PowerManager.WakeLock mWakeLock;

    // SurfaceHolder callbacks
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (mParent != null && mParent.getWidth() > 0) {
            FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) mSurfaceView.getLayoutParams();
            params.width = mParent.getWidth();
            params.height = params.width * mHeight / mWidth;
            mSurfaceView.setLayoutParams(params);
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        Log.v(TAG, "surfaceCreated");
        mSurface = holder.getSurface();
        if (mSurface == null) {
            Log.e(TAG, "mSurface is null!");
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
    }


    private class iPassEnrollmentCallback extends IrisManager.EnrollmentCallback {
        private IrisEnrollDemo mainActivity;

        private void setMainActivity(IrisEnrollDemo activity) {
            mainActivity = activity;
        }

        @Override
        public void onEnrollmentStatus(IrisOperationStatus status) {
            Log.v(TAG, "onEnrollmentStatus progress: " + status.mProgress);
            if (status.mQuality != 0) {
                mToast.setText(status.mHelpString);
                Log.v(TAG, "quality=" + status.mQuality);
                if (status.mHelpString != null)
                    Log.v(TAG, status.mHelpString.toString());
                mToast.show();
            } else {
                mToast.cancel();
            }
            if (mCurrentStatus != Status.ENROLLING) {
                mCurrentStatus = Status.ENROLLING;
                updateViews();
            }

            mEnrollmentProgress = status.mProgress;
            mEnrollLeftProgressBar.setProgress(mEnrollmentProgress, true);
            mEnrollRightProgressBar.setProgress(mEnrollmentProgress, true);
            postInfo("Enrolling...");

            long timer = (System.currentTimeMillis() - mStartTime) / 400;
            mTimerBar.setProgress(100 - (int) timer, true);
        }

        @Override
        public void onEnrollmentDone(int irisId) {
            Log.v(TAG, "onEnrollmentDone " + irisId);
            int result = mIrisManager.postEnroll();
            if (result < 0) {
                Log.v(TAG, "postEnroll failed: result = " + result);
            }
            mEnrollmentProgress = 100;
            mCurrentStatus = Status.STOPPED;
            mEnrollLeftProgressBar.setProgress(100, true);
            mEnrollRightProgressBar.setProgress(100, true);
            postInfo("Enrollment completed");
            refreshEnrollList();
            updateViews();
            resetViews();
        }

        @Override
        public void onEnrollmentError(IrisError error) {
            Log.v(TAG, "*** onEnrollmentError *** " + error.mErrorDesc.toString());
            mCurrentStatus = Status.STOPPED;
            // no need to show message for operation cancel
            if (error.mErrorId != 3) {
                if (error.mErrorDesc.toString().equals("Looking for your iris...")) {
                    postInfo("Iris cannot be verified!");
                } else {
                    postInfo(error.mErrorDesc.toString());
                }
                Vibrator v = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
                v.vibrate(100);
                mToneGen.startTone(ToneGenerator.TONE_CDMA_ABBR_INTERCEPT, 500);
                android.os.Handler handler = new android.os.Handler();
                handler.postDelayed(new Runnable() {
                    @Override
                    public void run() {
                        mainActivity.refreshEnrollList();
                        mainActivity.updateViews();
                        mainActivity.resetViews();
                    }
                }, 1000);
                return;
            }
            updateViews();
        }
    }

    private void updateViews() {
        if (Status.STOPPED == mCurrentStatus || Status.NO_MATCH == mCurrentStatus) {
            mBtnEnroll.setEnabled(true);
            mBtnEnroll.setClickable(true);
            mBtnEnroll.setVisibility(View.VISIBLE);
            mBtnCancel.setVisibility(View.GONE);
            enableEnrollListView(true);
            mBtnCancel.getDrawable().setColorFilter(null);
        } else if (Status.STARTING == mCurrentStatus) {
            mBtnEnroll.setVisibility(View.GONE);
            mBtnCancel.setVisibility(View.VISIBLE);
            mBtnCancel.setEnabled(false);
            mBtnCancel.setClickable(false);
            mBtnCancel.getDrawable().setColorFilter(Color.GRAY, PorterDuff.Mode.SRC_IN);
            enableEnrollListView(false);
        } else if (Status.MODIFYING == mCurrentStatus) {
            mBtnEnroll.setEnabled(false);
            mBtnEnroll.setClickable(false);
        } else {
            mBtnEnroll.setVisibility(View.GONE);
            mBtnCancel.setVisibility(View.VISIBLE);
            mBtnCancel.setEnabled(true);
            mBtnCancel.setClickable(true);
            mBtnCancel.getDrawable().setColorFilter(null);
            enableEnrollListView(false);
        }
    }

    private void enableEnrollListView(boolean enable) {
        if (enable) {
            mEnrollListView.setOnItemClickListener(new OnItemClickListener() {
                public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
                    onEnrollmentListViewItemClickMethod(arg0, arg1, position, arg3);
                }
            });
        } else
            mEnrollListView.setOnItemClickListener(null);
    }

    @Override
    protected void onResume() {
        super.onResume();

        postInfo("");
        mIrisManager = new IrisManager(this);
        Size previewSize = mIrisManager.getPreviewSize(IrisManager.IRIS_PREVIEW_SIZE_AUTH);

        if (previewSize != null) {
            mWidth = previewSize.getWidth();
            mHeight = previewSize.getHeight();
            Log.d(TAG, "Preview size - height: " + mHeight + " width: " + mWidth);
        } else
            Log.v(TAG, "fail to get preview size");

        mParent = findViewById(R.id.surface_parent);

        mSurfaceView = (IrisManager.IrisSurfaceView) findViewById(R.id.texture);
        mSurfaceView.setIrisManager(mIrisManager);
        SurfaceHolder mSurfaceHolder = mSurfaceView.getHolder();
        mSurfaceHolder.addCallback(IrisEnrollDemo.this);

        //Set the the camera preview TextureView with the same layout params as the SurfaceView
        mSurfaceHolder.setFixedSize(mWidth, mHeight);
        mEnrollListAdapter.notifyDataSetChanged();
        refreshEnrollList();
        resetViews();
        mEnrollLeftProgressBar.setProgress(0, true);
        mEnrollRightProgressBar.setProgress(0, true);
        mToast.cancel();
        if (mEnrollLeftProgressBar.getVisibility() == View.VISIBLE) {
            mEnrollLeftProgressBar.setVisibility(View.INVISIBLE);
        }
        if (mEnrollRightProgressBar.getVisibility() == View.VISIBLE) {
            mEnrollRightProgressBar.setVisibility(View.INVISIBLE);
        }
        updateViews();

    }


    @Override
    public void onWindowFocusChanged(boolean hasFocus) {
        if (hasFocus) {
            refreshEnrollList();
        }
    }

    @Override
    protected void onPause() {
        if (mCancelSignal != null)
            mCancelSignal.cancel();
        mCurrentStatus = Status.STOPPED;
        refreshEnrollList();
        updateViews();
        resetViews();
        postInfo("");
        super.onPause();
    }

    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");
        setContentView(R.layout.ipass_main);

        mTextViewInfo = (TextView) findViewById(R.id.textViewInfo);
        mTimerBar = (ProgressBar) findViewById(R.id.timerbar);

        mCurrentStatus = Status.STOPPED;

        PowerManager mPowerManager = (PowerManager) getSystemService(POWER_SERVICE);
        mWakeLock = mPowerManager.newWakeLock(PowerManager.FULL_WAKE_LOCK, "IrisEnrollDemoWakeLock");

        mBtnCancel = (ImageButton) findViewById(R.id.btnCancelAuth);
        mBtnCancel.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (Status.STOPPED != mCurrentStatus && Status.STARTING != mCurrentStatus) {
                    mBtnCancel.setEnabled(false);
                    handle_cancel();
                }
            }
        });

        mBtnEnroll = (ImageButton) findViewById(R.id.btn_enroll);
        mBtnEnroll.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                // enroll
                if (mCurrentStatus == Status.STOPPED) {
                    mWakeLock.acquire();
                    mBtnEnroll.setEnabled(false);
                    mBtnEnroll.setClickable(false);
                    mCurrentStatus = Status.STARTING;
                    updateViews();
                    mStartTime = System.currentTimeMillis();
                    enrollIris();
                }
            }
        });

        // Two rounded progress bars for enrollment
        mEnrollLeftProgressBar = (ProgressBar) findViewById(R.id.enrollProgLeft);
        mEnrollRightProgressBar = (ProgressBar) findViewById(R.id.enrollProgRight);

        mToast = Toast.makeText(IrisEnrollDemo.this, "", Toast.LENGTH_SHORT);
        mToast.setGravity(Gravity.TOP | Gravity.START, 0, 0);

        mEnrollListView = (ListView) findViewById(R.id.enrolledList);
        mEnrollListAdapter = new ArrayAdapter<String>(IrisEnrollDemo.this, android.R.layout.simple_list_item_1, mEnrollList) {
            @Override
            public View getView(int position, View convertView, ViewGroup parent) {

                View view = super.getView(position, convertView, parent);
                TextView textview = (TextView) view.findViewById(android.R.id.text1);
                textview.setTextAlignment(View.TEXT_ALIGNMENT_CENTER);
                textview.setGravity(Gravity.CENTER);

                return view;
            }
        };

        mEnrollListView.setOnItemClickListener(new OnItemClickListener() {
            public void onItemClick(AdapterView<?> arg0, View arg1, int position, long arg3) {
                onEnrollmentListViewItemClickMethod(arg0, arg1, position, arg3);
            }
        });
        mEnrollListView.setAdapter(mEnrollListAdapter);
        mEnrollListView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);

        mEnrollmentCallback = new iPassEnrollmentCallback();
    }

    /**
     * Making this a method because it has been used more than once.
     *
     * @param arg0
     * @param arg1
     * @param position
     * @param arg3
     */
    private void onEnrollmentListViewItemClickMethod(AdapterView<?> arg0, View arg1, int position, long arg3) {
        mCurrentStatus = Status.MODIFYING;
        updateViews();

        List<Iris> enrolledIris = mIrisManager.getEnrolledIris();

        if (enrolledIris.size() <= position) {
            return;
        }

        Iris selectedIris = enrolledIris.get(position);

        FragmentTransaction ft = getFragmentManager().beginTransaction();
        Fragment prev = getFragmentManager().findFragmentByTag("dialog");
        if (prev != null) {
            ft.remove(prev);
        }
        ft.addToBackStack(null);

        // Create and show the dialog.
        ModifyIrisDialog renameDeleteDialog = ModifyIrisDialog.newInstance(selectedIris);
        renameDeleteDialog.setActivity(IrisEnrollDemo.this);
        renameDeleteDialog.show(ft, ModifyIrisDialog.class.getName());

        // Refresh the enroll list to reflect the latest changes
        resetEnrollListView();
        refreshEnrollList();
    }

    public void handle_cancel() {
        mWakeLock.release();
        if (mCurrentStatus == Status.STOPPED) {
            updateViews();
            return;
        }
        cancelEnrollment();
        resetViews();
    }

    /**
     * This method enrolls new iris
     */
    public void enrollIris() {
        if (mCurrentStatus == Status.ENROLLING)
            return;

        if (mIrisManager.hasEnrolledIris()) {
            List<Iris> enrolledIris = mIrisManager.getEnrolledIris();
            if (enrolledIris.size() >= 5) {
                postInfo("Cannot enroll more than 5 users");
                mCurrentStatus = Status.STOPPED;
                updateViews();
                return;
            }
        }

        resetEnrollListView();
        postInfo("Starting");

        mCancelSignal = new CancellationSignal();
        new Random().nextBytes(mToken);

        mEnrollmentCallback.setMainActivity(this);
        mIrisManager.enroll(mToken, mCancelSignal, 0, mEnrollmentCallback);

        if (mSurface == null) {
            mSurfaceView.setVisibility(View.VISIBLE);
            if (mSurface == null) {
                Log.e(TAG, "surface is still null before setting it!");
            }
            mIrisManager.setPreviewSurface(mSurface);
        }

        mEnrollLeftProgressBar.setVisibility(View.VISIBLE);
        mEnrollRightProgressBar.setVisibility(View.VISIBLE);
        mEnrollmentProgress = 0;
    }

    /**
     * This method posts info to the info TextView
     *
     * @param info information
     */
    private void postInfo(final String info) {
        mTextViewInfo.setText(info);
    }

    public void resetEnrollListView() {
        if (null == mIrisManager || null == mEnrollListView) {
            return;
        }

        List<Iris> enrolledIris = mIrisManager.getEnrolledIris();
        if (null == enrolledIris) {
            return;
        }

        for (int i = 0; i < enrolledIris.size(); i++) {
            if (null != mEnrollListView.getChildAt(i)) {
                mEnrollListView.getChildAt(i).setBackgroundColor(Color.TRANSPARENT);
            }
        }
    }

    /**
     * This method refreshes the iris enrollment list
     */
    public void refreshEnrollList() {
        Log.v(TAG, "refreshing irises enrollment list");
        if (null == mIrisManager || null == mEnrollListAdapter) {
            return;
        }

        mEnrollList.clear();
        List<Iris> enrolledIris = mIrisManager.getEnrolledIris();
        CharSequence irisName;

        if (null == enrolledIris) {
            return;
        }

        for (int i = 0; i < enrolledIris.size(); i++) {
            irisName = enrolledIris.get(i).getName();
            Log.v(TAG, "name=" + irisName);
            mEnrollList.add(irisName.toString());
        }
        mEnrollListAdapter.notifyDataSetChanged();
    }

    /**
     * This method cancels the on-going verification session
     */
    public void cancelEnrollment() {
        if (mCancelSignal != null) {
            mCancelSignal.cancel();
        }

        mToast.cancel();
        postInfo("");
    }

    /**
     * This method resets the UI to their default states
     */
    public void resetViews() {
        mSurfaceView.setVisibility(View.INVISIBLE);
        mTimerBar.setProgress(100, true);
        mEnrollLeftProgressBar.setProgress(0, true);
        mEnrollRightProgressBar.setProgress(0, true);
        mToast.cancel();
        mSurface = null;
        if (mEnrollLeftProgressBar.getVisibility() == View.VISIBLE) {
            mEnrollLeftProgressBar.setVisibility(View.INVISIBLE);
        }
        if (mEnrollRightProgressBar.getVisibility() == View.VISIBLE) {
            mEnrollRightProgressBar.setVisibility(View.INVISIBLE);
        }
        resetEnrollListView();
        updateViews();
    }

    public static class ModifyIrisDialog extends DialogFragment {
        private Iris mIris;
        private EditText mDialogTextField;
        private String mIrisName;
        private Boolean mTextHadFocus;
        private int mTextSelectionStart;
        private int mTextSelectionEnd;
        private IrisEnrollDemo mActivity;

        public static ModifyIrisDialog newInstance(final Iris iris) {
            ModifyIrisDialog dialogFragment = new ModifyIrisDialog();
            Bundle bundle = new Bundle();
            bundle.putParcelable("iris", iris);
            dialogFragment.setArguments(bundle);
            return dialogFragment;
        }

        public void setActivity(IrisEnrollDemo activity) {
            mActivity = activity;
        }

        @Override
        public void onDismiss(DialogInterface dialog) {
            super.onDismiss(dialog);
            if (Status.STOPPED != mActivity.mCurrentStatus) {
                mActivity.mCurrentStatus = Status.STOPPED;
                mActivity.updateViews();
            }
        }

        @Override
        public Dialog onCreateDialog(Bundle savedInstanceState) {
            mIris = getArguments().getParcelable("iris");
            if (savedInstanceState != null) {
                mIrisName = savedInstanceState.getString("irisName");
                mTextHadFocus = savedInstanceState.getBoolean("textHadFocus");
                mTextSelectionStart = savedInstanceState.getInt("startSelection");
                mTextSelectionEnd = savedInstanceState.getInt("endSelection");
            }
            final AlertDialog alertDialog = new AlertDialog.Builder(getActivity())
                    .setView(R.layout.iris_rename_dialog)
                    .setPositiveButton(
                            R.string.iris_enroll_dialog_rename,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    final String newName =
                                            mDialogTextField.getText().toString();
                                    final CharSequence name = mIris.getName();
                                    if (!newName.equals(name)) {
                                        IrisEnrollDemo.mIrisManager.rename(mIris.getIrisId(), newName);
                                    }
                                    dialog.dismiss();
                                }
                            })
                    .setNeutralButton(
                            R.string.iris_enroll_dialog_delete,
                            new DialogInterface.OnClickListener() {
                                @Override
                                public void onClick(DialogInterface dialog, int which) {
                                    if (IrisEnrollDemo.mIrisManager.getEnrolledIris().size() > 0) {
                                        IrisEnrollDemo.mIrisManager.remove(mIris, new IrisManager.RemovalCallback() {
                                            @Override
                                            public void onRemovalError(Iris iris, IrisError error) {
                                                //
                                            }

                                            @Override
                                            public void onRemovalSucceeded(Iris iris) {
                                                if (Status.STOPPED != mActivity.mCurrentStatus) {
                                                    mActivity.mCurrentStatus = Status.STOPPED;
                                                    mActivity.updateViews();
                                                }
                                                mActivity.refreshEnrollList();
                                            }
                                        });
                                    }
                                    dialog.dismiss();
                                }
                            })
                    .create();

            alertDialog.setOnShowListener(new DialogInterface.OnShowListener() {
                @Override
                public void onShow(DialogInterface dialog) {
                    mDialogTextField = (EditText) alertDialog.findViewById(R.id.iris_rename_field);
                    CharSequence name = mIrisName == null ? mIris.getName() : mIrisName;
                    mDialogTextField.setText(name);
                    if (null == mTextHadFocus) {
                        mDialogTextField.selectAll();
                    } else {
                        mDialogTextField.setSelection(mTextSelectionStart, mTextSelectionEnd);
                    }
                }
            });

            if (null == mTextHadFocus || mTextHadFocus) {
                // Request the IME
                try {
                    alertDialog.getWindow().setSoftInputMode(
                            WindowManager.LayoutParams.SOFT_INPUT_STATE_ALWAYS_VISIBLE);
                } catch (NullPointerException e) {
                    Log.e(TAG, "IME not ready", e);
                }
            }
            return alertDialog;
        }

        @Override
        public void onSaveInstanceState(Bundle outState) {
            super.onSaveInstanceState(outState);
            if (mDialogTextField != null) {
                outState.putString("irisName", mDialogTextField.getText().toString());
                outState.putBoolean("textHadFocus", mDialogTextField.hasFocus());
                outState.putInt("startSelection", mDialogTextField.getSelectionStart());
                outState.putInt("endSelection", mDialogTextField.getSelectionEnd());
            }
        }
    }
}
