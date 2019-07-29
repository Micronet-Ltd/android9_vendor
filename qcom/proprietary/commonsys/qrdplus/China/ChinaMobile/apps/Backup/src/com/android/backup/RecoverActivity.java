/*
 * Copyright (c) 2013 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 *
 * Not a Contribution, Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2008 The Android Open Source Project
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

package com.android.backup;

import android.app.ListActivity;
import android.os.Environment;
import android.os.storage.StorageManager;
import android.widget.AbsListView.MultiChoiceModeListener;
import android.widget.AbsListView;
import android.widget.AdapterView;
import android.widget.CheckBox;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.Spinner;
import android.widget.SpinnerAdapter;
import android.widget.TextView;
import android.widget.Toast;
import android.app.ActionBar;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.AppOpsManager;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.view.ActionMode;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.MenuInflater;
import android.view.ViewGroup;
import android.view.Window;
import android.view.View;
import android.view.ContextMenu;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ContextMenu.ContextMenuInfo;
import android.view.ViewGroup.LayoutParams;

import java.io.File;
import java.lang.reflect.Field;
import java.util.Vector;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.app.ProgressDialog;

import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.database.DataSetObserver;

import android.util.Log;
import android.util.SparseBooleanArray;

import static com.android.backup.HelpActivity.*;

import com.qualcomm.qti.backup_wrapper.AppOpsManagerWrapper;
import com.qualcomm.qti.backup_wrapper.ResourcesWrapper;
import com.qualcomm.qti.backup_wrapper.StorageManagerWrapper;
import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;
import com.qualcomm.qti.backup_wrapper.StorageEventListenerWrapper;

public class RecoverActivity extends ListActivity {

    private static final String TAG = "RecoverActivity";
    private static final boolean LOCAL_DEBUG = true;

    public static final String CURRENT_DIR_STR = "android.com.backup.directory";
    private static final String BACKUP_DATA_DIR = "/backup/Data/";

    public static final int DEFAULT_MODE = 0;
    public static final int PICKER_DELETE_MODE = 1;

    private static final int EVENT_DELETE_START = 100;
    private static final int EVENT_DELETE_END = 101;
    private static final int EVENT_DELETE_PER = 102;

    private String mRecoverPath;
    private String mBackupRootDir;
    private ListView mListView;
    private RecoverListAdapter mOnlyAdapter;

    private ProgressDialog mDelProgDlg = null;
    private Handler mDelHandler;
    private CheckBox mCheckBox;

    private static final String PREFERENCE_RECOVER_ACTIVITY = "Preference_Recover_Activity";
    private static final String PREFERENCE_RECOVER_SHOW_DIALOG_KEY = "Show_Dialog_Key";

    private boolean mMultiDelete = false;

    private StorageManager mStorageManager;
    private StorageEventListenerWrapper mStorageListener = new StorageEventListenerWrapper() {
        @Override
        public void onStorageStateChanged(String path, String oldState, String newState) {
            Log.d(TAG, "onStorageStateChanged(), path: " + path +
                    ", oldState: " + oldState + ", newState: " + newState);
            if (!(path != null && path.equals(BackupUtils.getSDPath(getApplicationContext())))) {
                return;
            }
            if (newState.equals(Environment.MEDIA_BAD_REMOVAL) ||
                    newState.equals(Environment.MEDIA_MOUNTED) ||
                    newState.equals(Environment.MEDIA_UNMOUNTED)) {
                finish();
            }
        }
    };

    private class DeleteHandler extends Handler {

        public void handleMessage(Message msg) {
            switch (msg.what) {
                case EVENT_DELETE_START: {
                    Log.d(TAG, "EVENT_DELETE_START");
                    mDelProgDlg = new ProgressDialog(RecoverActivity.this,
                            ResourcesWrapper.style.Theme_Holo_Dialog_Alert);
                    mDelProgDlg.setTitle(R.string.delete_title);
                    mDelProgDlg.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
                    mDelProgDlg.setCancelable(false);
                    mDelProgDlg.setCanceledOnTouchOutside(false);
                    mDelProgDlg.setMax(msg.arg1);
                    mDelProgDlg.setProgress(0);
                    mDelProgDlg.show();
                    break;
                }
                case EVENT_DELETE_END: {
                    Log.d(TAG, "EVENT_DELETE_END");
                    if (mDelProgDlg != null) {
                        mDelProgDlg.dismiss();
                        mDelProgDlg = null;
                    }
                    if (msg.arg1 == 1) {
                        Toast.makeText(RecoverActivity.this, R.string.delete_ok,
                                Toast.LENGTH_SHORT).show();
                    } else {
                        Toast.makeText(RecoverActivity.this, R.string.delete_failed,
                                Toast.LENGTH_SHORT).show();
                    }
                    mOnlyAdapter.sortImpl();
                    break;
                }
                case EVENT_DELETE_PER: {
                    Log.d(TAG, "EVENT_DELETE_PER");
                    if (mDelProgDlg != null) {
                        mDelProgDlg.setProgress(msg.arg1);
                    }
                }
                default:
                    Log.i(TAG, "unknow message: " + msg.what);
                    return;
            }
        }
    }

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);
        mRecoverPath = BackupUtils.getRestorePath(this);
        mBackupRootDir = mRecoverPath + BACKUP_DATA_DIR;

        mListView = getListView();
        mDelHandler = new DeleteHandler();
        getListView().setChoiceMode(ListView.CHOICE_MODE_MULTIPLE_MODAL);
        getListView().setMultiChoiceModeListener(new MultiActionDealer());
        mOnlyAdapter = new RecoverListAdapter(this, mDelHandler, mRecoverPath);
        mOnlyAdapter.sortImpl();
        setListAdapter(mOnlyAdapter);
        getListView().setOnCreateContextMenuListener(this);
        ActionBar actionBar = getActionBar();
        actionBar.setDisplayHomeAsUpEnabled(true);
        if (mListView.getCount() == 0) {
            final SharedPreferences sharedPreferences =
                    getSharedPreferences(PREFERENCE_RECOVER_ACTIVITY, Context.MODE_PRIVATE);
            if (sharedPreferences.getBoolean(PREFERENCE_RECOVER_SHOW_DIALOG_KEY, true)) {
                LayoutInflater layoutInflater = LayoutInflater.from(this);
                View dialogView = layoutInflater.inflate(R.layout.dialog_style, null);
                mCheckBox = (CheckBox) dialogView.findViewById(R.id.checkbox);
                new AlertDialog.Builder(RecoverActivity.this,
                        android.R.style.Theme_DeviceDefault_Dialog_NoActionBar)
                        .setMessage(R.string.no_sdcard_restore_abstract)
                        .setView(dialogView)
                        .setNeutralButton(R.string.button_help, new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which) {

                                Intent intent = new Intent();
                                intent.setClass(RecoverActivity.this, HelpActivity.class);
                                intent.putExtra(HELP_TYPE_KEY, TYPE_RESTORE);
                                startActivity(intent);
                            }
                        })
                        .setNegativeButton(R.string.button_cancel, new DialogInterface.OnClickListener() {

                            @Override
                            public void onClick(DialogInterface dialog, int which) {

                                if (mCheckBox.isChecked()) {
                                    SharedPreferences.Editor editor = sharedPreferences.edit();
                                    editor.putBoolean(PREFERENCE_RECOVER_SHOW_DIALOG_KEY, false).commit();
                                }
                            }
                        }).create().show();
            }
        }

        PackageManager packageManager = getPackageManager();
        AppOpsManager appOps = (AppOpsManager) getSystemService(Context.APP_OPS_SERVICE);
        AppOpsManagerWrapper appOpsWrapper = new AppOpsManagerWrapper(appOps);
        String packageName = getPackageName();
        try {
            PackageInfo info = packageManager.getPackageInfo(packageName, 0);
            try {
                int mode = appOpsWrapper.checkOp(AppOpsManagerWrapper.OP_WRITE_SMS,
                        info.applicationInfo.uid, packageName);
                Log.v(TAG, packageName + " mode: " + mode);
                if (mode != AppOpsManager.MODE_ALLOWED) {
                    Log.v(TAG, packageName + " lost OP_WRITE_SMS:  (fixing)");
                    appOpsWrapper.setMode(AppOpsManagerWrapper.OP_WRITE_SMS,
                            info.applicationInfo.uid, packageName, AppOpsManager.MODE_ALLOWED);
                }
            } catch (WrapperNotSupportException wrapperE) {
                Log.e(TAG, wrapperE.getMessage());
            }
        } catch (NameNotFoundException e) {
            Log.v(TAG, packageName + "  package not found");
        }

        try {
            mStorageManager = StorageManagerWrapper.from(this);
            mStorageListener.registerListener(mStorageManager);
        } catch (WrapperNotSupportException wrapperE) {
            Log.e(TAG, wrapperE.getMessage());
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if (mStorageManager != null && mStorageListener != null) {
            try {
                mStorageListener.unregisterListener(mStorageManager);
            } catch (WrapperNotSupportException wrapperE) {
                Log.e(TAG, wrapperE.getMessage());
            }
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        mOnlyAdapter.mMode = DEFAULT_MODE;
    }

    @Override
    public boolean onKeyDown(int keyCode, KeyEvent event) {

        switch (keyCode) {
            case KeyEvent.KEYCODE_BACK:
                if (mOnlyAdapter.handleBackKeyAction(keyCode, event) == true)
                    return true;
                break;

            default:
                break;
        }

        return super.onKeyDown(keyCode, event);
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        super.onListItemClick(l, v, position, id);

        if (LOCAL_DEBUG)
            Log.d(TAG, "onListItemClick v:" + v + " position: " + position + " id: " + id);

        if (DEFAULT_MODE == mOnlyAdapter.getShowMode()) {
            String fileName = (String) mOnlyAdapter.getItem(position);
            Log.d(TAG, "onListItemClick fileName: " + fileName);
            Log.d(TAG, "onContextItemSelected CONTEXT_MENU_ID_RESTORE");
            Intent intent = new Intent(RecoverActivity.this, RecoverOperateActivity.class);
            intent.putExtra(CURRENT_DIR_STR, fileName);
            startActivity(intent);
        } else {
            Log.d(TAG, "onListItemClick multi-selected mode");
            mOnlyAdapter.notifyDataSetChanged();
        }
    }

    public boolean getMultiDelFlag() {
        return mMultiDelete;
    }

    public boolean onPrepareOptionsMenu(Menu menu) {
        if (mOnlyAdapter.mMode != DEFAULT_MODE) {
            mOnlyAdapter.onPrepareOptionsMenu(this, menu);
        }
        return super.onPrepareOptionsMenu(menu);
    }

    private class MultiActionDealer implements ListView.MultiChoiceModeListener,
            Spinner.OnItemSelectedListener, SpinnerAdapter {
        private Spinner mSpin = null;
        private ViewGroup mView = null;
        private View mTitleView;
        private View mDropView;
        private TextView mTextView;
        private TextView mTitleTextView;
        private boolean mSeleting = false;

        public MultiActionDealer() {
            mTitleView = null;
            mDropView = null;
        }

        private void prepareObj() {
            if (mView == null) {
                mView = (ViewGroup) LayoutInflater.from(RecoverActivity.this)
                        .inflate(R.layout.list_multi_select_actionmode, null);
                mSpin = ((Spinner) mView.findViewById(R.id.select_actions));
                mSpin.setAdapter(this);
                mSpin.setEnabled(true);
                mSpin.setOnItemSelectedListener(this);
                mSpin.setSelected(true);
                mSpin.setSelection(2);
            }
        }

        @Override
        public boolean onCreateActionMode(ActionMode mode, Menu menu) {
            Log.d(TAG, "onCreateActionMode");
            MenuInflater inflater = getMenuInflater();
            inflater.inflate(R.menu.list_single_select, menu);
            prepareObj();
            mode.setCustomView(mView);
            mListView.invalidateViews();
            mSeleting = true;
            return true;
        }

        @Override
        public boolean onPrepareActionMode(ActionMode mode, Menu menu) {
            prepareObj();
            mode.setCustomView(mView);
            return true;
        }

        @Override
        public int getCount() {
            return 1;
        }

        @Override
        public Object getItem(int position) {
            return getSelectTitle();
        }

        @Override
        public long getItemId(int position) {
            return position;
        }

        @Override
        public int getItemViewType(int position) {
            return 0;
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (mTitleView == null) {
                mTitleView = new LinearLayout(RecoverActivity.this);
                ((LinearLayout) mTitleView).setOrientation(LinearLayout.VERTICAL);
                mTitleTextView = new TextView(RecoverActivity.this);
                LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                        LayoutParams.FILL_PARENT, LayoutParams.WRAP_CONTENT);
                mTitleTextView.setLayoutParams(params);
                ((LinearLayout) mTitleView).addView(mTitleTextView);
                mTitleView.setLayoutParams(new LinearLayout.LayoutParams(LayoutParams.FILL_PARENT,
                        LayoutParams.FILL_PARENT));
            }
            mTitleTextView.setText(getSelectTitle());
            if (mListView.getCheckedItemCount() > 0) {
                for (int mPosition = 0; mPosition < mListView.getCount(); mPosition++) {
                    setListViewBk(true, mPosition);
                }
            }
            return mTitleView;
        }

        @Override
        public int getViewTypeCount() {
            return 1;
        }

        @Override
        public boolean hasStableIds() {
            return false;
        }

        @Override
        public boolean isEmpty() {
            return false;
        }

        @Override
        public void registerDataSetObserver(DataSetObserver observer) {
            mSpin.setSelection(2);
        }

        @Override
        public void unregisterDataSetObserver(DataSetObserver observer) {
            long i = mSpin.getSelectedItemId();
            if (i == 0) {
                boolean selected = !isAllSelected();
                int all = mListView.getCount();
                for (int j = 0; j < all; ++j) {
                    mListView.setItemChecked(j, selected);
                }
                mListView.refreshDrawableState();
            }
        }

        @Override
        public View getDropDownView(int position, View convertView, ViewGroup parent) {
            if (mDropView == null) {
                mDropView = new LinearLayout(RecoverActivity.this);
                ((LinearLayout) mDropView).setOrientation(LinearLayout.HORIZONTAL);
                mTextView = new TextView(RecoverActivity.this);
                LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                        LayoutParams.FILL_PARENT, LayoutParams.WRAP_CONTENT);
                params.setMargins(20, 0, 0, 0);
                params.gravity = Gravity.CENTER_VERTICAL;
                mTextView.setLayoutParams(params);
                ((LinearLayout) mDropView).addView(mTextView);
                mDropView.setLayoutParams(new LinearLayout.LayoutParams(LayoutParams.MATCH_PARENT,
                        LayoutParams.MATCH_PARENT));
            }
            if (mTitleView != null)
                mDropView.setLayoutParams(new AbsListView.LayoutParams(mTitleView.getWidth() * 2,
                        mTitleView.getHeight() * 2));
            if (isAllSelected())
                mTextView.setText(R.string.menu_deselect_all);
            else
                mTextView.setText(R.string.menu_select_all);
            return mDropView;
        }

        private String getSelectTitle() {
            int listCnt = mListView.getCheckedItemCount();
            String title = RecoverActivity.this.getString(R.string.menu_delete_sel);
            return "" + listCnt + " " + title;
        }

        private boolean isAllSelected() {
            int listSelCnt = mListView.getCheckedItemCount();
            int listCnt = mListView.getChildCount();
            if (listSelCnt == listCnt)
                return true;
            else
                return false;
        }

        public boolean onActionItemClicked(ActionMode mode, MenuItem item) {

            switch (item.getItemId()) {
                case R.id.delete: {
                    Vector v = new Vector();
                    for (int i = 0; i < mListView.getCount(); i++) {
                        if (mOnlyAdapter.isCheckItems(i))
                            v.add((String) mOnlyAdapter.getItem(i));
                    }
                    final Vector sels = v;
                    final ActionMode m = mode;
                    AlertDialog alertdiag = new AlertDialog.Builder(RecoverActivity.this,
                            android.R.style.Theme_DeviceDefault_Dialog_NoActionBar)
                            .setTitle(R.string.delete_confirm_alert_title)
                            .setMessage((sels.size() == 1) ? R.string.delete_confirm_alert_msg1 :
                                    R.string.delete_confirm_alert_msg)
                            .setNegativeButton(R.string.button_cancel, null)
                            .setPositiveButton(R.string.button_ok, new OnClickListener() {
                                public void onClick(DialogInterface dlg, int which) {
                                    Log.d(TAG, "onClick1");
                                    Message message = mDelHandler
                                            .obtainMessage(EVENT_DELETE_START);
                                    message.arg1 = sels.size();
                                    mDelHandler.sendMessage(message);
                                    new Thread() {
                                        public void run() {
                                            Message message;
                                            int ret = 0;
                                            try {
                                                for (int i = 0; i < sels.size(); ++i) {
                                                    File tmpdir = new File(
                                                            mBackupRootDir
                                                                    + sels.get(i));
                                                    BackupUtils.deleteDir(tmpdir);
                                                    Thread.sleep(300);
                                                    message = mDelHandler
                                                            .obtainMessage(EVENT_DELETE_PER);
                                                    message.arg1 = i;
                                                    mDelHandler.sendMessage(message);
                                                }
                                                message = mDelHandler
                                                        .obtainMessage(EVENT_DELETE_PER);
                                                message.arg1 = sels.size();
                                                mDelHandler.sendMessage(message);
                                                ret = 1;
                                            } catch (Exception e) {
                                                Log.e(TAG, " " + e);
                                            } finally {
                                                message = mDelHandler
                                                        .obtainMessage(EVENT_DELETE_END);
                                                message.arg1 = ret;
                                                mDelHandler.sendMessage(message);
                                            }
                                        }
                                    }.start();
                                    m.finish();
                                }
                            }).create();
                    alertdiag.show();
                    break;
                }
                case R.id.recovery_item: {
                    if (DEFAULT_MODE == mOnlyAdapter.getShowMode()) {
                        SparseBooleanArray arr = mListView.getCheckedItemPositions();
                        for (int i = 0; i < mListView.getCount(); i++) {
                            if (arr.get(i)) {
                                String fileName = (String) mOnlyAdapter.getItem(i);
                                Log.d(TAG, "onListItemClick fileName: " + fileName);
                                Log.d(TAG, "onContextItemSelected CONTEXT_MENU_ID_RESTORE");
                                Intent intent = new Intent(RecoverActivity.this,
                                        RecoverOperateActivity.class);
                                intent.putExtra(CURRENT_DIR_STR, fileName);
                                startActivity(intent);
                                mode.finish();
                                break;
                            }
                        }
                    } else {
                        Log.d(TAG, "onListItemClick multi-selected mode");
                        mOnlyAdapter.notifyDataSetChanged();
                        mode.finish();
                    }
                }
                default:
                    mode.finish();
                    break;
            }
            return true;
        }

        public void onDestroyActionMode(ActionMode mode) {
            mListView.clearChoices();
            mListView.invalidateViews();
            mSeleting = false;
            setListViewBk(mSeleting, 1);
            try {
                Field field = mListView.getClass().getSuperclass()
                        .getDeclaredField("mChoiceActionMode");
                field.setAccessible(true);
                field.set(mListView, null);
            } catch (Exception ignored) {
            }
        }

        public void onItemCheckedStateChanged(ActionMode mode, int position,
                                              long id, boolean checked) {
            prepareObj();
            if (mTitleView != null) {
                mTitleTextView.setText(getSelectTitle());
                mTitleTextView.refreshDrawableState();
                mTitleTextView.invalidate();
            }
            mSpin.invalidate();
            mode.setTitle(getSelectTitle());
            setListViewBk(mSeleting, position);

            mode.getMenu().clear();
            if (mListView.getCheckedItemCount() > 0) {
                MenuInflater inflater = getMenuInflater();
                inflater.inflate(R.menu.list_single_select, mode.getMenu());
            } else {
                MenuInflater inflater = getMenuInflater();
                inflater.inflate(R.menu.list_multi_select, mode.getMenu());
            }
        }

        @Override
        public void onItemSelected(AdapterView<?> arg0, android.view.View arg1,
                                   int arg2, long arg3) {
        }

        @Override
        public void onNothingSelected(AdapterView<?> arg0) {
        }

        private void setListViewBk(boolean state, int position) {
            if (state == false) {
                int count = mListView.getCount();
                for (int i = 0; i < count; i++) {
                    ((RecoverListAdapter) (mListView.getAdapter())).selectItems(i, false);
                }
                Log.d(TAG, "SetListViewBk state:false");
            } else {
                if (mListView.isItemChecked(position)) {
                    ((RecoverListAdapter) (mListView.getAdapter())).selectItems(position, true);
                } else {
                    ((RecoverListAdapter) (mListView.getAdapter())).selectItems(position, false);
                }
            }
        }
    }
}
