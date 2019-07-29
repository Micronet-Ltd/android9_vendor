/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch;

/* Define interface of each specified Switchers */
public interface ISwitcher {
    /**
     * If need to factory reset to let the switcher available.
     */
    boolean needFactoryReset();
    /**
     * If need to do second Reboot after factory reset
     * to let switcher available.
     */
    boolean needSecondReboot();
    /**
     * Execute immediately when do carrier.
     * @param oldCarrier carrier Id of current carrier
     * @param newCarrier carrier Id of new carrier
     */
    boolean executeNow(int oldCarrier, int newCarrier);
    /**
     * Execute after factory reset.
     * @param oldCarrier carrier Id of current carrier
     * @param newCarrier carrier Id of new carrier
     */
    boolean executeAfterFactoryReset(int oldCarrier, int newCarrier);
    /**
     * Execute after second Reboot which is after factory reset.
     * @param oldCarrier carrier Id of current carrier
     * @param newCarrier carrier Id of new carrier
     */
    boolean executeAfterSecondReboot(int oldCarrier, int newCarrier);
}
