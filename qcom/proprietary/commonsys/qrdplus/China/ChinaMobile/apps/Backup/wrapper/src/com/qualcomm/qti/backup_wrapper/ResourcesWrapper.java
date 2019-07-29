/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

import android.content.res.CompatibilityInfo;
import android.content.res.Resources;
import android.util.Log;

import com.qualcomm.qti.backup_wrapper.CompatibilityInfoWrapper;
import com.qualcomm.qti.backup_wrapper.WrapperNotSupportException;

public class ResourcesWrapper {

    private static final String TAG = "ResourceWrapper";

    Resources mResource = null;

    public ResourcesWrapper (Resources instance) {
        mResource = instance;
    }

    public CompatibilityInfoWrapper getCompatibilityInfo() throws WrapperNotSupportException{  
        try{
            if(mResource != null){
                return new CompatibilityInfoWrapper(mResource.getCompatibilityInfo());
            }else{
                throw new WrapperNotSupportException(TAG + " instance is null!");
            }
        } catch (Exception e) {
            Log.e(TAG,"getCompatibilityInfo: catch an exception ", e);
            throw new WrapperNotSupportException(TAG + " Not support!", e);
        }
    }

    public static final class style {
        public static final int Theme_Dialog_Alert = com.android.internal.R.style.Theme_Dialog_Alert;
        public static final int Theme_Holo_Dialog_Alert = com.android.internal.R.style.Theme_Holo_Dialog_Alert;
    }

    public static final class string {
        public static final int sipAddressTypeCustom = com.android.internal.R.string.sipAddressTypeCustom;
        public static final int phoneTypeHome = com.android.internal.R.string.phoneTypeHome;
        public static final int phoneTypeMobile = com.android.internal.R.string.phoneTypeMobile;
        public static final int phoneTypeWork = com.android.internal.R.string.phoneTypeWork;
        public static final int phoneTypeFaxWork = com.android.internal.R.string.phoneTypeFaxWork;
        public static final int phoneTypeFaxHome = com.android.internal.R.string.phoneTypeFaxHome;
        public static final int phoneTypePager = com.android.internal.R.string.phoneTypePager;
        public static final int phoneTypeOther = com.android.internal.R.string.phoneTypeOther;
        public static final int phoneTypeCallback = com.android.internal.R.string.phoneTypeCallback;
        public static final int phoneTypeCar = com.android.internal.R.string.phoneTypeCar;
        public static final int phoneTypeCompanyMain = com.android.internal.R.string.phoneTypeCompanyMain;
        public static final int phoneTypeIsdn = com.android.internal.R.string.phoneTypeIsdn;
        public static final int phoneTypeMain = com.android.internal.R.string.phoneTypeMain;
        public static final int phoneTypeOtherFax = com.android.internal.R.string.phoneTypeOtherFax;
        public static final int phoneTypeRadio = com.android.internal.R.string.phoneTypeRadio;
        public static final int phoneTypeTelex = com.android.internal.R.string.phoneTypeTelex;
        public static final int phoneTypeTtyTdd = com.android.internal.R.string.phoneTypeTtyTdd;
        public static final int phoneTypeWorkMobile = com.android.internal.R.string.phoneTypeWorkMobile;
        public static final int phoneTypeWorkPager = com.android.internal.R.string.phoneTypeWorkPager;
        public static final int phoneTypeAssistant = com.android.internal.R.string.phoneTypeAssistant;
        public static final int phoneTypeMms = com.android.internal.R.string.phoneTypeMms;
        public static final int phoneTypeCustom = com.android.internal.R.string.phoneTypeCustom;

        public static final int emailTypeHome = com.android.internal.R.string.emailTypeHome;
        public static final int emailTypeWork = com.android.internal.R.string.emailTypeWork;
        public static final int emailTypeOther = com.android.internal.R.string.emailTypeOther;
        public static final int emailTypeMobile = com.android.internal.R.string.emailTypeMobile;
        public static final int emailTypeCustom = com.android.internal.R.string.emailTypeCustom;

        public static final int postalTypeHome = com.android.internal.R.string.postalTypeHome;
        public static final int postalTypeWork = com.android.internal.R.string.postalTypeWork;
        public static final int postalTypeOther = com.android.internal.R.string.postalTypeOther;
        public static final int postalTypeCustom = com.android.internal.R.string.postalTypeCustom;

        public static final int imTypeHome = com.android.internal.R.string.imTypeHome;
        public static final int imTypeWork = com.android.internal.R.string.imTypeWork;
        public static final int imTypeOther = com.android.internal.R.string.imTypeOther;
        public static final int imTypeCustom = com.android.internal.R.string.imTypeCustom;

        public static final int imProtocolAim = com.android.internal.R.string.imProtocolAim;
        public static final int imProtocolMsn = com.android.internal.R.string.imProtocolMsn;
        public static final int imProtocolYahoo = com.android.internal.R.string.imProtocolYahoo;
        public static final int imProtocolSkype = com.android.internal.R.string.imProtocolSkype;
        public static final int imProtocolQq = com.android.internal.R.string.imProtocolQq;
        public static final int imProtocolGoogleTalk = com.android.internal.R.string.imProtocolGoogleTalk;
        public static final int imProtocolIcq = com.android.internal.R.string.imProtocolIcq;
        public static final int imProtocolJabber = com.android.internal.R.string.imProtocolJabber;
        public static final int imProtocolNetMeeting = com.android.internal.R.string.imProtocolNetMeeting;
        public static final int imProtocolCustom = com.android.internal.R.string.imProtocolCustom;

        public static final int orgTypeWork = com.android.internal.R.string.orgTypeWork;
        public static final int orgTypeOther = com.android.internal.R.string.orgTypeOther;
        public static final int orgTypeCustom = com.android.internal.R.string.orgTypeCustom;

        public static final int eventTypeOther = com.android.internal.R.string.eventTypeOther;
        public static final int eventTypeAnniversary = com.android.internal.R.string.eventTypeAnniversary;
        public static final int eventTypeBirthday = com.android.internal.R.string.eventTypeBirthday;

        public static final int sipAddressTypeHome = com.android.internal.R.string.sipAddressTypeHome;
        public static final int sipAddressTypeWork = com.android.internal.R.string.sipAddressTypeWork;
        public static final int sipAddressTypeOther = com.android.internal.R.string.sipAddressTypeOther;
    }
}