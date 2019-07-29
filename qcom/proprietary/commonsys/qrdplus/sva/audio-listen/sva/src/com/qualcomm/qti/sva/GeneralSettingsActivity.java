/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.app.Activity;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.Spinner;
import android.widget.Switch;

import com.qualcomm.qti.sva.data.ISettingsModel;
import com.qualcomm.qti.sva.data.SettingsModel;
import com.qualcomm.qti.sva.utils.LogUtils;

public class GeneralSettingsActivity extends Activity {
    private final String TAG = GeneralSettingsActivity.class.getSimpleName();

    // view variable define
    private EditText mSm3EtGMMKeyphrase;
    private EditText mSm3EtGMMUser;
    private EditText mSm3EtVOPUser;
    private EditText mSm3EtCNNKeyphrase;
    private EditText mSm2EtGMMKeyphrase;
    private EditText mSm2EtGMMUser;
    private EditText mEtGMMTraining;
    private RelativeLayout mLayoutDetectionTone;
    private Switch mSwitchDetectionTone;
    private RelativeLayout mLayoutAdvancedDetails;
    private Switch mSwitchAdvancedDetails;
    private ISettingsModel mSettingModel;

    private static final int MAX_GMM_TRAINNING_LEVEL = 100;
    private static final int MIN_GMM_TRAINNING_LEVEL = 0;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        mSettingModel = new SettingsModel(getApplicationContext(), null);
        setContentView(R.layout.activity_general_settings);
        initializeUI();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    private void initializeUI() {
        //sound model 3.0 global settings
        mSm3EtGMMKeyphrase = findViewById(R.id.sm3_edit_gmm_keyphrase);
        mSm3EtGMMKeyphrase.setText(String.valueOf(
                mSettingModel.getGlobalSM3GMMKeyphraseConfidenceLevel()));
        mSm3EtGMMKeyphrase.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.setGlobalSM3GMMKeyphraseConfidenceLevel(
                            Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        mSm3EtCNNKeyphrase = findViewById(R.id.sm3_edit_cnn_keyphrase);
        mSm3EtCNNKeyphrase.setText(String.valueOf(
                mSettingModel.getGlobalSM3CNNKeyphraseConfidenceLevel()));
        mSm3EtCNNKeyphrase.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.setGlobalSM3CNNKeyphraseConfidenceLevel(
                            Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        mSm3EtGMMUser = findViewById(R.id.sm3_edit_gmm_user);
        mSm3EtGMMUser.setText(String.valueOf(
                mSettingModel.getGlobalSM3GMMUserConfidenceLevel()));
        mSm3EtGMMUser.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.setGlobalSM3GMMUserConfidenceLevel(
                            Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        mSm3EtVOPUser = findViewById(R.id.sm3_edit_vop_user);
        mSm3EtVOPUser.setText(String.valueOf(mSettingModel.getGlobalSM3VOPUserConfidenceLevel()));
        mSm3EtVOPUser.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.setGlobalSM3VOPUserConfidenceLevel(
                            Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        //sound model 2.0 global settings
        mSm2EtGMMKeyphrase = findViewById(R.id.sm2_edit_gmm_keyphrase);
        mSm2EtGMMKeyphrase.setText(String.valueOf(
                mSettingModel.getGlobalSM2GMMKeyphraseConfidenceLevel()));
        mSm2EtGMMKeyphrase.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.setGlobalSM2GMMKeyphraseConfidenceLevel(
                            Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        mSm2EtGMMUser = findViewById(R.id.sm2_edit_gmm_user);
        mSm2EtGMMUser.setText(String.valueOf(
                mSettingModel.getGlobalSM2GMMUserConfidenceLevel()));
        mSm2EtGMMUser.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    mSettingModel.setGlobalSM2GMMUserConfidenceLevel(
                            Integer.valueOf(s.toString()));
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        //global gmm training settings.
        mEtGMMTraining = findViewById(R.id.edit_gmm_training);
        mEtGMMTraining.setText(String.valueOf(mSettingModel.getGlobalGMMTrainingConfidenceLevel()));
        mEtGMMTraining.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence s, int start, int count, int after) {
            }

            @Override
            public void onTextChanged(CharSequence s, int start, int before, int count) {
            }

            @Override
            public void afterTextChanged(Editable s) {
                try {
                    String editString = s.toString();
                    int level = Integer.valueOf(editString);
                    if (level > MAX_GMM_TRAINNING_LEVEL || level < MIN_GMM_TRAINNING_LEVEL
                            || !String.valueOf(level).equals(editString)) {
                        if (level > MAX_GMM_TRAINNING_LEVEL) {
                            level = MAX_GMM_TRAINNING_LEVEL;
                        } else if (level < MIN_GMM_TRAINNING_LEVEL){
                            level = MIN_GMM_TRAINNING_LEVEL;
                        }
                        mEtGMMTraining.setText(String.valueOf(level));
                    }
                    mSettingModel.setGlobalGMMTrainingConfidenceLevel(level);
                } catch (NumberFormatException e) {
                    e.printStackTrace();
                }
            }
        });

        mSwitchDetectionTone = findViewById(R.id.switch_detection_tone);
        mSwitchDetectionTone.setChecked(mSettingModel.getGlobalDetectionToneEnabled());
        mLayoutDetectionTone = findViewById(R.id.layout_detection_tone);
        mLayoutDetectionTone.setTag(mSwitchDetectionTone);
        mLayoutDetectionTone.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Switch switchDetectionTone = (Switch) view.getTag();
                boolean bChecked = !switchDetectionTone.isChecked();
                switchDetectionTone.setChecked(bChecked);
                mSettingModel.setGlobalDetectionToneEnabled(bChecked);
                LogUtils.d(TAG, "onClick: detection tone bChecked = " + bChecked);
            }
        });

        mSwitchAdvancedDetails = findViewById(R.id.switch_advanced_details);
        mSwitchAdvancedDetails.setChecked(mSettingModel.getGlobalIsDisplayAdvancedDetails());
        mLayoutAdvancedDetails = findViewById(R.id.layout_advanced_details);
        mLayoutAdvancedDetails.setTag(mSwitchAdvancedDetails);
        mLayoutAdvancedDetails.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Switch switchAdvancedDetails = (Switch) view.getTag();
                boolean bChecked = !switchAdvancedDetails.isChecked();
                switchAdvancedDetails.setChecked(bChecked);
                mSettingModel.setGlobalIsDisplayAdvancedDetails(bChecked);
                LogUtils.d(TAG, "onClick: advanced details bChecked = " + bChecked);
            }
        });
    }
}
