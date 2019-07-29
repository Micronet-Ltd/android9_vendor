/*
 *  Copyright (c) 2017 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.backup_wrapper;

public class WrapperNotSupportException extends Exception {

    public WrapperNotSupportException(){
    }

    public WrapperNotSupportException(String error){
        super(error);
    }

    public WrapperNotSupportException(String error, Throwable cause){
        super(error, cause);
    }
}
