/**
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierswitch;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * Record information of a carrier. It's a parcelable class, which
 * will be transfer from server to client.
 */
public class CarrierInfo implements Parcelable {
    /**
     * Id of carrier, {@link SwitchModule#CARRIER_MODE_DEFAULT,
     * @link SwitchModule#CARRIER_MODE_CHINAMOBILE,
     * @link SwitchModule#CARRIER_MODE_CHINATELECOM}
     */
    int id;
    /**
     * A indicator of a carrier
     */
    String name;
    /**
     * Resource Id of carrier name, used for multi-language
     */
    int resId;

    public CarrierInfo(int id, String name, int resId) {
        this.id = id;
        this.name = name;
        this.resId = resId;
    }

    private CarrierInfo(Parcel in) {
        readFromParcel(in);
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeInt(id);
        out.writeString(name);
        out.writeInt(resId);
    }

    public void readFromParcel(Parcel in) {
        id = in.readInt();
        name = in.readString();
        resId = in.readInt();
    }

    public int describeContents() {
        return 0;
    }

    public static final Parcelable.Creator<CarrierInfo> CREATOR =
            new Parcelable.Creator<CarrierInfo>() {
                public CarrierInfo createFromParcel(Parcel in) {
                    return new CarrierInfo(in);
                }

                public CarrierInfo[] newArray(int size) {
                    return new CarrierInfo[size];
                }
            };
}
