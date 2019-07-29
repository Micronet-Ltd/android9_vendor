/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch;

public class AbstractSwitcher implements ISwitcher {
    @Override
    public boolean needFactoryReset() {
        return false;
    }

    @Override
    public boolean needSecondReboot() {
        return false;
    }

    @Override
    public boolean executeNow(int oldCarrier, int newCarrier) {
        return true;
    }

    @Override
    public boolean executeAfterFactoryReset(int oldCarrier, int newCarrier) {
        return true;
    }

    @Override
    public boolean executeAfterSecondReboot(int oldCarrier, int newCarrier) {
        return true;
    }
}
