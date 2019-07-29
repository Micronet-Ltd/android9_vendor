/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.content.ContentResolver;
import android.net.Uri;
import android.util.Log;
import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class ContentResolverWrapper {

    private static final String TAG = "ContentResolverWrapper";
    private ContentResolver mContentResolver = null;

    public ContentResolverWrapper(ContentResolver instance){
        mContentResolver = instance;
    }

    public void setInstance(ContentResolver instance){
        mContentResolver = instance;
    }

    /**
     * Returns the content provider for the given content URI.
     *
     * @param uri The URI to a content provider
     * @return The ContentProvider for the given URI, or null if no content provider is found.
     */
    public final IContentProviderWrapper acquireProvider(Uri uri) throws WrapperNotSupportException {
        try{
            if(mContentResolver != null){
                return new IContentProviderWrapper(mContentResolver.acquireProvider(uri));
            }else{
                throw new WrapperNotSupportException(TAG + " instance is null!");
            }
        } catch (Exception e) {
            Log.e(TAG,"acquireProvider: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }

    public boolean releaseProvider(IContentProviderWrapper icp) throws WrapperNotSupportException {
        try{
            if(mContentResolver != null){
                return mContentResolver.releaseProvider(icp.getInstance());
            }else{
                throw new WrapperNotSupportException(TAG + " instance is null!");
            }
        } catch (Exception e) {
            Log.e(TAG,"acquireProvider: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }
}
