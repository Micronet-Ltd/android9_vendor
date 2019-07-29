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
package com.qualcomm.qti.iris;

import android.app.Activity;
import android.graphics.Color;
import android.graphics.PorterDuff;

import android.os.Bundle;
import android.os.CancellationSignal;
import android.os.PowerManager;
import android.util.Log;
import android.util.Size;
import android.view.Gravity;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.FrameLayout;
import android.widget.ImageButton;
import android.widget.ListView;
import android.widget.ProgressBar;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;

public class IrisAuthenticationDemo extends Activity implements SurfaceHolder.Callback {

    /**
     * Debug tag
     */
    private static final String TAG = "Iris_Auth_Demo";

    /**
     * Authentication status
     */
    private enum Status {
        STOPPED, STARTING, VERIFYING, NO_MATCH, MODIFYING
    }

    private Status mCurrentStatus;

    /**
     * Current status or states of the app
     */
    private TextView mTextViewInfo;

    /**
     * Instance of iris manager
     */
    private static IrisManager mIrisManager;

    /**
     * Cancel signal, required for cancelling enrollment or authentication
     */
    private CancellationSignal mCancelSignal;

    /**
     * IrisSurfaceView, for displaying camera preview
     */
    private IrisManager.IrisSurfaceView mSurfaceView;


    /**
     * Surface for the surface view
     */
    private Surface mSurface;

    /**
     * Frame layout holding the IrisSurfaceView
     */
    private View mFrameLayout;

    /**
     * Preview layout parameters with default value, it will get changed in surfaceChanged method
     */
    private int mWidth = 1920, mHeight = 1080;

    // Enrollment list view
    private ListView mEnrollListView;
    private ArrayAdapter<String> mEnrollListAdapter = null;
    private final ArrayList<String> mEnrollList = new ArrayList<>();

    private Toast mToast;
    private ProgressBar mTimerBar;
    private ImageButton mBtnVerify, mBtnCancelAuth;
    private PowerManager.WakeLock mWakeLock;

    /* Surface holder callbacks. Implement these if a camera preview is desired */
    @Override
    public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        // If implementing the IrisSurfaceView in a FrameLayout, developer should also change
        // the layout params of the FrameLayout
        // mFrameLayout here is the reference to this FrameLayout mentioned above.
        if (mFrameLayout != null && mFrameLayout.getWidth() > 0) {
            FrameLayout.LayoutParams params = (FrameLayout.LayoutParams) mSurfaceView.getLayoutParams();
            params.width = mFrameLayout.getWidth();
            params.height = params.width * mHeight / mWidth;
            mSurfaceView.setLayoutParams(params);
        }
    }

    @Override
    public void surfaceCreated(SurfaceHolder holder) {
        // This is called when the surface become visible (i.e. after setting IrisSurfaceView to
        // visible)
        // Obtain a ref to the Surface object from SurfaceHolder of IrisSurfaceView for camera preview
        mSurface = holder.getSurface();
        if (mSurface == null) {
            Log.d(TAG, "mSurface is null after surface created!!");
        }
    }

    @Override
    public void surfaceDestroyed(SurfaceHolder holder) {
        //Nothing required in this method for Iris Solution
    }
    /* Surface holder callbacks ends */

    /**
     * Iris authentication callback implementation
     */
    private class mAuthenticationCallback extends IrisManager.AuthenticationCallback {
        /**
         * Called when an unrecoverable error has been encountered and the operation is complete.
         * No further callbacks will be made on this object.
         *
         * @param error contains all error information including an error message id and A human-
         *              readable error string that can be shown in UI
         */
        @Override
        public void onAuthenticationError(IrisError error) {
            // 3 stands for cancel, so it's not really an error
            if (error.mErrorId != 3) {
                postInfo(error.mErrorDesc.toString());
            }
            mCurrentStatus = Status.STOPPED;
            resetViews();
        }

        /**
         * Called when a recoverable error has been encountered or useful info posting back during
         * authentication The help string is provided to give the user guidance for what went wrong,
         * such as "Sensor dirty, please clean it."
         *
         * @param status Contains an integer identifying the error, and a human-readable string that
         *               can be shown in UI
         */
        @Override
        public void onAuthenticationStatus(IrisOperationStatus status) {
            // Put the status/button updates here to avoid race condition when people
            // continuously tapping on verify and cancel
            if (mCurrentStatus != Status.VERIFYING) {
                mCurrentStatus = Status.VERIFYING;
                updateViews();
            }
            if (status.mQuality != 0) {
                mToast.setText(status.mHelpString);
                Log.v(TAG, "quality=" + status.mQuality);
                if (status.mHelpString != null)
                    Log.v(TAG, status.mHelpString.toString());
                mToast.show();
            } else {
                mToast.cancel();
            }
        }

        /**
         * Called when an iris is recognized, which matches one of the saved templates.
         *
         * @param result Not used
         */
        @Override
        public void onAuthenticationSucceeded(IrisManager.AuthenticationResult result) {
            mCurrentStatus = Status.STOPPED;
            updateViews();
            resetViews();
            refreshEnrollList();

            postInfo("Iris authenticated.");
        }

        /**
         * Called when an iris is valid but not recognized (i.e. does not match any saved templates)
         */
        @Override
        public void onAuthenticationFailed() {
            // Implementations in this method are optional, nothing related to Iris Solution is
            // required here
            postInfo("Failed to Authenticate!");
            mCurrentStatus = Status.STOPPED;
            updateViews();
            resetViews();
        }
    }

    /**
     * This method updates related views in the sample app, it's not related to the solution.
     */
    private void updateViews() {
        if (Status.STOPPED == mCurrentStatus || Status.NO_MATCH == mCurrentStatus) {
            mBtnVerify.setEnabled(true);
            mBtnVerify.setClickable(true);
            mBtnVerify.setVisibility(View.VISIBLE);
            mBtnCancelAuth.setVisibility(View.GONE);
            mBtnCancelAuth.getDrawable().setColorFilter(null);
        } else if (Status.STARTING == mCurrentStatus) {
            mBtnVerify.setVisibility(View.GONE);
            mBtnCancelAuth.setVisibility(View.VISIBLE);
            mBtnCancelAuth.setEnabled(false);
            mBtnCancelAuth.setClickable(false);
            mBtnCancelAuth.getDrawable().setColorFilter(Color.GRAY, PorterDuff.Mode.SRC_IN);
        } else if (Status.MODIFYING == mCurrentStatus) {
            mBtnVerify.setEnabled(false);
            mBtnVerify.setClickable(false);
        } else {
            mBtnVerify.setVisibility(View.GONE);
            mBtnCancelAuth.setVisibility(View.VISIBLE);
            mBtnCancelAuth.setEnabled(true);
            mBtnCancelAuth.setClickable(true);
            mBtnCancelAuth.getDrawable().setColorFilter(null);
        }
    }

    /**
     * It is recommended to implement Iris Solution related initialization in onResume()
     */
    @Override
    protected void onResume() {
        super.onResume();

        mTimerBar.setProgress(100, true);
        mToast.cancel();
        updateViews();
        postInfo("");

        mIrisManager = new IrisManager(this);

        // Get preview size from iris manager
        Size previewSize = mIrisManager.getPreviewSize();

        if (previewSize != null) {
            mWidth = previewSize.getWidth();
            mHeight = previewSize.getHeight();
            Log.v(TAG, "Preview size - height: " + mHeight + " width: " + mWidth);
        } else
            Log.d(TAG, "fail to get preview size");

        mFrameLayout = findViewById(R.id.surface_parent);

        mSurfaceView = findViewById(R.id.texture);
        mSurfaceView.setIrisManager(mIrisManager);

        //A surface holder, holding the SurfaceView
        SurfaceHolder mSurfaceHolder = mSurfaceView.getHolder();
        mSurfaceHolder.addCallback(IrisAuthenticationDemo.this);

        // Set the the camera preview's SurfaceHolder with the same layout params as the
        // IrisSurfaceView
        mSurfaceHolder.setFixedSize(mWidth, mHeight);
        mEnrollListAdapter.notifyDataSetChanged();
        refreshEnrollList();
        resetViews();
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

    /**
     * It is recommended to implement UI related initialization in onCreate method
     *
     * @param savedInstanceState passed down from Android system
     */
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Log.d(TAG, "onCreate");
        setContentView(R.layout.ipass_main);

        mTextViewInfo = findViewById(R.id.textViewInfo);
        mTimerBar = findViewById(R.id.timerbar);

        mCurrentStatus = Status.STOPPED;

        PowerManager mPowerManager = (PowerManager) getSystemService(POWER_SERVICE);
        mWakeLock = mPowerManager.newWakeLock(PowerManager.FULL_WAKE_LOCK, "IrisWakeLock");

        mBtnCancelAuth = findViewById(R.id.btnCancelAuth);
        mBtnCancelAuth.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                if (Status.STOPPED != mCurrentStatus && Status.STARTING != mCurrentStatus) {
                    mBtnCancelAuth.setEnabled(false);
                    handle_cancel();
                }
            }
        });

        mBtnVerify = findViewById(R.id.btn_continuous_verify);
        mBtnVerify.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                //cont'd verification
                if (Status.STOPPED == mCurrentStatus) {
                    mWakeLock.acquire();
                    mBtnVerify.setClickable(false);
                    mBtnVerify.setEnabled(false);
                    mCurrentStatus = Status.STARTING;
                    updateViews();
                    refreshEnrollList();
                    startAuthentication();
                }
            }
        });

        //Init toast, do not show
        mToast = Toast.makeText(IrisAuthenticationDemo.this, "", Toast.LENGTH_SHORT);
        mToast.setGravity(Gravity.TOP | Gravity.START, 0, 0);

        mEnrollListView = findViewById(R.id.enrolledList);
        mEnrollListAdapter = new ArrayAdapter<String>(IrisAuthenticationDemo.this, android.R.layout.simple_list_item_1, mEnrollList) {
            @Override
            public View getView(int position, View convertView, ViewGroup parent) {
                View view = super.getView(position, convertView, parent);
                TextView textview = view.findViewById(android.R.id.text1);
                textview.setTextAlignment(View.TEXT_ALIGNMENT_CENTER);
                textview.setGravity(Gravity.CENTER);
                return view;
            }
        };

        mEnrollListView.setAdapter(mEnrollListAdapter);
        mEnrollListView.setChoiceMode(ListView.CHOICE_MODE_SINGLE);
    }

    private void handle_cancel() {
        mWakeLock.release();
        if (mCurrentStatus == Status.STOPPED) {
            updateViews();
            return;
        }
        cancelVerification();
        resetViews();
    }

    /**
     * This methods starts the authentication
     */
    private void startAuthentication() {
        if (mCurrentStatus == Status.VERIFYING)
            return;

        if (!mIrisManager.hasEnrolledIris()) {
            postInfo("No irises enrolled");
            mCurrentStatus = Status.STOPPED;
            updateViews();
            return;
        }

        resetEnrollListView();
        postInfo("Starting...");

        mAuthenticationCallback authenticationResult = new mAuthenticationCallback();
        mCancelSignal = new CancellationSignal();

        // Whether to show camera preview or not
        boolean showPreview = true;

        // Enables the camera preview if we want to
        if (mSurface == null && showPreview) {
            // By setting the visibility to visible, surfaceCreated() will be called and mSurface
            // will have a reference (being not null)
            mSurfaceView.setVisibility(View.VISIBLE);
            if (mSurface == null) {
                Log.e(TAG, "surface is still null before setting it!");
            }
            // Pass the surface to iris manager if we want to see the preview
            mIrisManager.setPreviewSurface(mSurface);
        }

        //Start authentication
        mIrisManager.authenticate(null, mCancelSignal, 0, authenticationResult, null);
    }

    /**
     * This method posts info to the info TextView
     *
     * @param info information
     */
    private void postInfo(final String info) {
        mTextViewInfo.setText(info);
    }

    private void resetEnrollListView() {
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
    private void refreshEnrollList() {
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
            mEnrollList.add(irisName.toString());
        }
        mEnrollListAdapter.notifyDataSetChanged();
    }

    /**
     * This method cancels the on-going verification session
     */
    private void cancelVerification() {
        if (mCancelSignal != null) {
            mCancelSignal.cancel();
            if (!mCancelSignal.isCanceled()) {
                Log.e(TAG, "Cancel signal is NOT working!");
            }
        }
        mToast.cancel();
        postInfo("");
    }

    /**
     * This method resets the UI
     */
    private void resetViews() {
        mSurfaceView.setVisibility(View.INVISIBLE);
        mTimerBar.setProgress(100, true);

        mToast.cancel();
        mSurface = null;

        resetEnrollListView();
        updateViews();
    }
}
