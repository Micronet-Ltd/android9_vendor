/* ======================================================================
*  Copyright (c) 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.location.sdk;

import java.util.List;

import android.content.Context;
import android.location.Location;

/**
 * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
 * <p>All Rights Reserved.</p>
 * <p>Confidential and Proprietary - Qualcomm Technologies, Inc</p>
 * <br/>
 * <p><b>IZatWWANDBReceiver</b> interface - the api for
 * injecting WWAN Cell Location database to IZat Location framework. </p>
 * @version 1.0.0
 */

public abstract class IZatWWANDBReceiver {
    protected final IZatWWANDBReceiverResponseListener mResponseListener;

    /**
     * IZatWWANDBReceiver
     * <p>
     * Constructor - IZatWWANDBReceiver </p>
     *
     * @param  listener the listener to receive WWAN DB Receiver
     *         responses. This parameter can not be null, otherwise
     *         a {@link IZatIllegalArgumentException} will be
     *         thrown.
     * @throws IZatIllegalArgumentException
     */
    protected IZatWWANDBReceiver(IZatWWANDBReceiverResponseListener listener)
                                throws IZatIllegalArgumentException {
        if(null == listener) {
            throw new IZatIllegalArgumentException("Unable to obtain IZatWWANDBReceiver instance");
        }
        mResponseListener = listener;
    }

    /**
     * Request list of base stations.
     * <p>
     * This allows WWAN database provider to request list of BSs
     * which needs location information.
     * </p>
     *
     * @param  expire_in_days the number of days in future in which
     *                        the associated location of a BS if
     *                        available, will expire, to be fetched
     *                        by this request. Optional Parameter.
     *                        If 0 is provided only BSs which has an
     *                        already expired location or no
     *                        location associated with, will be
     *                        fetched.
     */
    public abstract void requestBSList(int expire_in_days);

    /**
     * Request WWAN DB update.
     * <p>
     * This allows WWAN database provider to insert a list of BSs
     * with location information.
     * </p>
     *
     * @param location_data Location information of base stations.
     *                      If not available null/empty list can be
     *                      provided.
     * @param special_info Special information on base stations. If
     *                     not available null/empty list can be
     *                     provided.
     * @param days_valid days in future, for which location_data and
     *                   special_info will be valid. Optional
     *                   parameter. Default to 15 days if 0 is
     *                   provided.
     */
    public abstract void pushWWANDB(List<IZatBSLocationData> location_data,
                                    List<IZatBSSpecialInfo> special_info,
                                    int days_valid);

    /**
     * Interface class IZatWWANDBReceiverResponseListener.
     * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
     * <p>All Rights Reserved.</p>
     * <p>Confidential and Proprietary - Qualcomm Technologies, Inc</p>
     * <br/>
     * <p><b>IZatWWANDBReceiverResponseListener</b> is the interface
     * to receive responses from WWAN dabase receiver in IZat
     * location framework.</p>
     */
    public interface IZatWWANDBReceiverResponseListener {
        /**
         * Response for BS List request.
         * <p>
         * This API will be called by the underlying service back
         * to applications when list of BSs are available.
         * Applications should implement this interface.</p>
         *
         * @param bs_list the list of BSs
         */
        void onBSListAvailable(List<IZatBSInfo> bs_list);

        /**
         * Response for BS location injection request.
         * <p>
         * This API will be called by the underlying service back
         * to applications when BS Location injection completes.
         * Applications should implement this interface.</p>
         *
         * @param is_success the injection of BS locations success or
         *                   failure.
         * @param error the error details if the BS location injection
         *              was a failure.
         */
        void onStatusUpdate(boolean is_success, String error);

        /**
         * Service request to WWAN DB Provider.
         * <p>
         * This API will be called by the underlying service back
         * to applications when they need service. Applications should
         * implement this interface.</p>
         *
         */
        void onServiceRequest();
    }

    /**
     * class IZatBSInfo.
     * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
     * <p>All Rights Reserved.</p>
     * <p>Confidential and Proprietary - Qualcomm Technologies, Inc</p>
     */
    public static class IZatBSInfo {
        IZatBSCellInfo mCellInfo;

        /**
         * IZatBSInfo Constructor
         * <p>Constructor</p>
         *
         * @param Cell information for this BS
         */
        public IZatBSInfo(IZatBSCellInfo cellInfo) {
            mCellInfo = cellInfo;
        }

        /**
         * Get Cell Information
         * <p>
         *  Get cellInfo in which this BS is observed. This information
         *  is not supported now</p>
         *
         * @return IZatBSCellInfo
         */
        public IZatBSCellInfo getCellInfo() {
            return mCellInfo;
        }
    }

    /**
     * class IZatBSCellInfo.
     * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
     * <p>All Rights Reserved.</p>
     * <p>Confidential and Proprietary - Qualcomm Technologies, Inc</p>
     */
    public static class IZatBSCellInfo {
        public final int mRegionID1;
        public final int mRegionID2;
        public final int mRegionID3;
        public final int mRegionID4;

        /**
         * enum IZatBSCellTypes
         * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
         * <p>All Rights Reserved.</p>
         * <p>Confidential and Proprietary - Qualcomm Technologies,
         * Inc</p>
         */
        public enum IZatBSCellTypes {
            GSM, CDMA, WCDMA, LTE
        }
        public final IZatBSCellTypes mType;

        /**
         * Constructor - IZatBSCellInfo
         * <p>
         *  Constructor</p>
         *
         * @param regionID1 Mobile Country Code(MCC), For CDMA Set to 0
         * @param regionID2 Mobile Network Code(MNC), For CDMA set to
         *                  System ID(SID), For WCDMA set to 0 if not
         *                  vailable
         * @param regionID3 GSM: Local Area Code(LAC), WCDMA: Local Area
         *                  Code(LAC) set to 0 if not available, CDMA:
         *                  Network ID(NID), LTE: Tracking Area
         *                  Code(TAC) set to 0 if not available
         * @param regionID4 Cell ID(CID), For CDMA set to Base Station
         *                  ID(BSID)
         * @param type Cell type
         */
        public IZatBSCellInfo(int regionID1, int regionID2, int regionID3,
                              int regionID4, IZatBSCellTypes type) {
            mRegionID1 = regionID1;
            mRegionID2 = regionID2;
            mRegionID3 = regionID3;
            mRegionID4 = regionID4;
            mType = type;
        }
    }

    /**
     * class IZatBSLocationData.
     * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
     * <p>All Rights Reserved.</p>
     * <p>Confidential and Proprietary - Qualcomm Technologies, Inc</p>
     */
    public static class IZatBSLocationData {

        int mCellType;
        int mCellRegionID1;
        int mCellRegionID2;
        int mCellRegionID3;
        int mCellRegionID4;

        float mLatitude;
        float mLongitude;

        /**
         * enum IZatReliablityTypes
         * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
         * <p>All Rights Reserved.</p>
         * <p>Confidential and Proprietary - Qualcomm Technologies,
         * Inc</p>
         */
        public enum IZatReliablityTypes {
            VERY_LOW, LOW, MEDIUM, HIGH, VERY_HIGH
        }

        // BS Location masks
        public static final int IZAT_BS_LOC_WITH_LAT_LON = 0x0;
        public static final int IZAT_BS_LOC_HORIZONTAL_COV_RADIUS_VALID = 0x1;
        public static final int IZAT_BS_LOC_HORIZONTAL_CONFIDENCE_VALID = 0x2;
        public static final int IZAT_BS_LOC_HORIZONTAL_RELIABILITY_VALID = 0x4;
        public static final int IZAT_BS_LOC_ALTITUDE_VALID = 0x8;
        public static final int IZAT_BS_LOC_ALTITUDE_UNCERTAINTY_VALID = 0x10;
        public static final int IZAT_BS_LOC_ALTITUDE_CONFIDENCE_VALID = 0x20;
        public static final int IZAT_BS_LOC_ALTITUDE_RELIABILITY_VALID = 0x40;
        int mValidBits;

        float mHorizontalCoverageRadius;
        int mHorizontalConfidence;
        IZatReliablityTypes mHorizontalReliability;
        float mAltitude;
        float mAltitudeUncertainty;
        int mAltitudeConfidence;
        IZatReliablityTypes mAltitudeReliability;

        /**
         * Constructor - IZatBSLocationData
         * <p>
         *  Constructor </p>
         *
         * @param cell type
         * @param cell RegionID1
         * @param cell RegionID2
         * @param cell RegionID3
         * @param cell RegionID4
         * @param latitude Latitude of BS
         * @param longitude Longitude of BS
         */
        public IZatBSLocationData(int cellType,
                                  int cellRegionID1, int cellRegionID2,
                                  int cellRegionID3, int cellRegionID4,
                                  float latitude, float longitude) {

            mCellType = cellType;
            mCellRegionID1 = cellRegionID1;
            mCellRegionID2 = cellRegionID2;
            mCellRegionID3 = cellRegionID3;
            mCellRegionID4 = cellRegionID4;
            mLatitude = latitude;
            mLongitude = longitude;
            mValidBits = IZAT_BS_LOC_WITH_LAT_LON;
        }

        /**
         * Set Cell Type
         * <p>
         *  Set Cell Type.
         *  </p>
         * @param ct
         */
        public void setCellType(int ct) {
            mCellType = ct;
        }

        /**
         * Set Latitude
         * <p>
         * Set Latitude <p>
         *
         * @param latitude Latitude of BS
         */
        public void setLatitude(float latitude) {
            mLatitude = latitude;
        }

        /**
         * Set Longitude
         * <p>
         * Set Longitude <p>
         *
         * @param longitude Longitude of BS
         */
        public void setLongitude(float longitude) {
            mLongitude = longitude;
        }

        /**
         * Set Horizontal Coverage Radius
         * <p>
         *  Set Horizontal Coverage Radius.  </p>
         *
         * @param hcr Horizontal Coverage Radius
         */
        public void setHorizontalCoverageRadius(float hcr) {
            mHorizontalCoverageRadius = hcr;
            mValidBits |= IZAT_BS_LOC_HORIZONTAL_COV_RADIUS_VALID;
        }

        /**
         * Set Horizontal Confidence
         * <p>
         *  Set Horizontal Confidence of the location information provided.
         *  Range 0-100
         *  </p>
         * @param hc
         */
        public void setHorizontalConfidence(int hc) {
            mHorizontalConfidence = hc;
            mValidBits |= IZAT_BS_LOC_HORIZONTAL_CONFIDENCE_VALID;
        }

        /**
         * Set Horizontal Reliability
         * <p>
         *  Set Horizontal Reliability of the location information provided.
         *  VERY_LOW : when probability of position outlier 1 in one
         *  hundres or even more likely
         *  LOW : when probability of position outlier about 1 in a
         *  thousand
         *  MEDIUM : when probability of position outlier about 1 in a
         *  100 thousand
         *  HIGH : when probability of position outlier about 1 in a
         *  10 million
         *  VERY HIGH : when probability of position outlier about 1 in
         *  a thousand million
         *  until sufficient experience is obtained, the reliability
         *  input value should remain unset or set to LOW.
         *  </p>
         * @param reliability
         */
        public void setHorizontalReliability(IZatReliablityTypes reliability) {
            mHorizontalReliability = reliability;
            mValidBits |= IZAT_BS_LOC_HORIZONTAL_RELIABILITY_VALID;
        }

        /**
         * Set Altitude
         * <p>
         *  Set Altitude.  </p>
         *
         * @param alt Altitude
         */
        public void setAltitude(float alt) {
            mAltitude = alt;
            mValidBits |= IZAT_BS_LOC_ALTITUDE_VALID;
        }

        /**
         * Set Altitude Uncertainty
         * <p>
         *  Set Altitude Uncertainty.  </p>
         *
         * @param alu Altitude Uncertainty
         */
        public void setAltitudeUncertainty(float alu) {
            mAltitudeUncertainty = alu;
            mValidBits |= IZAT_BS_LOC_ALTITUDE_UNCERTAINTY_VALID;
        }

        /**
         * Set Altitude Confidence
         * <p>
         *  Set Altitude Confidence.  </p>
         *
         * @param alc Altitude Confidence
         */
        public void setAltitudeConfidence(int alc) {
            mAltitudeConfidence = alc;
            mValidBits |= IZAT_BS_LOC_ALTITUDE_CONFIDENCE_VALID;
        }

        /**
         * Set Altitude Reliability
         * <p>
         *  Set Altitude Reliability of the location information provided.
         *  VERY_LOW : when probability of position outlier 1 in one
         *  hundres or even more likely
         *  LOW : when probability of position outlier about 1 in a
         *  thousand
         *  MEDIUM : when probability of position outlier about 1 in a
         *  100 thousand
         *  HIGH : when probability of position outlier about 1 in a
         *  10 million
         *  VERY HIGH : when probability of position outlier about 1 in
         *  a thousand million
         *  until sufficient experience is obtained, the reliability
         *  input value should remain unset or set to LOW.
         *  </p>
         * @param reliability
         */
        public void setAltitudeReliability(IZatReliablityTypes reliability) {
            mAltitudeReliability = reliability;
            mValidBits |= IZAT_BS_LOC_ALTITUDE_RELIABILITY_VALID;
        }

        /**
         * Get Cell Type
         *
         * @return int
         */
        public int getCellType() {
            return mCellType;
        }

        /**
         * Get Cell RegionID1
         *
         * @return int
         */
        public int getCellRegionID1() {
            return mCellRegionID1;
        }

        /**
         * Get Cell RegionID2
         *
         * @return int
         */
        public int getCellRegionID2() {
            return mCellRegionID2;
        }

        /**
         * Get Cell RegionID3
         *
         * @return int
         */
        public int getCellRegionID3() {
            return mCellRegionID3;
        }

        /**
         * Get Cell RegionID4
         *
         * @return int
         */
        public int getCellRegionID4() {
            return mCellRegionID4;
        }

        /**
         * Get Latitude
         *
         * @return float
         */
        public float getLatitude() {
            return mLatitude;
        }

        /**
         * Get Longitude
         *
         * @return float
         */
        public float getLongitude() {
            return mLongitude;
        }

        /**
         * Get Horizontal Coverage Radius
         *
         * @throws IZatStaleDataException
         * @return float
         */
        public float getHorizontalCoverageRadius() throws IZatStaleDataException {
            if(0 == (IZAT_BS_LOC_HORIZONTAL_COV_RADIUS_VALID & mValidBits)) {
                throw new IZatStaleDataException("Horizontal coverage information is not valid");
            }
            return mHorizontalCoverageRadius;
        }

        /**
         * Get Horizontal Confidence
         *
         * @throws IZatStaleDataException
         * @return int
         */
        public int getHorizontalConfidence() throws IZatStaleDataException {
            if(0 == (IZAT_BS_LOC_HORIZONTAL_CONFIDENCE_VALID & mValidBits)) {
                throw new IZatStaleDataException("Horizontal confidence information is not valid");
            }
            return mHorizontalConfidence;
        }

        /**
         * Get Horizontal Reliability
         *
         * @throws IZatStaleDataException
         * @return IZatReliablityTypes
         */
        public IZatReliablityTypes getHorizontalReliability() throws IZatStaleDataException {
            if(0 == (IZAT_BS_LOC_HORIZONTAL_RELIABILITY_VALID & mValidBits)) {
                throw new IZatStaleDataException("Horizontal reliability information is not valid");
            }
            return mHorizontalReliability;
        }

        /**
         * Get Altitude
         *
         * @throws IZatStaleDataException
         * @return float
         */
        public float getAltitude() throws IZatStaleDataException {
            if(0 == (IZAT_BS_LOC_ALTITUDE_VALID & mValidBits)) {
                throw new IZatStaleDataException("Altitude information is not valid");
            }
            return mAltitude;
        }

        /**
         * Get Altitude Uncertainty
         *
         * @throws IZatStaleDataException
         * @return float
         */
        public float getAltitudeUncertainty() throws IZatStaleDataException {
            if(0 == (IZAT_BS_LOC_ALTITUDE_UNCERTAINTY_VALID & mValidBits)) {
                throw new IZatStaleDataException("Altitude uncertainty information is not valid");
            }
            return mAltitudeUncertainty;
        }

        /**
         * Get Altitude Confidence
         *
         * @throws IZatStaleDataException
         * @return int
         */
        public int getAltitudeConfidence() throws IZatStaleDataException {
            if(0 == (IZAT_BS_LOC_ALTITUDE_CONFIDENCE_VALID & mValidBits)) {
                throw new IZatStaleDataException("Altitude confidence information is not valid");
            }
            return mAltitudeConfidence;
        }

        /**
         * Get Altitude Reliability
         *
         * @throws IZatStaleDataException
         * @return IZatReliablityTypes
         */
        public IZatReliablityTypes getAltitudeReliability() throws IZatStaleDataException {
            if(0 == (IZAT_BS_LOC_ALTITUDE_RELIABILITY_VALID & mValidBits)) {
                throw new IZatStaleDataException("Altitude reliability information is not valid");
            }
            return mAltitudeReliability;
        }
    }


    /**
     * class IZatBSSpecialInfo.
     * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
     * <p>All Rights Reserved.</p>
     * <p>Confidential and Proprietary - Qualcomm Technologies, Inc</p>
     */
    public static class IZatBSSpecialInfo {

        int mCellType;
        int mCellRegionID1;
        int mCellRegionID2;
        int mCellRegionID3;
        int mCellRegionID4;

        /**
         * enum IZatBSSpecialInfoTypes
         * <p>Copyright (c) 2017 Qualcomm Technologies, Inc.</p>
         * <p>All Rights Reserved.</p>
         * <p>Confidential and Proprietary - Qualcomm Technologies,
         * Inc</p>
         */
        public enum IZatBSSpecialInfoTypes {
            NO_INFO_AVAILABLE, MOVING_BS
        }
        public final IZatBSSpecialInfoTypes mInfo;


        /**
         * Constructor - IZatBSSpecialInfo
         * <p>
         *  Constructor </p>
         *
         * @param info Information on this BS
         * @return
         *
         */
        public IZatBSSpecialInfo(int cellType,
                                 int cellRegionID1, int cellRegionID2,
                                 int cellRegionID3, int cellRegionID4,
                                 IZatBSSpecialInfoTypes info) {
            mCellType = cellType;
            mCellRegionID1 = cellRegionID1;
            mCellRegionID2 = cellRegionID2;
            mCellRegionID3 = cellRegionID3;
            mCellRegionID4 = cellRegionID4;
            mInfo = info;
        }

        /**
         * Get Cell Type
         *
         * @return int
         */
        public int getCellType() {
            return mCellType;
        }

        /**
         * Get Cell RegionID1
         *
         * @return int
         */
        public int getCellRegionID1() {
            return mCellRegionID1;
        }

        /**
         * Get Cell RegionID2
         *
         * @return int
         */
        public int getCellRegionID2() {
            return mCellRegionID2;
        }

        /**
         * Get Cell RegionID3
         *
         * @return int
         */
        public int getCellRegionID3() {
            return mCellRegionID3;
        }

        /**
         * Get Cell RegionID4
         *
         * @return int
         */
        public int getCellRegionID4() {
            return mCellRegionID4;
        }
    }

}
