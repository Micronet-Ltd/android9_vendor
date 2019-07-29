/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch;

// Declare any non-default types here with import statements

import com.qualcomm.qti.carrierswitch.CarrierInfo;

interface ICarrierSwitchDeamonService {
    /**
     * Get current carrier Id
     */
    int getCurrentCarrier();
    /**
      * Get all supported carrier list
      */
    List<CarrierInfo> getSupportedCarrierList();
    /**
      * Switch carrier from old carrier to new carrier.
      */
    boolean switchCarrier(int oldCarrier, int newCarrier);
}
