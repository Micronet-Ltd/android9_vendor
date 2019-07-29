/*
 * Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

package com.android.backup;

import java.io.File;

import java.util.List;
import java.util.ArrayList;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.ActivityNotFoundException;
import android.content.ContentResolver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.preference.ListPreference;
import android.preference.Preference;
import android.preference.PreferenceActivity;
import android.preference.PreferenceManager;
import android.preference.PreferenceScreen;
import android.preference.Preference.OnPreferenceClickListener;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.Toast;

import static com.android.backup.HelpActivity.*;
import static com.android.backup.BackupUtils.*;

import com.qualcomm.qti.backup_wrapper.StorageManagerWrapper;
import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;
import com.qualcomm.qti.backup_wrapper.StorageEventListenerWrapper;

public class SettingsActivity extends PreferenceActivity implements
        Preference.OnPreferenceClickListener, Preference.OnPreferenceChangeListener {

    private static String TAG = "SettingsActivity";
    private static final String KEY_BACKUP = "backup_location";
    private static final String KEY_RESTORE = "restore_location";
    private static final String KEY_HELP = "settings_help";

    private ListPreference mBackupPreference;
    private ListPreference mRestorePreference;
    private Preference mHelpPreference;

    private SharedPreferences sharedPreferences;
    private StorageEventListenerWrapper mStorageListener;
    private StorageManager mStorageManager;

    private List<String> mStorageLocations = new ArrayList<String>();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        final ContentResolver resolver = this.getContentResolver();
        addPreferencesFromResource(R.xml.settings_preference);
        sharedPreferences = getSharedPreferences(BackupUtils.LOCATION_PREFERENCE_NAME,
              Context.MODE_PRIVATE);

        mBackupPreference = (ListPreference) findPreference(KEY_BACKUP);
        mRestorePreference = (ListPreference) findPreference(KEY_RESTORE);
        mHelpPreference = (Preference) findPreference(KEY_HELP);

        mBackupPreference.setOnPreferenceChangeListener(this);
        mRestorePreference.setOnPreferenceChangeListener(this);
        mRestorePreference.setOnPreferenceClickListener(this);
        mHelpPreference.setOnPreferenceClickListener(this);

        updateSummary();
        updateListPreference();
        checkPathAndWarn();

        mStorageListener = new StorageEventListenerWrapper() {
            @Override
            public void onStorageStateChanged(String path, String oldState, String newState) {
                //update backup and restore status for the pathes in settings
                Log.d(TAG, "onStorageStateChanged(), path: " + path +
                        ", oldState: " + oldState + ", newState: " + newState);
                updateListPreference();
                if (!(path != null && path.equals(BackupUtils.getSDPath(getApplicationContext())))) {
                    return;
                }
                if (newState.equalsIgnoreCase(Environment.MEDIA_MOUNTED) ||
                        newState.equals(Environment.MEDIA_BAD_REMOVAL) ||
                        newState.equals(Environment.MEDIA_UNMOUNTED)) {
                    updateSummary();

                }
            }
        };
        try {
            Log.d(TAG, "registerListener");
            mStorageManager = StorageManagerWrapper.from(SettingsActivity.this);
            mStorageListener.registerListener(mStorageManager);
            //mStorageManager.registerListener(mStorageListener);
        } catch (WrapperNotSupportException wrapperE) {
            Log.e(TAG, wrapperE.getMessage());
        }
    }

    private void updateListPreference(){
        mStorageLocations.clear();
        File[] dirs = getApplicationContext().getExternalFilesDirs(null);
        for(File file: dirs){
            if (file != null && BackupUtils.isDirAvalible(file.getAbsolutePath())) {
                mStorageLocations.add(file.getAbsolutePath());
                Log.e("wangyingren", "updateListPreference file : " + file.getAbsolutePath());
            }
        }
        if (mStorageLocations.size() == 1){
            mBackupPreference.setEntries(getResources().
                    getStringArray(R.array.InternalStorageLocation));
            mBackupPreference.setEntryValues(getResources().
                    getStringArray(R.array.InternalStorageLocation_value));
            mRestorePreference.setEntries(getResources().
                    getStringArray(R.array.InternalStorageLocation));
            mRestorePreference.setEntryValues(getResources().
                    getStringArray(R.array.InternalStorageLocation_value));
            mBackupPreference.setValue(getResources().
                    getString(R.string.storage_location_internal_value));
            mRestorePreference.setValue(getResources().
                    getString(R.string.storage_location_internal_value));
            updateStorageLocationAndSummary(mBackupPreference,mBackupPreference.getValue(),
                    BackupUtils.KEY_BACKUP_LOCATION);
            updateStorageLocationAndSummary(mRestorePreference,mRestorePreference.getValue(),
                    BackupUtils.KEY_RESTORE_LOCATION);
        }else if (mStorageLocations.size() >= 2){
            mBackupPreference.setEntries(getResources().
                    getStringArray(R.array.StorageLocation));
            mBackupPreference.setEntryValues(getResources().
                    getStringArray(R.array.StorageLocation_value));
            mRestorePreference.setEntries(getResources().
                    getStringArray(R.array.StorageLocation));
            mRestorePreference.setEntryValues(getResources().
                    getStringArray(R.array.StorageLocation_value));
        }else {
            Log.e(TAG," this device has no external storage.");
        }

        if (mBackupPreference != null && mBackupPreference.getDialog() != null){
            mBackupPreference.getDialog().dismiss();
        }
        if (mRestorePreference != null && mRestorePreference.getDialog() != null){
            mRestorePreference.getDialog().dismiss();
        }
    }

    private void updateSummary() {
        String backUpPath = BackupUtils.getBackupPath(this);
        mBackupPreference.setSummary(backUpPath);

        String restorePath = BackupUtils.getRestorePath(this);
        mRestorePreference.setSummary(restorePath);
    }

    private void checkPathAndWarn(){
        String backUpPath = BackupUtils.getBackupPath(this);
        String restorePath = BackupUtils.getRestorePath(this);
        if (!BackupUtils.isDirAvalible(backUpPath)){
            Toast.makeText(SettingsActivity.this,
                getString(R.string.backup_path_dont_exist,
                    backUpPath)
                , Toast.LENGTH_SHORT).show();
        }
        if (!BackupUtils.isDirAvalible(restorePath)){
            Toast.makeText(SettingsActivity.this,
                getString(R.string.restore_path_dont_exist,
                    restorePath)
                , Toast.LENGTH_SHORT).show();

        }
    }

    @Override
    public void onDestroy() {
        if (mStorageManager != null && mStorageListener != null) {
            try {
                Log.d(TAG, "unregisterListener");
                mStorageListener.unregisterListener(mStorageManager);
                //mStorageManager.unregisterListener(mStorageListener);
            } catch (WrapperNotSupportException wrapperE) {
                Log.e(TAG, wrapperE.getMessage());
            }
        }
        super.onDestroy();
    }

    @Override
    public boolean onPreferenceClick(Preference preference) {
        if (preference.getKey().equals(KEY_HELP)) {
            Intent intent = new Intent();
            intent.setClass(SettingsActivity.this, HelpActivity.class);
            intent.putExtra(HELP_TYPE_KEY, TYPE_SETTING);
            startActivity(intent);
            return true;
        }
        return false;
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object objValue) {
        if (preference == mBackupPreference){
            updateStorageLocationAndSummary(preference, objValue, BackupUtils.KEY_BACKUP_LOCATION);
        }else if(preference == mRestorePreference){
            updateStorageLocationAndSummary(preference, objValue, BackupUtils.KEY_RESTORE_LOCATION);
        }
        return true;
    }

    private void updateStorageLocationAndSummary(Preference preference,
                 Object listPreferenceValue, String sharedPreferencesKey){
        String path = "";
        if(listPreferenceValue.equals(getResources().
                getString(R.string.storage_location_internal_value))
                && mStorageLocations.size() > 0){
            path = mStorageLocations.get(0);
        }else if(listPreferenceValue.equals(getResources().
                getString(R.string.storage_location_external_value))
                && mStorageLocations.size() > 1){
            path = mStorageLocations.get(1);
        }else {
            Log.e(TAG,"It's an exception condition. Value = "+ listPreferenceValue);
            return ;
        }
        SharedPreferences.Editor editor = sharedPreferences.edit();
        editor.putString(sharedPreferencesKey, path).commit();
        preference.setSummary(path);
    }
}
