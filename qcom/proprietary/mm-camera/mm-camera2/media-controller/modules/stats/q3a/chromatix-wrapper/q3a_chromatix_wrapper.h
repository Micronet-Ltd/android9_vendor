/*q3a_chromatix_wrapper .h
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __Q3A_CHROMATIX_WRAPPER_H__
#define __Q3A_CHROMATIX_WRAPPER_H__


//TODO: read chromatix version from the make file and avoid including this file
#include "chromatix.h"
#include "chromatix_3a.h"
#include "q3a_platform.h"

//=============================================================================
//=============================================================================
//  AF related params start
//=============================================================================
//=============================================================================
#define MAX_AF_KERNEL_NUM               2
#define MAX_HPF_2x5_BUFF_SIZE           10
#define MAX_HPF_2x11_BUFF_SIZE          22
#define FILTER_SW_LENGTH_FIR            11
#define FILTER_SW_LENGTH_IIR            6
#define MAX_ACT_MOD_NAME_SIZE           32
#define MAX_ACT_NAME_SIZE               32

#define AF_PD_MAX_NUM_ROI               10
#define AF_PD_MAX_TABLE_ENTRIES         20
#define AF_PD_MAX_DEFOCUS_RANGE         4
#define AF_TOF_MAX_DISTANCE_ENTRIES     8
#define AF_SCENE_CHANGE_MAX_ENTRIES     5
/* BAF related macro */
#define MAX_BAF_GAMMA_Y_ENTRY           3
#define MAX_BAF_GAMMA_LUT_ENTRY         32
#define MAX_BAF_FIR_ENTRY               13
#define MAX_BAF_IIR_ENTRY               10
#define MAX_BAF_CORING_ENTRY            17
#define MAX_BAF_FILTER                  3
#define MAX_BAF_FILTER_2ND_TIER         2
#define MAX_BAF_ROI_NUM                 282
#define MAX_HPF_BUFF_SIZE               22
#define AF_BV_COUNT                     8
#define DEFAULT_MAX_SCAN_CNT_TH         60
#define AF_GRID_SIZE_MAX                281
#define AF_LUMA_TABLE_SIZE              20

typedef enum _af_kernel_adapter_type
{
    Q3A_AF_KERNEL_PRIMARY = 0,
    Q3A_AF_KERNEL_SCALE,
} af_kernel_adapter_type;

//=============================================================================
// af_algo_adapter_type: Type of algorithm currently supported
//=============================================================================
typedef enum _af_algo_adapter_t
{
    Q3A_AF_PROCESS_DEFAULT = -2,
    Q3A_AF_PROCESS_UNCHANGED = -1,
    Q3A_AF_EXHAUSTIVE_SEARCH = 0,
    Q3A_AF_EXHAUSTIVE_FAST,
    Q3A_AF_HILL_CLIMBING_CONSERVATIVE,
    Q3A_AF_HILL_CLIMBING_DEFAULT,
    Q3A_AF_HILL_CLIMBING_AGGRESSIVE,
    Q3A_AF_FULL_SWEEP,
    Q3A_AF_SLOPE_PREDICTIVE_SEARCH,
    Q3A_AF_CONTINUOUS_SEARCH,
    Q3A_AF_PROCESS_MAX
} af_algo_adapter_t;

//=============================================================================
// af_single_index_adapter_t: Enum for indexing mapping for distance to
// lens position
//=============================================================================
typedef enum _af_single_index_adapter_t
{
    Q3A_AF_SINGLE_NEAR_LIMIT_IDX = 0,
    Q3A_AF_SINGLE_7CM_IDX = 1,
    Q3A_AF_SINGLE_10CM_IDX = 2,
    Q3A_AF_SINGLE_14CM_IDX = 3,
    Q3A_AF_SINGLE_20CM_IDX = 4,
    Q3A_AF_SINGLE_30CM_IDX = 5,
    Q3A_AF_SINGLE_40CM_IDX = 6,
    Q3A_AF_SINGLE_50CM_IDX = 7,
    Q3A_AF_SINGLE_60CM_IDX = 8,
    Q3A_AF_SINGLE_120CM_IDX = 9,
    Q3A_AF_SINGLE_HYP_F_IDX = 10,
    Q3A_AF_SINGLE_INF_LIMIT_IDX = 11,
    Q3A_AF_SINGLE_MAX_IDX = 12,
} af_single_index_adapter_t;

/**
* af_cam_name_adapter: Enum for camera profile
**/
typedef enum _af_cam_name_adapter
{
    Q3A_AF_ACT_MAIN_CAM_0,
    Q3A_AF_ACT_MAIN_CAM_1,
    Q3A_AF_ACT_MAIN_CAM_2,
    Q3A_AF_ACT_MAIN_CAM_3,
    Q3A_AF_ACT_MAIN_CAM_4,
    Q3A_AF_ACT_MAIN_CAM_5,
    Q3A_AF_ACT_WEB_CAM_0,
    Q3A_AF_ACT_WEB_CAM_1,
    Q3A_AF_ACT_WEB_CAM_2,
} af_cam_name_adapter;

/**
* af_baf_tuning_preset_adapter_t : enum for preset_id
*
**/
typedef enum _af_baf_tuning_preset_adapter_t
{
    Q3A_BAF_TUNING_CUSTOM = -1,
    Q3A_BAF_TUNING_PRESET_0,
    Q3A_BAF_TUNING_PRESET_1,
    Q3A_BAF_TUNING_PRESET_2,
    Q3A_BAF_TUNING_PRESET_3,
    Q3A_BAF_TUNING_PRESET_4,
    Q3A_BAF_TUNING_PRESET_5,
    Q3A_BAF_TUNING_PRESET_MAX
} af_baf_tuning_preset_adapter_t;

/**
* af_baf_roi_pattern_adapter_t : enum for ROI pattern type
*
**/
typedef enum _af_baf_roi_pattern_adapter_t
{
    Q3A_BAF_ROI_PATTERN_CUSTOM = -1,
    Q3A_BAF_ROI_PATTERN_RECT = 0,
    Q3A_BAF_ROI_PATTERN_CROSS,
    Q3A_BAF_ROI_PATTERN_DIAMOND,
    Q3A_BAF_ROI_PATTERN_SPARSE,
    Q3A_BAF_ROI_PATTERN_MAX
} af_baf_roi_pattern_adapter_t;

/**
* _af_scene_type_enum_t : enum for AF scene type
*
**/
typedef enum _af_scene_type_adapter_t
{
    Q3A_AF_SCENE_TYPE_NORMAL = 0,
    Q3A_AF_SCENE_TYPE_LOWLIGHT = 1,
    Q3A_AF_SCENE_TYPE_FACE = 2,
    Q3A_AF_SCENE_TYPE_MAX,
} af_scene_type_adapter_t;

/**
* af_tuning_fdac_adapter_t: Enum for indexing mapping for collected DAC for
*   face forward/up/down
**/
typedef enum _af_tuning_fdac_adapter_t
{
    Q3A_AF_TUNING_DAC_FACE_FORWARD = 0,
    Q3A_AF_TUNING_DAC_FACE_UP,
    Q3A_AF_TUNING_DAC_FACE_DOWN,
    Q3A_AF_TUNING_DAC_MAX
} af_tuning_fdac_adapter_t;

/**
* af_actuator_adapter_t: Enum for Type of actuator, which impacts core algo behaviors
**/
typedef enum _af_actuator_adapter_t
{
    Q3A_AF_ACT_TYPE_CLOSELOOP = 0,
    Q3A_AF_ACT_TYPE_OPENLOOP,
    Q3A_AF_ACT_TYPE_MAX,
} af_actuator_adapter_t;


/**
 * af_fv_stats_adapter_t: Enum for Type of Stats
 **/
typedef enum _af_fv_stats_adapter_t
{
    Q3A_AF_FV_STATS_TYPE_HW_1 = 0,
    Q3A_AF_FV_STATS_TYPE_HW_2 = 1,
    Q3A_AF_FV_STATS_TYPE_SW   = 2,
    Q3A_AF_FV_STATS_TYPE_MAX  = 3,
} af_fv_stats_adapter_t;

/**
 * af_haf_algo_adapter_t: Enum for HAF algorithm types
 *
 *    @Q3A_AF_HAF_ALGO_TOF: TOF algo
 *    @Q3A_AF_HAF_ALGO_PDAF: PDAF algo
 *    @Q3A_AF_HAF_ALGO_DCIAF: DCIAF algo
 *    @Q3A_AF_HAF_ALGO_DBG: Debug Algo
 **/
typedef enum
{
    Q3A_AF_HAF_ALGO_TOF = 0,
    Q3A_AF_HAF_ALGO_PDAF,
    Q3A_AF_HAF_ALGO_DCIAF,
    Q3A_AF_HAF_ALGO_DBG,
    Q3A_AF_HAF_ALGO_MAX,
} af_haf_algo_adapter_t;

/**
* af_cam_orientation_adapter_t: Direction of Aux wrt Main Camera
*
*    @Q3A_AF_CAM_ORIENT_LEFT: Towards left
*    @Q3A_AF_CAM_ORIENT_RIGHT: Towards right
*    @Q3A_AF_CAM_ORIENT_UP: Towards up
*    @Q3A_AF_CAM_ORIENT_DOWN: Towards down
**/
typedef enum _af_cam_orientation_adapter_t
{
    Q3A_AF_CAM_ORIENT_LEFT = 0,
    Q3A_AF_CAM_ORIENT_RIGHT,
    Q3A_AF_CAM_ORIENT_UP,
    Q3A_AF_CAM_ORIENT_DOWN,
    Q3A_AF_CAM_ORIENT_MAX,
} af_cam_orientation_adapter_t;

typedef struct _af_step_size_adapter_t
{
    unsigned short rgn_0; /* reserved */
    unsigned short rgn_1;
    unsigned short rgn_2;
    unsigned short rgn_3;
    unsigned short rgn_4; /* reserved */
} af_step_size_adapter_t;

typedef struct _af_step_size_table_adapter_t
{
    af_step_size_adapter_t  Prescan_normal_light;
    af_step_size_adapter_t  Prescan_low_light;
    af_step_size_adapter_t  Finescan_normal_light;
    af_step_size_adapter_t  Finescan_low_light;
} af_step_size_table_adapter_t;

/*
*af_follower_fine_search_step_info:
* Fine Search Step info for Tuning
*
*@stepSize : Step Size
*@far_steps : No of Steps in Far Direction
*@near_steps : No of Steps in Near Direction
*/
typedef struct _af_follower_fine_search_step_info {
  int                                       stepSize;
  int                                       far_steps;
  int                                       near_steps;
} af_follower_fine_search_step_info;

typedef enum {
  AF_FOLLOWER_MACRO_RANGE =0,
  AF_FOLLOWER_MID_RANGE,
  AF_FOLLOWER_FAR_RANGE,
  AF_FOLLOWER_MAX_RANGE
} af_follower_distance_range;

typedef enum{
  AF_ROI_SHIFT_DISABLED = 0,
  AF_ROI_SHIFT_BY_DISTANCE_DATA,
  AF_ROI_SHIFT_BY_DISTANCE_CALIB_DATA,
  AF_ROI_SHIFT_MAX,
}af_roi_shift_type;

typedef enum{
  AF_DISTANCE_ENTRY_ZERO = 0,
  AF_DISTANCE_ENTRY_ONE,
  AF_DISTANCE_ENTRY_TWO,
  AF_DISTANCE_ENTRY_THREE,
  AF_DISTANCE_ENTRY_FOUR,
  AF_DISTANCE_ENTRY_MAX,
} af_distance_entry_enum;

/**
* _af_stats_use_type: type of stats to use in algorithm.
**/
typedef enum _af_stats_use_type {
  AF_STATS_HV = 0,
  AF_STATS_HV_NORM,
  AF_STATS_H1,
  AF_STATS_H1_NORM,
  AF_STATS_H2,
  AF_STATS_H2_NORM,
  AF_STATS_V,
  AF_STATS_V_NORM,
  AF_STATS_SCALE,
  AF_STATS_SCALE_NORM,
  AF_STATS_MAX,
} af_stats_use_type;

typedef struct _af_tuning_tof_extended_adapter_t
{
  /// reserve[0]->resv_enabled
  /// Default Value: 1/true
  /// Range: [0 – 1]
  /// Description: A flag that indicate some tof reserve variables have proper values and can be used.
  float     resv_enabled;

  /// reserve[6]  ->frames_to_wait_stable
  /// Default Value: 3
  /// Range: [1 – 10]
  /// Description: Number of frames SAD need to be stable. If the number of stable frames are less
  /// than this threshold, panning will continue.
  float     frames_to_wait_stable;

  /// reserve[42]  ->tof_delay_flag_enable
  /// Default Value: 1/true
  /// Range: [0 – 1]
  /// Description: A flag that indicate if some new tof reserve parameter are valid.
  float     tof_delay_flag_enable;
} af_tuning_tof_extended_adapter_t;

typedef struct _af_tuning_pd_conf_tbl_entry_adapter_t
{
    float                                 noise_gain;
    int                                   min_conf;
} af_tuning_pd_conf_tbl_entry_adapter_t;

typedef struct _af_tuning_pd_conf_tbl_adapter_t
{
    int                                   num_entries;
    af_tuning_pd_conf_tbl_entry_adapter_t entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_conf_tbl_adapter_t;

typedef struct _af_tuning_pdaf_extended_adapter_t
{
  /// reserve[0] -> defocus_stable_filter_len
  /// Default Value: 3
  /// Range: [0, 10]
  /// Description: Number of successive PD samples in which max/min is calculated
  /// for assessing stability condition
  /// Constraints: The larger, longer it takes to meet stability criteria and for
  /// PDAF to trigger new search
  float           defocus_stable_filter_len;

  /// reserve[1] -> enable_trigger_suppr_by_scene_change
  /// Default Value: 0
  /// Range: [0, 1]
  /// Description: When enabled,  PDAF focus search is triggered only when a large
  /// defocus is also accompanied by scene change (i.e. gyro, focus value, luma change)
  /// Constraints: Generally disabled for PDAF Type 1 and enabled for PDAF Type 2 and 3.
  boolean         enable_trigger_suppr_by_scene_change;

  /// reserve[2] -> enable_fine_scan_extension
  /// Default Value: 0
  /// Range: [0, 1]
  /// Description: When enabled, fine scan searches for focus peak beyond the
  /// estimated range based on PD noise model
  /// Constraints: Generally disabled for PDAF Type 1 and enabled for PDAF Type
  /// 2 and 3.
  boolean         enable_fine_scan_extension;

  /// reserve[3] -> enable_do_not_use_defocus_sample_by_pd
  /// Default Value: 0
  /// Range: [0, 1]
  /// Description: When enabled, defocus sample is discarded if the reported
  /// defocus exceeds pd_max_limit_do_not_use_defocus_sample
  /// Constraints: Generally disabled for PDAF Type 1 and enabled for PDAF Type
  /// 2 and 3.
  boolean         enable_do_not_use_defocus_sample_by_pd;

  /// reserve[4] -> enable_inc_min_conf_thres_by_pd
  /// Default Value: 0
  /// Range: [0, 1]
  /// Description: When enabled, minimum confidence level is increased by a linear
  /// scaling factor, inc_min_conf_thres_by_pd_scaler, if defocus exceeds
  /// pd_max_limit_inc_min_conf_thres
  /// Constraints: Generally disabled for PDAF Type 1 and enabled for PDAF Type
  /// 2 and 3.
  boolean         enable_inc_min_conf_thres_by_pd;

  /// reserve[5] -> pd_max_limit_do_not_use_defocus_sample
  /// Default Value: 616
  /// Range: [0, 1000]
  /// Description: Represents out-of-bound limit of absolute value of  PD when
  /// enable_do_not_use_defcous_sample_by_pd is enabled
  /// Constraints: Specifies the maximum limit of PD above which PD sample is
  /// discarded by PDAF
  float           pd_max_limit_do_not_use_defocus_sample;

  /// reserve[6] -> pd_max_limit_inc_min_conf_thres
  /// Default Value: 422
  /// Range: [0, 1000]
  /// Description: Represents out-of-bound limit of absolute value of  PD which
  /// triggers increase in minimum confidence level when
  /// enable_inc_min_conf_thres_by_pd is enabled
  /// Constraints: Specifies the maximum limit of PD above which  a higher
  /// confidence level requirement is imposed
  float           pd_max_limit_inc_min_conf_thres;


  /// reserve[7] -> inc_min_conf_thres_by_pd_scaler
  /// Default Value: 1.5
  /// Range: [0, 3]
  /// Description: Represents the scaler by which the minimum confidence level
  /// is increased
  /// Constraints: Specifies the scaler used in increasing the minimum confidence level
  float           inc_min_conf_thres_by_pd_scaler;


  /// reserve[8] -> defocus_dof_multiplier
  /// Default Value: 1.2
  /// Range: [1, 2]
  /// Description: Represents the scaler by which the increase in lens
  /// depth-of-focus change from the far to the near focus limit is estimated
  /// Constraints: Specifies a linear scaler used in modeling the change in lens
  /// depth-of-focus from the far to the near focus limit
  float           defocus_dof_multiplier;

  /// reserve[9] -> first_run_not_conf_cnt_add
  /// Default Value: 0
  /// Range: [0, 5]
  /// Description: Represents additional number of frame count PDAF is allowed
  /// to wait for confident sample of PD at camera start-up
  /// Constraints: Increasing this number allows PDAF to wait longer for a
  /// confident PD sample at camera start-up (e.g. due to a delay in PD lib
  /// initialization). Setting this number too high can cause a lengthy delay
  /// in the transition between PDAF to  contrast-based AF at camera start-up.
  float           first_run_not_conf_cnt_add;

  /// reserve[10] -> inc_defocus_thres_prev_error
  /// Default Value: 1.5
  /// Range: [1, 3]
  /// Description: Represents the scaler by which defocus threshold is
  /// de-sensitized if previous PDAF focus converge resulted in a failure (e.g. focus hunting)
  /// Constraints: Set this value to ensure that PDAF does not oscillate between
  /// focus convengence and monitor by requiring a larger minimum defocus requirement
  /// for the transition out of monitor state. Generally disabled for PDAF Type 1
  /// and enabled for Type 2 and 3
  float           inc_defocus_thres_prev_error;

  /// reserve[11] -> soft_rgn_thres_multiplier
  /// Default Value: 0.6
  /// Range: [0, 1]
  /// Description: If defocus value is less than defocus threshold and
  /// above soft defocus threshold, then scene change determines if PDAF focus
  /// convergence is to be triggered
  /// Constraints: Soft threshold defocus is the calculated by defocus_threshold(tuning)
  /// multiplied by soft_rgn_thres_multiplier. Decreasing this number may cause PDAF
  /// to be more sensitive to defocus condition that is accompanied by a scene change.
  float           soft_rgn_thres_multiplier;

  /// reserve[12] -> fine_scan_range_enhance
  /// Default Value: 0
  /// Range: [0, 1]
  /// Description: When enabled this feature decreases the estimated fine scan
  /// range by 25%
  /// Constraints: Enable this feature to tighten PDAF fine scan range by 25%
  /// for PDAF sensors with lower PD noise
  boolean         fine_scan_range_enhance;

  /// reserve[13] -> enable_soft_conf_thresh
  /// Default Value: 0
  /// Range: [0, 1]
  /// Description: Enables feature which allow the minimum confidence level to
  /// slightly decrease during focus convergence due to increased aliasing
  /// Constraints: Enable this feature if PDAF frequently transitions to
  /// contrast-based AF in the midst of focus convergence. Generally disabled
  /// for Type 1 and enabled for Type 2 and 3
  boolean         enable_soft_conf_thresh;


  /// reserve[14] -> soft_conf_thresh_delta
  /// Default Value: 100
  /// Range: [0, 200]
  /// Description: Represents the decrease in minimum confidence level allowed
  /// during focus convergence if enable_soft_conf_thresh is enabled
  /// Constraints: Increasing this number allows PDAF to employ lower
  /// minimum confidence level during focus converge. Setting this number
  /// higher than the minimum confidence level causes minimum confidence level
  /// requirement to be omitted during focus converge; hence, a care needs to
  /// be taken in revising this parameter.
  int             soft_conf_thresh_delta;

  /// reserve[15] -> soft_conf_focus_pnt
  /// Default Value: 0.2
  /// Range: [0, 1]
  /// Description: Represents the scaler by which PDAF focus convergence slows
  /// down for PD samples whose confidence level resides between minimum confidence
  /// level and (minimum confidence level - soft_conf_thresh_delta)
  /// Constraints: Decreasing this number causes PDAF to perform focus convergence
  /// more slowly for PD samples whose confidence level lies between minimum
  /// confidence level and  (minimum confidence level - soft_conf_thresh_delta)
  float           soft_conf_focus_pnt;


  /// reserve[16] -> enable_soft_rgn_thres_high
  /// Default Value: 1
  /// Range: [0, 1]
  /// Description: It is a boolean flag which indicate if soft_defocus_thres_high
  /// will be used as a parameter to trigger focus in soft region when soft region
  /// threshold hig(enable_soft_rgn_thres_high) is enabled.
  boolean         enable_soft_rgn_thres_high;

  /// reserve[17] -> soft_rgn_thres_high_multiplier
  /// Default Value: 4
  /// Range: [2 - 6]
  /// Description: Defocus multiplier to make defocus threshold higher.
  /// Please refer to enable_soft_rgn_thres_high image
  float           soft_rgn_thres_high_multiplier;

  /// reserve[18] -> soft_scene_change_cnt
  /// Default Value: 1
  /// Range: [1 - 6]
  /// Description: number of scene changed reported by AF continues.
  float           soft_scene_change_cnt;

 /*
  * reserve[30] -> is_2PD_enable
  * Default Value: 0
  * Range: [0 - 1]
  * Description: Whether the sensor is a 2PD sensor and 2PD is enabled.*/
 float           is_2PD_enable;

 /*
  * reserve[31] -> face_2pd_min_move_thresh
  * Default Value: 0
  * Range: [0 - 7]
  * Description: While running face tracking 2PD, if defocus is less than this threshold
         face 2PD will not move.*/
 float           face_2pd_min_move_thresh;

 /*
  * reserve[32] -> face_2pd_roi_move_converge_suppress_enable
  * Default Value: 0
  * Range: [0 - 1]
  * Description: Enable flag for face 2pd ROI move converge suppress .
  Example: If face roi is moving over threshold, suppress pd from converging to avoid
      jittery user experience. */
 float           face_2pd_roi_move_converge_suppress_enable;

 /*
  * reserve[33] -> face_2pd_roi_stable_counter_thresh
  * Default Value: 3
  * Range: [1 - 100]
  * Description: Stable counter threshold for face 2PD roi move converge suppress.
    Once face ROI is less than moving threshold for continuous frames over the counter
    PDAF will resume converging */
 float           face_2pd_roi_stable_counter_thresh;

 /*
  * reserve[34] -> face_2pd_roi_move_thresh_x
  * Default Value: 0.5f
  * Range: [0.1 - 1.0f]
  * Description: Face tracking 2PD, if ROI is moving over defined X threshold, then
      converge suppress will occur.*/
 float           face_2pd_roi_move_thresh_x;

 /*
  * reserve[35] -> face_2pd_roi_move_thresh_y
  * Default Value: 0.5f
  * Range: [0.1 - 1.0f]
  * Description: Face tracking 2PD, if ROI is moving over defined Y threshold, then
     converge suppress will occur.*/
 float           face_2pd_roi_move_thresh_y;

 /*
  * reserve[36] -> dpd_fallback_conf
  * Default Value: 10
  * Range: [1 - 1000f]
  * Description: The threshold value to decide do we use PD information when fallback.
  *              Two PD performance is good enough. When fallback condition is match,
  *              we still can trust PD information when conf is higher then this threshold.
  * ex: fallback conf threshold is 60. we still can trust PD near/far range if
  * conf is higher than 10, then we can set this value to 10. When fallback happened,
  * we pass this range to CAF and do contrast-base AF in PD near/far range.
  */
 float           dpd_fallback_conf;

 /*
  * reserve[37] -> touch_pd_off_center_thresh
  * Default Value: 0.33f
  * Range: [0 - 1.0f]
  * Description: Radius distance from touch ROI to center ROI in terms of dist divided
  *              by CAMIF radius. This value is a percentage of camif radius.
  *  Image center PD perforamnce is better than corner. When defocus is large
  *  enough, corner image PD may not accurate enough and hit over-shoot situation.
  *  So we use this threshold to divide image to two zone, center/corner zone.
  *  If corner zone defocus is too bigger, we move lens with half defocus value.
  *  And do residual PDAF process after half defocus move.
  */
 float           touch_pd_off_center_thresh;

 /*
  * reserve[38] -> conv_done_temporal_enable
  * Default Value: 1
  * Range: [0,1]
  * Description: flag to enable converge done temproal filter feature.
                 2PD didn't use fine search, so we need restrict condition for converge done.
                 We use temporal filter to recorder history target_pos and use averge target
                 position to check is AF focused.
  */
 float           conv_done_temporal_enable;

 /*
  * reserve[39] -> conv_done_temporal_thresh
  * Default Value: 5
  * Range: [1 - 900]
  * Description: Threshold to judge does focus converge done when enable converge done temporal
  * filter feature. We calculate average target position and check does difference
  *              between average target position and next position. If difference is small than
  *              this threshold, focus converage is done.*/
 float           conv_done_temporal_thresh;
  
 /*
  * reserve[40] -> conv_done_temporal_filter_len
  * Default Value: 10
  * Range: [1 - 5]
  * Description: how much history target position we are recorded for converge done temporal
  * filter feature.
 */
 float           conv_done_temporal_filter_len;

 /*
  * reserve[41] -> face_pd_temporal_enable
  * Default Value: 1
  * Range: [0, 1]
  * Description: 2 PD face always keep in converge status for tracking face AF.
  *              It has hunting issue if PD value has variation, usually happend in small face.
  *              This feature use a temporal filter to calculate average target position and
  *              use target positon to calculate average defocus and next position.
  */
 float          face_pd_temporal_enable;

 /*
 * reserve[42] -> face_pd_temporal_len
 * Default Value: 3
 * Range: [1 - 10]
 * Description: how much history data used for face pd temporal filter
 */
 float           face_pd_temporal_len;

 /*
 * reserve[43] -> face_pd_temporal_conf_thres_high
 * Default Value: 700
 * Range: [1 - 1024]
 * Description: Face treaking AF is always in converge status. We move less lens position
 *               if confidence is low. Thise threshold use to specifiy move lens with 100%
 *   defocus when PD confidence large this threshold. If confidence is lower
 *   than min_conf, we don't move lens. Confidence between high and min_conf,
 *   we move lens with interpaltion percentage.
 */
 float           face_pd_temporal_conf_thres_high;

 /*
  * reserve[44] ->
  * Default Value:
  * Range:
  * Description: */

 /*
 * reserve[45] -> face_pd_fallback_enable
 * Default Value: 1
 * Range: [0, 1]
 * Description: Face PDAF is always in converge status and no fallback by default.
 *              This flag is used to enable fallback in some specification condition.
 *              If face fallback condition is met and this flag is enable, we go to
 *              fallback flow. Currently, face enter fallback when roi size is too small.
 */
 float           face_pd_fallback_enable;

 /*
 * reserve[46] -> face_pd_fallback_roi_size
 * Default Value: 200
 * Range: [0 - 4032]
 * Description: Face PDAF enter fallback status if one of ROI dimension is small than this
 *              threshold.
 */
 float           face_pd_fallback_roi_size;

 /*
 * reserve[47] -> fast_enter_lsp_in_low_conf_and_hunting_enable
 * Default Value: 1
 * Range: [0, 1]
 * Description: PD variation is large in some test scene and cause hunting issue.
 *              Force this test scene enter to LSP when sign of PD is reverted in some times.
 *              LSP will use contrast information to find final position.
 */
 float           fast_enter_lsp_in_low_conf_and_hunting_enable;

 /*
 * reserve[48] -> fast_enter_lsp_in_low_conf_and_hunting_threshold
 * Default Value: 2
 * Range: [0 - 10]
 * Description: PDAF stats is forced to enter focus converge done when sign of PD reverted counter
 *              is over this threshold.
 */
 float           fast_enter_lsp_in_low_conf_and_hunting_threshold;

 /*
 * reserve[50 -60] -> conf_table_high
 * Default Value:
    conf_tbl_h.num_entries = 5;
   conf_tbl_h.entries[0].noise_gain = 0;
   conf_tbl_h.entries[0].min_conf = 180;
   conf_tbl_h.entries[1].noise_gain = 1;
   conf_tbl_h.entries[1].min_conf = 180;
   conf_tbl_h.entries[2].noise_gain = 12;
   conf_tbl_h.entries[2].min_conf = 200;
   conf_tbl_h.entries[3].noise_gain = 24;
   conf_tbl_h.entries[3].min_conf = 200;
   conf_tbl_h.entries[4].noise_gain = 36;
   conf_tbl_h.entries[4].min_conf = 250;
 * num_entries
 * Range: [0 - 5]
 * noise_gain
 * Range: [0 - 40]
 * high_conf
 * Range: [0 - 1000]
 * Description: When the conf is over high_conf,  PDAF algorithm will skip fine search and all rely on PD's info to reach AF target.
                Otherwise, as the conf is btw high_conf and min_conf,  PDAF algorithm will use fine search to get AF target.
 */
 af_tuning_pd_conf_tbl_adapter_t        conf_table_high;

 } af_tuning_pd_extended_adapter_t;


typedef struct _BV_threshold_adapter_t
{
    float thres[AF_BV_COUNT]; /* CUR_BV_INFO */ /* 0, 20, 50, 100, 400, 700, OUTDOOR_, Sky */
} BV_threshold_adapter_t;

typedef struct _single_threshold_adapter_t
{
    float                       change_ratio_inc;
    float                       change_ratio_dec;
    float                       flat_inc_thres;
    float                       flat_dec_thres;
    float                       macro_thres;
    float                       drop_thres;
    unsigned long int           hist_dec_dec_thres;
    unsigned long int           hist_inc_dec_thres;
    BV_threshold_adapter_t      dec_dec_3frame;
    BV_threshold_adapter_t      inc_dec_3frame;
    BV_threshold_adapter_t      dec_dec;
    BV_threshold_adapter_t      dec_dec_noise;
    BV_threshold_adapter_t      inc_dec;
    BV_threshold_adapter_t      inc_dec_noise;
    BV_threshold_adapter_t      flat_threshold;
} single_threshold_adapter_t;

typedef struct _FVscore_threshold_adapter_t
{
    int                     default_stats;
    BV_threshold_adapter_t  score_ratio_showdif;
    float                   strict_noise;
    float                   normal_noise;
    float                   light_noise;
    int                     strong_peak_thres;
    int                     strong_inc_thres;
    int                     strong_dec_thres;
} FVscore_threshold_adapter_t;

typedef struct _single_optic_adapter_t
{
    unsigned short CAF_far_end;
    unsigned short CAF_near_end;
    unsigned short TAF_far_end;
    unsigned short TAF_near_end;
    unsigned short srch_rgn_1;
    unsigned short srch_rgn_2;
    unsigned short srch_rgn_3;
    unsigned short fine_srch_rgn;
    unsigned short far_zone;
    unsigned short near_zone;
    unsigned short mid_zone;
    unsigned short far_start_pos;
    unsigned short near_start_pos;
    unsigned short init_pos;
} single_optic_adapter_t;

typedef struct _af_single_frame_delay_adapter_t
{
    unsigned short       base_frame_delay;
    unsigned short       coarse_frame_delay;
    unsigned short       fine_frame_delay;
} af_single_frame_delay_adapter_t;

typedef struct _af_tuning_single_adapter_t
{
    unsigned short                    index[Q3A_AF_SINGLE_MAX_IDX]; /* af_single_index_adapter_t */
    unsigned short                    actuator_type;         /* af_actuator_adapter_t */
    unsigned short                    is_hys_comp_needed;
    unsigned short                    step_index_per_um;
    af_step_size_table_adapter_t      TAF_step_table;
    af_step_size_table_adapter_t      CAF_step_table;
    unsigned short                    PAAF_enable;
    single_threshold_adapter_t        sw;
    single_threshold_adapter_t        hw;
    float                             BV_gain[AF_BV_COUNT];            /* CUR_BV_INFO */
    single_optic_adapter_t            optics;
    FVscore_threshold_adapter_t       score_thresh;
    af_single_frame_delay_adapter_t   frame_delay;
} af_tuning_single_adapter_t;

typedef struct _af_tuning_sp_adapter
{
    float           fv_curve_flat_threshold;
    float           slope_threshold1;
    float           slope_threshold2;
    float           slope_threshold3;
    float           slope_threshold4;
    unsigned int    lens_pos_0;
    unsigned int    lens_pos_1;
    unsigned int    lens_pos_2;
    unsigned int    lens_pos_3;
    unsigned int    lens_pos_4;
    unsigned int    lens_pos_5;
    unsigned int    base_frame_delay;
    int             downhill_allowance;
    int             downhill_allowance_1;
} af_tuning_sp_adapter_t;

typedef struct _af_tuning_gyro_adapter
{
    unsigned char         enable;
    float                 min_movement_threshold;
    float                 stable_detected_threshold;
    unsigned short        unstable_count_th;
    unsigned short        stable_count_th;
    float                 fast_pan_threshold;
    float                 slow_pan_threshold;
    unsigned short        fast_pan_count_threshold;
    unsigned short        sum_return_to_orig_pos_threshold;
    unsigned short        stable_count_delay;
    unsigned short        num_of_samples;
} af_tuning_gyro_adapter_t;

typedef struct _af_tuning_lens_sag_comp_adapter
{
    unsigned char                         enable;
    unsigned int                          f_dac[Q3A_AF_TUNING_DAC_MAX];
    float                                 f_dist;
    float                                 trigger_threshold;
} af_tuning_lens_sag_comp_adapter_t;

typedef struct _af_tuning_pd_roi_config_adapter_t
{
    int                                   roi_loc_y;
    int                                   roi_loc_x;
    int                                   roi_num_rows;
    int                                   roi_num_cols;
} af_tuning_pd_roi_config_adapter_t;

typedef struct _af_tuning_pd_roi_adapter_t
{
    int                                   num_entries;
    af_tuning_pd_roi_config_adapter_t     config[AF_PD_MAX_NUM_ROI];
} af_tuning_pd_roi_adapter_t;

typedef struct _af_tuning_pd_noise_tbl_entry_adapter_t
{
    float                                 noise_gain;
    float                                 multiplier;
} af_tuning_pd_noise_tbl_entry_adapter_t;

typedef struct _af_tuning_pd_stable_tbl_entry_adapter_t
{
    int                                   fps;
    int                                   min_stable_cnt;
} af_tuning_pd_stable_tbl_entry_adapter_t;

typedef struct _af_tuning_pd_focus_tbl_entry_adapter_t
{
    int                                   defocus;
    float                                 move_pcnt;
} af_tuning_pd_focus_tbl_entry_adapter_t;

typedef struct _af_tuning_pd_noise_tbl_adapter_t
{
    int                                     num_entries;
    af_tuning_pd_noise_tbl_entry_adapter_t  entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_noise_tbl_adapter_t;

typedef struct _af_tuning_pd_focus_tbl_adapter_t
{
    int                                     num_entries;
    af_tuning_pd_focus_tbl_entry_adapter_t  entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_focus_tbl_adapter_t;

typedef struct _af_tuning_pd_stable_tbl_adapter_t
{
    int                                       num_entries;
    af_tuning_pd_stable_tbl_entry_adapter_t   entries[AF_PD_MAX_TABLE_ENTRIES];
} af_tuning_pd_stable_tbl_adapter_t;

typedef struct _af_tuning_pd_focus_scan_adapter_t
{
    int                                   focus_conv_frame_skip;
    int                                   enable_fine_scan;
    int                                   min_fine_scan_range;
    int                                   fine_scan_step_size;
    int                                   focus_done_threshold;
    int                                   enable_fine_scan_extension;
    int                                   first_run_not_conf_cnt_add;
} af_tuning_pd_focus_scan_adapter_t;

typedef struct _af_tuning_pd_monitor_adapter_t
{
    int                                   wait_after_focus_cnt;
    int                                   wait_conf_recover_cnt;
    float                                 defocused_threshold;
    float                                 depth_stable_threshold;
    int                                   defocus_stable_filter_len;
    int                                   enable_trigger_suppr_by_scene_change;
    float                                 defocus_dof_multiplier;
    float                                 inc_defocus_thres_scaler;
} af_tuning_pd_monitor_adapter_t;

typedef struct _af_tuning_pd_pdlib_adapter_t
{
    int                               enable;
    int                               max_shift;
    int                               resize_ratio;
    int                               confidence_scale;
    int                               smooth_filter[7];
    int                               sobel_filter[9];
    int                               defocus_confidence_th;
} af_tuning_pd_pdlib_adapter_t;

typedef struct _af_tuning_pd_preproc_adapter_t
{
    int                                   enable;
    int                                   enable_do_not_use_defocus_sample_by_pd;
    int                                   enable_inc_min_conf_thres_by_pd;
    float                                 pd_max_limit_do_not_use_defocus_sample;
    float                                 pd_max_limit_inc_min_conf_thres;
    float                                 inc_min_conf_thres_by_pd_scaler;
    float                                 pd_std_Threshold;
    int                                   pd_defocushi_err_sc_check;
    int                                   pd_avgdefocushi_err_sc_check;
    int                                   pd_lowlight_gain_thr_sc_check;
    int                                   pd_defocushi_sc_check[AF_PD_MAX_DEFOCUS_RANGE];
    int                                   pd_avgdefocushi_sc_check[AF_PD_MAX_DEFOCUS_RANGE];
    int                                   pd_lowlight_defocushi_sc_check[AF_PD_MAX_DEFOCUS_RANGE];
    int                                   pd_lowlight_avgdefocushi_sc_check[AF_PD_MAX_DEFOCUS_RANGE];
} af_tuning_pd_preproc_adapter_t;

typedef struct _af_tuning_pdaf_internal_adapter_t
{
    unsigned int                          type;
    unsigned int                          init_pos;
    unsigned int                          pdaf_enable_face_caf_priority;
    float                                 inc_defocus_thres_prev_error;
    unsigned int                          fine_scan_range_enhance;
} af_tuning_pd_internal_adapter_t;

typedef struct _af_tuning_pdaf_adapter_t
{
    af_tuning_pd_roi_adapter_t                      roi;
    af_tuning_pd_focus_tbl_adapter_t                focus_tbl;
    af_tuning_pd_noise_tbl_adapter_t                noise_tbl;
    af_tuning_pd_conf_tbl_adapter_t                 conf_tbl;
    af_tuning_pd_stable_tbl_adapter_t               stable_tbl;
    af_tuning_pd_focus_scan_adapter_t               focus_scan;
    af_tuning_pd_monitor_adapter_t                  scene_monitor;
    af_tuning_pd_pdlib_adapter_t                    pd_lib;
    af_tuning_pd_preproc_adapter_t                  pd_preproc;
    af_tuning_pd_internal_adapter_t                 pd_int;
    af_tuning_pd_extended_adapter_t                 pd_extended;
} af_tuning_pdaf_adapter_t;

typedef enum
{
    Q3A_AF_DCIAF_MACRO_RANGE = 0,
    Q3A_AF_DCIAF_MID_RANGE,
    Q3A_AF_DCIAF_FAR_RANGE,
    Q3A_AF_DCIAF_MAX_RANGE
} af_dciaf_distance_range_adapter;

typedef struct _af_dciaf_fine_search_step_adapter
{
    int                                   stepSize;
    int                                   far_steps;
    int                                   near_steps;
} af_dciaf_fine_search_step_adapter;

typedef struct _af_dciaf_extended
{

  /// reserve[0]  -> enable
  /// Default Value: 1/true
  /// Range: [0 – 1]
  /// Description: if the following reserve parameter have valid values.
  boolean    enable;

  /// reserve[1]  ->maxLowLightWaitMs
  /// Default Value: 250
  /// Range: [0 – 2000]
  /// Description: Convergence time out in Low light
  float      maxLowLightWaitMs;

  /// reserve[2]  ->maxNormalLightWaitMs
  /// Default Value: 250
  /// Range: [0 – 2000]
  /// Description: Convergence time out in Normal light
  float      maxNormalLightWaitMs;


  /// reserve[3]  ->jumpWithNoSkip
  /// Default Value: 15
  /// Range: [10 - 500]
  /// Description: If Skip frame is enabled, then if we have moved than the
  /// threshold of lens steps specified by this, then we need to skip a frame
  float      jumpWithNoSkip;


  /// reserve[4]  ->SkipframesBeforeFineSearch
  /// Default Value: 0
  /// Range: [0 - 5]
  /// Description: This will enable skip frame before fine search if we move beyond the threshold
  /// specified by [jumpWithNoSkip]
  float      SkipframesBeforeFineSearch;


  /// reserve[5]  ->dist_thresh_macro_region_in_mm
  /// Default Value: 200
  /// Range: [0 - 10000]
  /// Description: Distance threshold for Macro region in mm
  float      dist_thresh_macro_region_in_mm;


  /// reserve[6]  ->dist_thresh_non_macro_factor
  /// Default Value: 2
  /// Range: [0 - 10000]
  /// Description: Distance threshold for Non Macro region in mm
  float      dist_thresh_non_macro_factor;


  /// reserve[7]  ->lowLightThreshold
  /// Default Value: 3
  /// Range: [0 - 8]
  /// Description: Threshold used to compare against Scene BV to check
  /// if we are in Low Light
  float      lowLightThreshold;


  /// reserve[8]  ->useFineSearchExtension
  /// Default Value: 0
  /// Range: [0 - 1]
  /// Description: If Fine search extension needs to be enabled in HAF
  boolean    useFineSearchExtension;


  /// reserve[9]  ->macroRegionThreshold_in_mm
  /// Default Value: 200
  /// Range: [0 - 10000]
  /// Description: Macro region threshold in mm
  float      macroRegionThreshold_in_mm;


  /// reserve[10]  ->farRegionThresholdin_mm
  /// Default Value: 5000
  /// Range: [0 - 10000]
  /// Description: Far region threshold in mm
  float      farRegionThresholdin_mm;


  /// Fine Search Step info for Tuning
  /// stepSize : Step Size
  /// far_steps : No of Steps in Far Direction
  /// near_steps : No of Steps in Near Direction
  af_dciaf_fine_search_step_adapter  normalLightFineSearchInfo[Q3A_AF_DCIAF_MAX_RANGE];


  /// Fine Search Step info for Tuning
  /// stepSize : Step Size
  /// far_steps : No of Steps in Far Direction
  /// near_steps : No of Steps in Near Direction
  af_dciaf_fine_search_step_adapter  lowLightFineSearchInfo[Q3A_AF_DCIAF_MAX_RANGE];
} af_dciaf_extended;

typedef struct _af_tuning_dciaf_adapter_t
{
    unsigned char                         monitor_enable;
    unsigned int                          monitor_freq;
    unsigned int                          search_freq;
    float                                 baseline_mm;
    af_cam_orientation_adapter_t          aux_direction;
    float                                 macro_est_limit_cm;
    unsigned char                         alignment_check_enable;
    int                                   jump_to_start_limit;
    int                                   num_near_steps;
    int                                   num_far_steps;
    int                                   dciaf_step_size;
    unsigned char                         motion_data_use_enable;
    int                                   scene_change_lens_pos_th;
    int                                   panning_stable_lens_pos_th;
    int                                   panning_stable_duration_ms_bright;
    int                                   panning_stable_duration_ms_low;
    int                                   maxLowLightWaitMs;
    int                                   maxNormalLightWaitMs;
    int                                   jumpWithNoSkip;
    int                                   SkipframesBeforeFineSearch;
    int                                   dist_thresh_macro_region_in_mm;
    int                                   dist_thresh_non_macro_factor;
    int                                   lowLightThreshold;
    unsigned char                         useFineSearchExtension;
    int                                   macroRegionThreshold_in_mm;
    int                                   farRegionThresholdin_mm;
    af_dciaf_fine_search_step_adapter     normalLightFineSearchInfo[Q3A_AF_DCIAF_MAX_RANGE];
    af_dciaf_fine_search_step_adapter     lowLightFineSearchInfo[Q3A_AF_DCIAF_MAX_RANGE];
    af_dciaf_extended                     dciaf_extended;
} af_tuning_dciaf_adapter_t;

/*
*_af_tuning_follower_adapter_t:
* Tuning information which is not present in regular tuning data
*
*@enable : If follower data is present
*@gravity_comp_on : If Lens Sag Compension needs to be enabled
*@jump_to_start_limit : How much of a Jump Can we do in one shot before final convergence before we skip a frame
*@lowLightThreshold : Lux Index to indcate the low light level
*@useFineSearchExtension : if Fine search extension needs to be enabled
*@activeConvergenceFollow: Should Follower be actively Converging when peer is converging
*@lensPosThresholdForActiveConvergenceFollow : How much lens move should dictate a lens move on Peer
*@macroRegionThreshold_in_mm : Threshold to determine if its macro region
*@farRegionThresholdin_mm : Threshold to determine if its far region
*@normalLightFineSearchInfo : Fine Search Info for Normal Light
*@lowLightFineSearchInfo : Fine Search Info for Low Light
*/
typedef struct _af_tuning_follower_adapter_t
{
  boolean                                     enable;
  boolean                                     gravity_comp_on;
  int                                         jump_to_start_limit;
  int                                         lowLightThreshold;
  boolean                                     useFineSearchExtension;
  boolean                                     activeConvergenceFollow;
  int                                         lensPosThresholdForActiveConvergenceFollow;
  int                                         macroRegionThreshold_in_mm;
  int                                         farRegionThresholdin_mm;
  af_follower_fine_search_step_info           normalLightFineSearchInfo[AF_FOLLOWER_MAX_RANGE];
  af_follower_fine_search_step_info           lowLightFineSearchInfo[AF_FOLLOWER_MAX_RANGE];
} af_tuning_follower_adapter_t;

typedef struct _af_tuning_sad_adapter
{
    unsigned char            enable;
    float                    gain_min;
    float                    gain_max;
    float                    ref_gain_min;
    float                    ref_gain_max;
    float                    threshold_min;
    float                    threshold_max;
    float                    ref_threshold_min;
    float                    ref_threshold_max;
    unsigned short           unstable_count_th;
    unsigned short           stable_count_th;
    unsigned short           frames_to_wait;
    unsigned short           grid_region_start;
    unsigned short           grid_region_end;
    float                    sadr_threshold_scaler;
    float                    aec_gain_input[AF_SCENE_CHANGE_MAX_ENTRIES];
    float                    sadr_threshold_scaler_multiplier[AF_SCENE_CHANGE_MAX_ENTRIES];
} af_tuning_sad_adapter_t;

//=============================================================================
// af_tuning_chroma_t: Chroma based scene change detection
//
// Variable name: enable
// Description: enable flag for chroma based scene change detection feature
// 3A version:
// Default value: 0
// Data range: 0 or 1
// Constraints:
//
// Variable name: g_over_b_thres
// Description: when G/B ratio is greater than this threshold
// scene change is detected and AF is triggered
// 3A version:
// Default value: 0.5
// Data range: [0 5]
// Constraints: Higher the threshold difficult to trigger AF
//              Smaller the threshold eady to trigger AF
//
// Variable name: g_over_r_thres
// Description: when G/R ratio is greater than this threshold
// scene change is detected and AF is triggered
// 3A version:
// Default value: 0.4
// Data range: [0 5]
// Constraints: Higher the threshold difficult to trigger AF
//              Smaller the threshold eady to trigger AF
//
// Variable name: aec_gain_input
// Description: Setting different gain levels to have different thresholds
// 3A version:
// Default value: {5, 10, 18, 22, 30}
// Data range: [0 30]
// Constraints:
//
// Variable name: g_over_b_threshold_scaler
// Description: Scaler threshold to multiple with g_over_b_thres at
// different gain level
// 3A version:
// Default value: {1, 1.5, 2, 2.2, 2.5}
// Data range: [1 5]
// Constraints: Higher the threshold difficult to trigger AF
//              Smaller the threshold eady to trigger AF
//
// Variable name: g_over_r_threshold_scaler
// Description: Scaler threshold to multiple with g_over_r_thres at
// different gain level
// 3A version:
// Default value: {1, 1.5, 2, 2.2, 2.5}
// Data range: [1 5]
// Constraints: Higher the threshold difficult to trigger AF
//              Smaller the threshold eady to trigger AF
//
// Variable name: unstable_count_th
// Description: number of frames to detect unstable scene change
// 3A version:
// Default value: 5
// Data range: [1 20]
// Constraints: Higher the threshold difficult to trigger AF
//              Smaller the threshold eady to trigger AF
//
// Variable name: num_of_samples
// Description: number of frames to monitor for scene change detection
// 3A version:
// Default value: 5
// Data range: [1 20]
//=============================================================================
typedef struct _af_tuning_chroma_adapter_t
{
    unsigned char     enable;
    float             g_over_b_thres;
    float             g_over_r_thres;
    float             aec_gain_input[AF_SCENE_CHANGE_MAX_ENTRIES];
    float             g_over_b_threshold_scaler[AF_SCENE_CHANGE_MAX_ENTRIES];
    float             g_over_r_threshold_scaler[AF_SCENE_CHANGE_MAX_ENTRIES];
    unsigned short    unstable_count_th;
    unsigned short    num_of_samples;
} af_tuning_chroma_adapter_t;

typedef struct _af_tof_fine_scan_adapter_t
{
  int                                   num_near_steps_near_direction;
  int                                   num_far_steps_near_direction;
  int                                   num_near_steps_far_direction;
  int                                   num_far_steps_far_direction;
  int                                   num_near_steps_far_converge_point;
  int                                   num_far_steps_far_converge_point;
  int                                   near_steps_far_direction_tof_conf;
  int                                   tof_step_size;
  int                                   tof_step_size_low_light;
  int                                   far_distance_fine_step_size;
} af_tof_fine_scan_adapter_t;

typedef struct _af_tof_frame_delay_adapter_t
{
    int                                   frame_delay_focus_converge_bright_light;
    int                                   frame_delay_focus_converge_low_light;
    int                                   frame_delay_fine_search_bright_light;
    int                                   frame_delay_fine_search_low_light;
    int                                   frame_delay_outdoor_light;
} af_tof_frame_delay_adapter_t;

typedef struct _af_tof_scene_change_adapter_t
{
    af_tuning_sad_adapter_t               af_sad;
    int                                   scene_change_distance_thres[AF_TOF_MAX_DISTANCE_ENTRIES];
    int                                   scene_change_distance_std_dev_thres;
    int                                   distance_region[AF_TOF_MAX_DISTANCE_ENTRIES];
    int                                   far_distance_unstable_cnt_factor;
    int                                   panning_stable_cnt;
    int                                   panning_stable_thres;
    int                                   normal_light_cnt;
    int                                   lowlight_cnt;
    int                                   num_monitor_samples;
} af_tof_scene_change_adapter_t;

typedef struct _af_tof_flag_adapter_t
{
    int                                   use_custom_sensitivity;
    int                                   enable_sad_when_tof_is_conf;
    int                                   run_tof_in_outdoor_light_flag;
    int                                   aec_wait_check_flag_enable;
    int                                   far_scene_coarse_srch_enable;
} af_tof_flag_adapter_t;

typedef struct _af_tof_filter_adapter_t
{
    int                                   median_filter_enable;
    int                                   median_filter_num_of_samples;
    int                                   average_filter_enable;
    int                                   average_filter_num_of_samples;
} af_tof_filter_adapter_t;

typedef struct _af_tof_focus_converge_adapter_t
{
    int                                   jump_to_start_limit;
    int                                   jump_to_start_limit_low_light;
    int                                   start_pos_for_tof;
    int                                   far_converge_point;
} af_tof_focus_converge_adapter_t;

typedef struct _af_tuning_tof_adapter_t
{
    float                                 sensitivity;
    int                                   outdoor_lux_idx;
    int                                   jump_to_start_limit;
    int                                   jump_to_start_limit_low_light;
    int                                   start_pos_for_tof;
    int                                   far_converge_point;
    int                                   normal_light_cnt;
    int                                   lowlight_cnt;
    int                                   num_monitor_samples;
    int                                   tof_step_size;
    af_tof_filter_adapter_t               tof_filter;
    af_tof_flag_adapter_t                 tof_flag;
    af_tof_frame_delay_adapter_t          frame_delay;
    af_tof_scene_change_adapter_t         scene_change;
    af_tof_focus_converge_adapter_t       focus_converge;
    af_tof_fine_scan_adapter_t            fine_scan;
    af_tuning_tof_extended_adapter_t      tof_extended;
} af_tuning_tof_adapter_t;

typedef struct _af_fine_srch_extension_adapter_t
{
    unsigned char                        enable;
    int                                  max_fine_srch_extension_cnt;
    int                                  num_near_steps;
    int                                  num_far_steps;
    int                                  step_size;
    float                                decrease_drop_ratio;
    float                                fine_srch_reserve[10];
} af_fine_srch_extension_adapter_t;


typedef struct _af_tuning_haf_extended_adapter_t
{
  /// reserve[0] -> max_fine_srch_extension_cnt
  /// Default Value: 3
  /// Range: [0 , 5]
  /// Description: This parameter keeps extending the range until max_fine_srch_extension_cnt
  /// is met. For example: if the range provided by PDAF/TOF/DciAF is [200, 230]
  /// and current lens pos is 200, it does search from 200 to 230 and if we keep
  /// getting increasing FV trend, then it means that the peak is beyond 230, so
  /// now it extends the range to [230, 260] and performs the search, again if it
  /// doesn’t find a peak then does a extension again to [260, 290] now the
  /// extension has been done 2 times, say if the max_fine_srch_extension_cnt
  /// is set to 3 then it would do extension three times.
  ///Constraints: Larger the number, more the search time, If we set small number
  /// such 0 then it would not perform the extension at all.
  float     max_fine_srch_extension_cnt;

  /// reserve[1] -> num_near_steps
  /// Default Value: 2
  /// Range: [0, 10]
  /// Description: In order to find the near_limit, num_near_steps is multiplied
  /// with the step_size and is added to the current position to determine the near_limit
  /// Constraints: Larger the number, larger the search range hence search time will increase
  float     num_near_steps;

  /// reserve[2] -> num_far_steps
  /// Default Value: 2
  /// Range: [0, 10]
  /// Description: In order to find the far_limit, num_far_steps is multiplied
  /// with the step_size and is added to the current position to determine the far_limit
  /// Constraints: Larger the number, larger the search range hence search time will increase
  float     num_far_steps;

  /// reserve[3] -> step_size
  /// Default Value: 6
  /// Range: [1, 100]
  /// Description: The step size used to find the search range if the extension is required.
  /// Constraints: Larger the number, larger the search range hence search time will increase
  float     step_size;

  ///Reseve[4]->decrease_drop_ratio
  /// Default Value: 0.985
  /// Range: [0, 1]
  /// Description: The decrease drop ratio is used to determine the stop criteria
  /// for the search. If the ratio of current frame fv by previous frame fv is
  /// less than decrease_drop_ratio then the search stops.
  /// Constraints: higher the number easily to find the peak but more susceptible to the noise
  float     decrease_drop_ratio;

  /// reserve[5] -> enable_init_pos:
  /// Default Value: 0/false
  /// Range: [0 – 1]
  /// Description: If this filed is enable, the initial/default lens position
  /// will be between [SINGLE_NEAR_LIMIT_IDX - SINGLE_INF_LIMIT_IDX], otherwise
  /// default lens positing will be infinity.
  boolean   enable_init_pos;

  /// reserve[6] -> idx
  /// Default Value: 0
  /// Range: [0 – 11]
  /// Description: index of initial lens position. This filed will be used if
  /// enable_init_pos is enable
  float     idx;

  /// reserve[7] -> depth_conf_cnt_th
  /// Default Value: 2
  /// Range: [1 – 10]
  /// Description: The number of frames which algo should return confident in
  /// order to overwrite caf decision.
  float     depth_conf_cnt_th;

  /// reserve[10] -> spotlight_enabled
  /// Default Value: 0
  /// Range: 0 or 1
  /// Description: Enable flag for spotlight AF process.
  boolean   spotlight_enabled;

  /// reserve[11] -> AF_SPOT_LIGHT_INF_FLAT_THRS
  /// Default Value: 0.55
  /// Range: float [0.0 – 1.0]
  /// Description: Flat threshold for far scene when detecting a spotlight condition.
  /// The lower the spotlight infinity scene easier to be recognized as flat scene
  float     AF_SPOT_LIGHT_INF_FLAT_THRS; //11

  /// reserve[12] -> AF_SPOT_LIGHT_MARCO_FLAT_THRS
  /// Default Value: 0.85
  /// Range: float [0.0 – 1.0]
  /// Description: Flat_threshold for macro scene when detecting spotlight condition.
  /// The lower the spotlight macro focus scene easier to be recognized as flat scene
  float     AF_SPOT_LIGHT_MARCO_FLAT_THRS; //12

  /// reserve[13] -> AF_SPOT_LIGHT_INC_FLAT_THRS
  /// Default Value: 1.035
  /// Range: float [1-3]
  /// Description: Threshold to judge if current AF position is in increase or flat condition.
  /// General Scene = 1.005 Spotlight Scene = 1.035
  ///The larger the value, the harder to be recognized as FV increment.
  /// Increasing inc_flat_th and decreasing dec_flat_th helps to prevent ending
  /// in a false peak position and to recognize it as a flat curve.
  float     AF_SPOT_LIGHT_INC_FLAT_THRS; //13

  /// reserve[14] -> AF_SPOT_LIGHT_DEC_FLAT_THRS
  /// Default Value: 0.978
  /// Range: float [0 – 1]
  /// Description: Threshold to judge if current AF position is in decrease or
  /// flat condition. General Scene = 0.995, Spotlight Scene = 0.978. The lower
  /// the value the harder to be recognized as FV decrement. Increasing inc_flat_th
  /// and decreasing dec_flat_th helps to prevent ending in a false peak position
  /// and to recognize it as a flat curve.
  float     AF_SPOT_LIGHT_DEC_FLAT_THRS; //14

  /// reserve[15] -> exp_index_th
  /// Default Value: 300
  /// Range: int [0 – 500]
  /// Description: Exposure index threshold, if exposure index is smaller than
  /// this threshold, the spotlight will not be detected. The smaller the threshold,
  /// the less chances that spotlight can be detected
  /// Constraints: data value need to be bigger than aec_indoor_index
  float     exp_index_th;

  /// reserve[16] -> af_bright_region_zone_start_h
  /// Default Value: 4
  /// Range: uint32_t [0 – 15]
  /// Description: Table that defines the spotlight zone size. This value defines
  /// the horizontal start location in terms of grid of bright region zone
  /// Constraints: The resulting grid should be equal to or smaller than the
  /// background zone size
  float     af_bright_region_zone_start_h;

  /// reserve[17] -> af_bright_region_zone_end_h
  /// Default Value: 12
  /// Range: uint32_t [0 – 15]
  /// Description: Table that defines the spotlight zone size. This value defines
  /// the horizontal end location in terms of grid of bright region zone
  /// Constraints: The resulting grid should be equal to or smaller than the
  /// background zone size
  float     af_bright_region_zone_end_h;

  /// reserve[18] -> af_bright_region_zone_start_v
  /// Default Value: 4
  /// Range: uint32_t [0 – 15]
  /// Description: Table that defines the spotlight zone size.
  /// This value defines the vertical start location in terms of grid of bright
  /// region zone
  /// Constraints: The resulting grid should be equal to or smaller than the
  /// background zone size
  float     af_bright_region_zone_start_v;

  /// reserve[19] -> af_bright_region_zone_end_v
  /// Default Value: 12
  /// Range: uint32_t [0 – 15]
  /// Description: Table that defines the spotlight zone size. This value defines
  /// the vertical end location in terms of grid of bright region zone
  /// Constraints: The resulting grid should be equal to or smaller than the
  /// background zone size
  float     af_bright_region_zone_end_v;

  /// reserve[20] -> af_background_zone_start_h
  /// Default Value: 2
  /// Range: uint32_t [0 – 15]
  /// Description:  Table that defines the background zone location and size.
  /// This variable defines the starting grid horizontal location of background zone.
  /// Constraints: The resulting grid should be equal to or larger than the
  /// spotlight detection zone size
  float     af_background_zone_start_h;

  /// reserve[21] -> af_background_zone_end_h
  /// Default Value: 14
  /// Range: uint32_t [0 – 15]
  /// Description: Table that defines the background zone location and size.
  /// This variable defines the end grid horizontal location of background zone.
  /// Constraints: The resulting grid should be equal to or larger than the
  /// spotlight detection zone size
  float     af_background_zone_end_h;

  /// reserve[22] -> af_background_zone_start_v
  /// Default Value: 2
  /// Range: uint32_t [0 – 15]
  /// Description: Table that defines the background zone location and size.
  /// This variable defines the starting grid vertical location of background zone.
  /// Constraints: The resulting grid should be equal to or larger than the
  /// spotlight detection zone size
  float     af_background_zone_start_v;

  /// reserve[23] -> af_background_zone_end_v
  /// Default Value: 14
  /// Range: uint32_t [0 – 15]
  /// Description: Table that defines the background zone location and size.
  /// This variable defines the en grid vertical location of background zone.
  /// Constraints: The resulting grid should be equal to or larger than the
  /// spotlight detection zone size
  float     af_background_zone_end_v;

  /// reserve[24] -> bright_region_luma_delta_background
  /// Default Value: 2.0  
  /// Range: float [1.0 – 8.0]
  /// Description: if region’s luma is higher than average luma of background zone
  /// multiply this parameter, this region will be recognize as a bright region  
  /// Constraints: Give higher value will be harder to recognize region as
  /// spotlight
  float     bright_region_luma_delta_background;

  /// reserve[25] -> bright_region_luma_delta_spotlight_zone_low_grey
  /// Default Value: 2.5
  /// Range: float [0.0 – 15.0]
  /// Description: if region’s luma of cluster zone is higher than average luma
  /// of background zone,  multiply it by this parameter. The region will be
  /// recognized as a bright region. The bigger the threshold, the less likely
  /// that region is detected as a bright region.
  /// Constraints: Suggest to keep default
  float     bright_region_luma_delta_spotlight_zone_low_grey;

  /// reserve[26] -> bright_region_luma_delta_spotlight_zone_dark_grey
  /// Default Value: 3.0
  /// Range: float [0.0 – 15.0]
  /// Description: if region’s luma of cluster zone is higher than average luma
  /// of background zone,  multiply it by this parameter. The region will be
  /// recognized as a bright region. The bigger the threshold, the less likely
  /// that region is detected as a bright region.
  /// Constraints: Suggest to keep default
  float     bright_region_luma_delta_spotlight_zone_dark_grey;

  /// reserve[27] -> region_low_grey_th
  /// Default Value: 30
  /// Range: uint32 [0 – 255]
  /// Description: The threshold of the low grey region. If the region’s luma of
  /// the background zone is smaller than this threshold, this region is recognized
  /// as a low grey region. The smaller the threshold, the less likely that region
  /// can be recognized as a low grey region.
  /// Constraints: Should be larger than region_dark _grey_th
  float     region_low_grey_th;

  /// reserve[28] -> region_dark_grey_th
  /// Default Value: 7
  /// Range: uint32 [0 – 255]
  /// Description: The threshold of the dark grey region, if the region’s luma of
  /// the background zone is smaller than this threshold, this region will be
  /// recognized as a dark grey region. The smaller the threshold, the less likely
  /// that region can be recognized as a dark grey region.
  /// Constraints: Should be smaller than region_low _grey_th
  float     region_dark_grey_th;

  /// reserve[29] -> background_low_grey_pct_th
  /// Default Value: 0.85
  /// Range: float [0.6 – 1.0]
  /// Description: The threshold percentage of low in the background zone.
  /// If low grey percentage of the background zone is larger than this threshold,
  /// the system considers the background is dark enough. The bigger the threshold,
  /// the less likely the system will consider the background is dark enough.
  /// Constraints: Should be bigger than BACKGROUND_DARK_GREY_PCT_TH. Suggest to
  /// keep default. A lower value is not recommended.
  float     background_low_grey_pct_th;

  /// reserve[30] -> background_dark_grey_pct_th
  /// Default Value: 0.3
  /// Range: float [0.2 – 1.0]
  /// Description: The threshold percentage of dark grey in the background zone.
  /// If dark grey percentage of the background zone is larger than this threshold,
  /// the system considers the background is dark enough. The bigger the threshold,
  /// the less likely the system will consider the background is dark enough.
  /// Constraints: Should be smaller than BACKGROUND_LOW_GREY_PCT_TH.
  /// Recommend to adjust by user preference
  float     background_dark_grey_pct_th;

  /// reserve[31] -> background_luma_ave_th
  /// Default Value: 30
  /// Range: uint32_t [0 – 255]
  /// Description: The threshold for average luma of the background zone, if for
  /// average luma of the background zone is bigger than this threshold, the
  /// spotlight will not be detected. The larger the threshold, the more chances
  /// that spotlight can be detected.
  /// Constraints: None
  float     background_luma_ave_th;

  /// reserve[32] -> square_cluster_increase_size
  /// Default Value: 1
  /// Range: uint32_t [0 – 3]
  /// Description: The extended size of the bright region square cluster,
  /// both vertically and horizontally. Set to 0 to disable a cluster
  /// shape (square, vertical, horizontal) extension. Just one bright region
  /// can be recognized as spotlight.
  /// Constraints: The extended size should not be over the background zone size.
  /// Suggest to keep default.
  float     square_cluster_increase_size;

  /// reserve[33] -> square_cluster_pct_th
  /// Default Value:
  /// Range:
  /// Description: The threshold percentage of bright region in the cluster zone.
  /// If bright region percentage of the cluster zone is larger than this threshold
  /// and background is dark enough, the flag of bright region cluster will be raised.
  /// The bigger the threshold, the less likely that cluster is detected.
  /// Constraints: If set to 1.0, the cluster is disabled. Suggest to keep default
  float     square_cluster_pct_th;

  /// reserve[34] -> vertical_cluster_increase_size
  /// Default Value: 2
  /// Range: uint32_t [0 – 3]
  /// Description: The extended size of the bright region vertical cluster.
  /// Set to 0 to disable a cluster shape (square, vertical, horizontal) extension.
  /// Just one bright region can be recognized as spotlight.
  /// Constraints: The extended size should not be over the background zone size.
  /// Suggest to keep default.
  float     vertical_cluster_increase_size;

  /// reserve[35] -> vertical_cluster_pct_th
  /// Default Value:
  /// Range:
  /// Description: The threshold percentage of bright region in the cluster zone.
  /// If bright region percentage of the cluster zone is larger than this threshold
  /// and background is dark enough, the flag of bright region cluster will be raised.
  /// The bigger the threshold, the less likely that cluster is detected.
  /// Constraints: If set to 1.0, the cluster is disabled. Suggest to keep default
  float     vertical_cluster_pct_th;

  /// reserve[36] -> horizontal_cluster_increase_size
  /// Default Value: 2
  /// Range: uint32_t [0 – 3]
  /// Description: The extended size of the bright region horizontal cluster.
  /// Set to 0 to disable a cluster shape (square, vertical, horizontal). Just one
  /// bright region can be recognized as spotlight.
  /// Constraints: The extended size should not be over the background zone size.
  /// Suggest to keep default.
  float     horizontal_cluster_increase_size;

  /// reserve[37] -> horizontal_cluster_pct_th
  /// Default Value:
  /// Range:
  /// Description: The threshold percentage of bright region in the cluster zone.
  /// If bright region percentage of the cluster zone is larger than this threshold
  /// and background is dark enough, the flag of bright region cluster will be raised.
  /// The bigger the threshold, the less likely that cluster is detected.
  /// Constraints: If set to 1.0, the cluster is disabled. Suggest to keep default
  float     horizontal_cluster_pct_th;

  /// reserve[38] -> spotlight_detect_cnt_th
  /// Default Value: 3
  /// Range: uint32_t [0 – max]
  /// Description: Threshold to determine how many countinuous spotlight detected
  /// flags need to be raised before declaring final spotlight detected to be true.
  /// Usually the threshold means how many continuous frames of spotlight detected.
  /// Constraints: Setting the value to too large will make final spotlight
  /// detected to be infeasible.
  float     spotlight_detect_cnt_th;

  /// reserve[40] -> haf_roi_enable
  /// Default Value: 0
  /// Range: [0 – 7]
  /// Description: control enableing/disableing touch and face ROI handling in HAF.
  int       haf_roi_enable;

  /// reserve[41] -> haf_roi_h_num
  /// Default Value: 1
  /// Range: 1-6
  /// Description: Number of Horizontal PDAF window
  /// Constraints: PDAF Type 1,2 and 3 currently supports only 1 window
  float     haf_roi_h_num;

  /// reserve[42] -> haf_roi_v_num
  /// Default Value: 1
  /// Range: 1-8
  /// Description: Number of Vertical PDAF window
  /// Constraints: PDAF Type 1,2 and 3 currently supports only 1 window
  float     haf_roi_v_num;

  /// reserve[43] -> haf_center_roi_size
  /// Default Value: 0.2
  /// Range: 0.2-0.3
  /// Description: Default size of PDAF window at the center
  /// Constraints: Value  has to be smaller or equal to haf_corner_roi_size
  float     haf_center_roi_size;

  /// reserve[44] -> haf_corner_roi_size
  /// Default Value: 0.25
  /// Range: 0.2-0.3
  /// Description: Default size of PDAF window at the corner. ROI size will be interpolated between center and corner roi size.
  /// Constraints: Value has to be larger or equal to haf_center_roi_size
  float     haf_corner_roi_size;
  float     haf_roi_delay;

  boolean            enableROIShifting;
  af_roi_shift_type  roi_shift_type;
  int       fine_start_move_buffer;
  float     dof_multiplier;

  boolean   enable_force_scan_end;
  int       enable_passive_scan_timeout;
  int       max_scan_cnt_th;

}af_tuning_haf_extended_adapter_t;

typedef struct _af_tuning_haf_adapter_t
{
    unsigned char                     enable;
    unsigned char                     algo_enable[Q3A_AF_HAF_ALGO_MAX];
    unsigned char                     stats_consume_enable[Q3A_AF_FV_STATS_TYPE_MAX];
    char                              lens_sag_comp_enable;
    char                              hysteresis_comp_enable;
    int                               actuator_shift_comp;
    int                               actuator_hysteresis_comp;
    float                             actuator_sensitivity;
    af_fv_stats_adapter_t             stats_select;
    float                             fine_srch_drop_thres;
    unsigned int                      fine_step_size;
    unsigned int                      max_move_step;
    unsigned int                      max_move_step_buffer;
    unsigned int                      base_frame_delay;
    af_tuning_pdaf_adapter_t          pdaf;
    af_tuning_tof_adapter_t           tof;
    af_tuning_dciaf_adapter_t         dciaf;
    af_tuning_follower_adapter_t      follower;
    af_fine_srch_extension_adapter_t  fine_srch_extension;
    af_tuning_sad_adapter_t           af_sad;
    af_tuning_gyro_adapter_t          af_gyro;
    unsigned int                      scene_change_during_FS;
    int                               face_caf_high_priority_enable;
    af_tuning_haf_extended_adapter_t  haf_extended;
} af_tuning_haf_adapter_t;

typedef struct _af_tuning_vbt_adapter_t
{
    unsigned char       enable;
    float               comp_time;
    float               contamination_pcnt_allow;
} af_tuning_vbt_adapter_t;

//=============================================================================
// af_tuning_smooth_lens_convergence: Smooth lens convergence feature parameters
//
// Variable name: enable
// Description: enable flag for smooth lens convergence
// 3A version:
// Default value: 0
// Data range: 0 or 1
// Constraints: None
// Effect:
//
// Variable name: fudge_factor
// Description: Compensation factor inorder to get the
//              right crop factor
// 3A version:
// Default value: 1
// Data range: [0 1]
// Constraints: None
//
// Variable name: anchor_lens_pos1
// Description: Lens position corresponding to the anchor_distance_mm_1
//              Need to specify the lens position, can refer to Contrast AF
//              tuning settings from single_index_t
// 3A version:
// Default value: lens position specified at SINGLE_10CM_IDX in single_index_t
// Data range: [near_end far_end]
//
// Variable name: anchor_lens_pos2
// Description: Lens position corresponding to the anchor_distance_mm_2
//              Need to specify the lens position, can refer to Contrast AF
//              tuning settings from single_index_t
// 3A version:
// Default value: lens position specified at SINGLE_HYP_F_IDX in single_index_t
// Data range: [near_end far_end]
//
// Variable name: anchor_distance_mm_1
// Description: Physical distance in millimeters at which anchor_lens_pos1
//              corresponds to.
// 3A version:
// Default value: 100
// Data range: [0 3000]
//
// Variable name: anchor_distance_mm_2
// Description: Physical distance in millimeters at which anchor_lens_pos2
//              corresponds to.
// 3A version:
// Default value: 2000
// Data range: [0 3000]
//
// Variable name: focus_converge_step_size_bright_light
// Description: Step size for focus converge in bright light
// 3A version:
// Default value: 8
// Data range: [0 50]
//
// Variable name: fine_search_step_size_bright_light
// Description: Step size for fine search in bright light
// 3A version:
// Default value: 6
// Data range: [0 50]
//
// Variable name: focus_converge_step_size_low_light
// Description: Step size for focus converge in low light
// 3A version:
// Default value: 9
// Data range: [0 50]
//
// Variable name: fine_search_step_size_low_light
// Description: Step size for fine search in low light
// 3A version:
// Default value: 7
// Data range: [0 50]
//=============================================================================
typedef struct _af_tuning_fovc_adapter_t
{
    unsigned char   enable;
    float           fudge_factor;
    int             anchor_lens_pos1;
    int             anchor_lens_pos2;
    int             anchor_distance_mm_1;
    int             anchor_distance_mm_2;
    int             focus_converge_step_size_bright_light;
    int             fine_search_step_size_bright_light;
    int             focus_converge_step_size_low_light;
    int             fine_search_step_size_low_light;
} af_tuning_fovc_adapter_t;

//=============================================================================
// af_tuning_spotlight_af: Spotlight detection/af process feature parameters
//
// Variable name: enable
// Description: enable flag for spotlight af process
// 3A version:
// Default value: 0
// Data range: 0 or 1
// Constraints: None
//
// Variable name: far_flat_th
// Description: flat_threshold for far scene when detecting spotlight condition
// 3A version:
// Default value: 0.55
// Data range: [0 1]
// Constraints: None
// Effect: The lower the easier to be recognized as flat scene
//
// Variable name: marco_flat_th
// Description: flat_threshold for marco scene when detecting spotlight condition
// 3A version:
// Default value: 0.85
// Data range: [0 1]
// Constraints: None
// Effect: The lower the easier to be recognized as flat scene
//
// Variable name: inc_flat_th
// Description: inc_flat_threshold to judge current af position is in increase or flat condition
// 3A version:
// Default value: 1.035
// Data range: [1 3]
// Constraints: None
// Effect: The larger the harder to be recognized as FV increment
//
// Variable name: dec_flat_th
// Description: dec_flat_threshold to judge current af position is in decrease or flat condition
// 3A version:
// Default value: 0.978
// Data range: [0 1]
// Constraints: None
// Effect: The lower the harder to be recognized as FV decrement
//
// Variable name: spotlight_zone_start_h
// Description: Defines the horizontal start region of spotlight zone in 16x16 total regions
// 3A version:
// Default value:4
// Data range: [0 15]
// Constraints: need to be smaller than spotlight_zone_end_h and also smaller than background_zone_start_h
//
// Variable name: spotlight_zone_end_h
// Description: Defines the horizontal end region of spotlight zone in 16x16 total regions,
//              the spotlight zone horizontal end region = spotlight_zone_end_h - 1
// 3A version:
// Default value:12
// Data range: [1 16]
// Constraints: need to be bigger than spotlight_zone_start_h;
//
// Variable name: spotlight_zone_start_v
// Description: Defines the vertical start region of spotlight zone in 16x16 total regions
// 3A version:
// Default value:4
// Data range: [0 15]
// Constraints: need to be smaller than spotlight_zone_end_v
//
// Variable name: spotlight_zone_end_v
// Description: Defines the vertical end region of spotlight zone in 16x16 total regions,
//             the spotlight zone vertical end region = spotlight_zone_end_v - 1
// 3A version:
// Default value:12
// Data range: [1 16]
// Constraints: need to be bigger than spotlight_zone_start_v
//
// Variable name: background_zone_start_h
// Description: Defines the horizontal start region of background zone in 16x16 total regions
// 3A version:
// Default value:2
// Data range: [0 15]
// Constraints: need to be smaller than background_zone_end_h
//
// Variable name: background_zone_end_h
// Description: Defines the horizontal end region of background zone in 16x16 total regions,
//              the background zone horizontal end region = background_zone_end_h - 1
// 3A version:
// Default value:14
// Data range: [1 16]
// Constraints: need to be bigger than background_zone_start_h
//
// Variable name: background_zone_start_v
// Description: Defines the vertical start region of background zone in 16x16 total regions
// 3A version:
// Default value:2
// Data range: [0 15]
// Constraints: need to be smaller than background_zone_end_v
//
// Variable name: background_zone_end_v
// Description: Defines the vertical end region of background zone in 16x16 total regions,
//              the background zone vertical end region = background_zone_end_v - 1
// 3A version:
// Default value:14
// Data range: [1 16]
// Constraints: need to be bigger than background_zone_start_v
//
// Variable name: exposure_index_trigger_th
// Description: if exposure index is smaller than this threshold, the spotlight will not be detected
// 3A version:
// Default value:350
// Data range: [200 400]
// Constraints: None
//
// Variable name: background_luma_avg_th
// Description: If the average luma of background zone is higher than this threshold,
//              the spotlight will not be detected
// 3A version:
// Default value:30
// Data range: [1 255]
// Constraints:
//
// Variable name: region_low_grey_th
// Description: Defines the low grey luma threshold. If luma is smaller than this threshold,
//               it will be recognized as a low grey region, suggest to disable it by default value 255
// 3A version:
// Default value:255
// Data range: [1 255]
// Constraints:
//
// Variable name: region_dark_grey_th
// Description: Defines the dark grey luma threshold. If luma is smaller than this threshold,
//              it will be recognized as a dark grey region, good to keep default value
// 3A version:
// Default value:7
// Data range: [1 255]
// Constraints:
//
// Variable name: background_low_grey_pct_th
// Description: if the low grey percentage is higher than this threshold, the low grey flag will be raised.
//              Suggest to disable it by default
// 3A version:
// Default value:1.0
// Data range: [0 1.0]
// Constraints: If dark grey has been detected first, the low grey will not be used anymore
//
// Variable name: background_dark_grey_pct_th
// Description: if the dark grey percentage is higher than this threshold, the dark grey flag will be raised
// 3A version:
// Default value:0.3
// Data range: [0 1.0]
// Constraints: If dark grey has been detected first, the low grey will not be used anymore
//
// Variable name: bright_region_luma_delta_background_zone
// Description: if regionâ€™s luma is higher than average luma of background zone multiply this parameter, this region will be recognize as a bright region
// 3A version:
// Default value:2.0
// Data range: [0 8.0]
// Constraints:
//
// Variable name: bright_region_luma_delta_spotlight_zone_low_grey
// Description: In low grey condition and cluster is enabled,if regionâ€™s luma is higher than average luma of background zone multiply
//                this parameter, this region will be recognize as a bright region
// 3A version:
// Default value:2.5
// Data range: [0 8.0]
// Constraints:
//
// Variable name: bright_region_luma_delta_spotlight_zone_dark_grey
// Description: In dark grey condition and cluster is enabled,if regionâ€™s luma is higher than average luma of background zone multiply
//                this parameter, this region will be recognize as a bright region
// 3A version:
// Default value:3.0
// Data range: [0 8.0]
// Constraints:
//
// Variable name: square_cluster_inc_size
// Description: The maximum bright region will be the center region and the square cluster size will depends
//              on the parameter, set to 0 will disable this cluster effect
// 3A version:
// Default value:1
// Data range: [0 2]
// Constraints:
//
// Variable name: vertical_cluster_inc_size
// Description: The maximum bright region will be the center region and the vertical cluster size will depends
//              on the parameter, set to 0 will disable this cluster effect
// 3A version:
// Default value:2
// Data range: [0 2]
// Constraints:
//
// Variable name: horizontal_cluster_inc_size
// Description: The maximum bright region will be the center region and the vertical cluster size will depends
//              on the parameter, set to 0 will disable this cluster effect
// 3A version:
// Default value:2
// Data range: [0 2]
// Constraints:
//
// Variable name: square_cluster_pct_th
// Description: If the bright region percentage of square cluster zones is higher this threshold,
//              the flag of square_bright_region_cluster will be raised
// 3A version:
// Default value:0.8
// Data range: [0 1.0]
// Constraints:
//
// Variable name: vertical_cluster_pct_th
// Description: If the bright region percentage of vertical cluster zones is higher this threshold, the flag of
//              vertical_bright_region_cluster will be raised
// 3A version:
// Default value:0.2
// Data range: [0 1.0]
// Constraints:
//
// Variable name: horizontal_cluster_pct_th
// Description: If the bright region percentage of horizontal cluster zones is higher this threshold, the flag of
//              horizontal_bright_region_cluster will be raised
// 3A version:
// Default value:0.2
// Data range: [0 1.0]
// Constraints:
//
// Variable name: spotlight_detect_cnt_th
// Description: Spotlight detect flag threshold. Only if consecutive success spotlight detect over the threshold
//              count will result in the final spotlight detected flag be updated.
// 3A version:
// Default value:3
// Data range: [1 infinity]
// Constraints:
//=============================================================================
typedef struct _af_tuning_spot_light_adapter
{
    unsigned char              enable;
    float                      far_flat_th;
    float                      marco_flat_th;
    float                      inc_flat_th;
    float                      dec_flat_th;
    unsigned int               spotlight_zone_start_h;
    unsigned int               spotlight_zone_end_h;
    unsigned int               spotlight_zone_start_v;
    unsigned int               spotlight_zone_end_v;
    unsigned int               background_zone_start_h;
    unsigned int               background_zone_end_h;
    unsigned int               background_zone_start_v;
    unsigned int               background_zone_end_v;
    int                        exposure_index_trigger_th;
    unsigned int               background_luma_avg_th;
    unsigned int               region_low_grey_th;
    unsigned int               region_low_dark_th;
    float                      background_low_grey_pct_th;
    float                      background_dark_grey_pct_th;
    float                      bright_region_luma_delta_background_zone;
    float                      bright_region_luma_delta_spotlight_zone_low_grey;
    float                      bright_region_luma_delta_spotlight_zone_dark_grey;
    unsigned int               square_cluster_inc_size;
    unsigned int               vertical_cluster_inc_size;
    unsigned int               horizontal_cluster_inc_size;
    float                      square_cluster_pct_th;
    float                      vertical_cluster_pct_th;
    float                      horizontal_cluster_pct_th;
    unsigned int               spotlight_detect_cnt_th;
} af_tuning_spot_light_adapter_t;

typedef struct _af_tuning_continuous_adapter
{
    unsigned char                   enable;
    unsigned char                   scene_change_detection_ratio;
    float                           panning_stable_fv_change_trigger;
    float                           panning_stable_fvavg_to_fv_change_trigger;
    unsigned short                  panning_unstable_trigger_cnt;
    unsigned short                  panning_stable_trigger_cnt;
    unsigned long                   downhill_allowance;
    unsigned short                  uphill_allowance;
    unsigned short                  base_frame_delay;
    unsigned short                  scene_change_luma_threshold;
    unsigned short                  luma_settled_threshold;
    float                           noise_level_th;
    unsigned short                  search_step_size;
    af_algo_adapter_t               init_search_type;
    af_algo_adapter_t               search_type;
    unsigned short                  low_light_wait;
    unsigned short                  max_indecision_cnt;
    float                           flat_fv_confidence_level;
    af_tuning_sad_adapter_t         af_sad;
    af_tuning_gyro_adapter_t        af_gyro;
    af_tuning_chroma_adapter_t      af_chroma;
    unsigned int                    af_caf_trigger_after_taf;
    af_tuning_spot_light_adapter_t  af_spot_light;
} af_tuning_continuous_adapter_t;

typedef struct _af_tuning_exhaustive_adapter
{
    unsigned short    num_gross_steps_between_stat_points;
    unsigned short    num_fine_steps_between_stat_points;
    unsigned short    num_fine_search_points;
    unsigned short    downhill_allowance;
    unsigned short    uphill_allowance;
    unsigned short    base_frame_delay;
    unsigned short    coarse_frame_delay;
    unsigned short    fine_frame_delay;
    unsigned short    coarse_to_fine_frame_delay;
    float             noise_level_th;
    float             flat_fv_confidence_level;
    float             climb_ratio_th;
    int               low_light_luma_th;
    int               enable_multiwindow;
    float             gain_thresh;
} af_tuning_exhaustive_adapter_t;

typedef struct _af_tuning_fullsweep_adapter_t
{
    unsigned short    num_steps_between_stat_points;
    unsigned short    frame_delay_inf;
    unsigned short    frame_delay_norm;
    unsigned short    frame_delay_final;
    unsigned short    PAAF_enable;
} af_tuning_fullsweep_adapter_t;

typedef struct _af_shake_resistant_adapter
{
    unsigned char   enable;
    float           max_gain;
    unsigned char   min_frame_luma;
    float           tradeoff_ratio;
    unsigned char   toggle_frame_skip;
} af_shake_resistant_adapter_t;

typedef struct _af_motion_sensor_adapter
{
    float af_gyro_trigger;
    float af_accelerometer_trigger;
    float af_magnetometer_trigger;
    float af_dis_motion_vector_trigger;
} af_motion_sensor_adapter_t;

typedef struct _af_fd_priority_caf_adapter
{
    float   pos_change_th;
    float   pos_stable_th_hi;
    float   pos_stable_th_low;
    float   size_change_th;
    float   old_new_size_diff_th;
    int     stable_count_size;
    int     stable_count_pos;
    int     no_face_wait_th;
    int     fps_adjustment_th;
} af_fd_priority_caf_adapter_t;

//=============================================================================
// af_tuning_multiwindow_t: AF tuning parameters specific to
//  Multiwindow feature
//
// Variable name: enable
// Description: enable flag for multiwindow feature
// 3A version:
// Default value: 0
// Data range: 0 or 1
// Constraints: None
// Effect:
//
// Variable name: h_clip_ratio_normal_light
// AF window horizontal size in terms of ratio to the whole image. For the
// same example above, Horizontal Clip Ratio is 500/1000=0.5.
// 3A version:
// Default value: 0.5
// Data range: 0 - 1
// Constraints:
// Effect:
//
// Variable name: v_clip_ratio_normal_light
// AF window vertical size in terms of ratio to the whole image. For the
// same example above, Vertical Clip Ratio is 500/1000=0.5.
// 3A version:
// Default value: 0.5
// Data range: 0 -1
// Constraints:
// Effect:
//
// Variable name: h_clip_ratio_low_light
// AF window horizontal size in terms of ratio to the whole image. For the
// same example above, Horizontal Clip Ratio is 500/1000=0.5.
// 3A version:
// Default value: 0.5
// Data range: 0 - 1
// Constraints:
// Effect:
//
// Variable name: v_clip_ratio_low_light
// AF window vertical size in terms of ratio to the whole image. For the
// same example above, Vertical Clip Ratio is 500/1000=0.5.
// 3A version:
// Default value: 0.5
// Data range: 0 - 1
// Constraints:
// Effect:
//
// Variable name: h_num_default
// Default horizontal number of grids in an ROI.
// 3A version:
// Default value: 6
// Data range: [1 18]
// Constraints:
// Effect:
//
// Variable name: v_num_default
// Default vertical number of grids in an ROI.
// 3A version:
// Default value: 6
// Data range: [1 14]
// Constraints:
// Effect:
//
// Variable name: h_num_windows
// Default horizontal number of windows in an ROI.
// 3A version:
// Default value: 5
// Data range: [1 h_num_default]
// Constraints:
// Effect:
//
// Variable name: v_num_windows
// Default vertical number of windows in an ROI.
// 3A version:
// Default value: 5
// Data range: [1 v_num_default]
// Constraints:
// Effect:
//=============================================================================
typedef struct _af_tuning_multiwindow_adapter_t
{
    int     enable;
    float   h_clip_ratio_normal_light;
    float   v_clip_ratio_normal_light;
    float   h_clip_ratio_low_light;
    float   v_clip_ratio_low_light;
    int     h_num_default;
    int     v_num_default;
    int     h_num_windows;
    int     v_num_windows;
} af_tuning_multiwindow_adapter_t;

//=============================================================================
// af_focus_mode_adapter_t: Enum for List of focus modes supported
//
//  Q3A_AF_FOCUS_MODE_DEFAULT: Default focus mode
//  Q3A_AF_FOCUS_MODE_MACRO: Macro  focus mode
//  Q3A_AF_FOCUS_MODE_NORMAL: Normal focus mode
//  Q3A_AF_FOCUS_MODE_CAF: Continuous focus mode
//  Q3A_AF_FOCUS_MODE_MANUAL: Manual focus mode
//=============================================================================
typedef enum
{
    Q3A_AF_FOCUS_MODE_DEFAULT,
    Q3A_AF_FOCUS_MODE_MACRO,
    Q3A_AF_FOCUS_MODE_NORMAL,
    Q3A_AF_FOCUS_MODE_CAF,
    Q3A_AF_FOCUS_MODE_MANUAL,
    Q3A_AF_FOCUS_MODE_MAX
} af_focus_mode_adapter_t;

/**
* Structure defining search limits applicable to
* each focus mode.
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
//=============================================================================
// af_tuning_focus_mode_search_limit_t: AF tuning parameters defining search limits applicable to each focus mode
//
// Variable name: enable
// Description: enable/disable flag for the feature
// Default values: 0
// Range: [0,1]
//
//  Variable name: near_end
//  Description: Near end position for the mode
//  Default value: position_near_end.
//  Data range: [position_near_end position_far_end]
//
//  Variable name: far_end
//  Description: Near end position for the mode
//  Default value: position_far_end.
//  Data range: [position_near_end position_far_end]
//
//  Variable name: default_pos
//  Description: default position for the mode
//  Default value: position_default_in_normal.
//  Data range: [position_near_end position_far_end]
//=============================================================================
typedef struct af_tuning_focus_mode_search_limit_adapter_t
{
    int          enable;
    unsigned int near_end;
    unsigned int far_end;
    unsigned int default_pos;
} af_tuning_focus_mode_search_limit_adapter_t;

/**
* Structure defining advance fd tuning
* inculding robust fd-af and lost face enable
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
typedef struct af_adv_fd_tuning {
  unsigned char     robust_fd_enable;
  unsigned int      noface_cnt_thr;
  unsigned int      stable_cnt_thr;
  float             fd_roi_pos_change_th;
  unsigned char     face_lost_trigger_enable;
  unsigned int      face_lost_stable_th;
}af_adv_fd_tuning;

/**
* Structure defining spot light tuning
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
typedef struct af_spot_light_filter_tuning {
  int                   enable;
  float                 iir_2tap_filter[10];
  float                 iir_4tap_filter[10];
}af_spot_light_filter_tuning;

/**
* Structure defining spot light tuning
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
typedef struct af_face_extension_tuning {
  int                   af_roi_face_extension;
  float                 af_roi_face_scale_x;
  float                 af_roi_face_scale_y;
  float                 af_roi_face_symmetric_x;
  float                 af_roi_face_symmetric_y;
}af_face_extension_tuning;


/**
* Structure defining search limits applicable to
* each focus mode.
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
typedef struct _af_chromatix_search_limit {
  unsigned int near_end;
  unsigned int far_end;
  unsigned int default_pos;
} af_chromatix_search_limit_wrapper_t;

/*=============================================================================
* af_chromatix_step_back_wrapper_t: AF tuning parameters specific to step back
*============================================================================*/
typedef struct _af_chromatix_step_back {
  boolean enable;
  int     start_inf;
  int     start_mac;
  int     reverse_inf;
  int     reverse_mac;
} af_chromatix_step_back_wrapper_t;

/**
* Struct which defines mapping
* between luma and weight of a
* region
*/
typedef struct _luma_weight_entry {
  float luma;
  float weight;
} luma_weight_entry;

/**
* Table to contain mapping entries
* of luma and corresponding weight
*/
typedef struct _luma_weight_table {
  int  number_of_entries;
  luma_weight_entry entry[AF_LUMA_TABLE_SIZE];
} luma_weight_table;

/**
* Floating af weight table
*/
typedef struct _region_weight_table {
  int  window_size;
  float weight[AF_GRID_SIZE_MAX];
} region_weight_table;

/**
* Af weight tuning params
*/
typedef struct _af_weight_tuning{
  boolean is_af_region_weight_enable;
  boolean is_af_Luma_weight_enable;
  region_weight_table region_weight_table;
  luma_weight_table luma_weight_table;
} af_weight_tuning;

/**
* Structure defining parameters which are
* not a part of chromatix but needed by algo
* Note: This structure is to be moved to chromatix in the next
* chromatix revision.
*/
typedef struct _af_algo_params
{
  // weight params used by multi-window algorithm
  // to compensate brightness or position's influence
  af_weight_tuning  af_weight_tuning;
  int               firstrun_spotlight_detect_cnt_th;
} af_algo_params;

/*=============================================================================
* af_chromatix_scan_range_wrapper_t: AF tuning parameters specific to scan range
*============================================================================*/
typedef struct _af_chromatix_scan_range {
  float fratio_min;
  float fratio_max;
  float first_prev_y1;
  float first_next_y1;
  float first_prev_y2;
  float first_next_y2;
  float second_prev_y1;
  float second_next_y1;
  float second_prev_y2;
  float second_next_y2;
} af_chromatix_scan_range_wrapper_t;

/*=============================================================================
* af_chromatix_flat_detection_wrapper_t: AF tuning parameters about flat detection
*============================================================================*/
typedef struct _af_chromatix_flat_detection {
  boolean enable;
  float   threshold_bv;
} af_chromatix_flat_detection_wrapper_t;

/*=============================================================================
* af_chromatix_flat_threshold_wrapper_t: AF tuning parameters about flat threshold
*============================================================================*/
typedef struct _af_chromatix_flat_threshold {
  float increase;
  float decrease;
} af_chromatix_flat_threshold_wrapper_t;

/*=============================================================================
* af_chromatix_hys_offset_wrapper_t: AF tuning parameters about hysteresis offset
*============================================================================*/
typedef struct _af_chromatix_hys_offset {
  boolean enable;
  int     offset;
} af_chromatix_hys_offset_wrapper_t;

/*=============================================================================
* af_chromatix_fd_roi_wrapper_t: AF tuning parameters about FD ROI
*============================================================================*/
typedef struct _af_chromatix_fd_roi {
  boolean enable;
  float   ratio;
} af_chromatix_fd_roi_wrapper_t;

/*=============================================================================
* af_chromatix_outdoor_roi_wrapper_t: AF tuning parameters about Outdoor ROI
*============================================================================*/
typedef struct _af_chromatix_outdoor_roi {
  boolean enable;
  float   ratio_x;
  float   ratio_y;
} af_chromatix_outdoor_roi_wrapper_t;

/*=============================================================================
* af_chromatix_scene_detection_bv_wrapper_t: AF tuning parameters about scene change detection by BV
*============================================================================*/
typedef struct _af_chromatix_scene_detection_bv {
  boolean enable;
  float   outdoor_unstable_th;
  float   outdoor_stable_th;
  int     outdoor_trigger_cnt_th;
  float   low_unstable_th;
  float   low_stable_th;
  int     low_trigger_cnt_th;
  float   normal_unstable_th;
  float   normal_stable_th;
  int     normal_trigger_cnt_th;
} af_chromatix_scene_detection_bv_wrapper_t;

typedef struct _af_tuning_algo_extended_adapter_t
{
  af_chromatix_search_limit_wrapper_t         search_limit[AF_FOCUS_MODE_MAX];
  af_chromatix_step_back_wrapper_t            step_back;
  af_chromatix_scan_range_wrapper_t           scan_range;
  af_chromatix_flat_detection_wrapper_t       flat_detection;
  af_chromatix_flat_threshold_wrapper_t       flat_threshold;
  af_chromatix_hys_offset_wrapper_t           hys_offset;
  af_chromatix_fd_roi_wrapper_t               fd_roi;
  af_chromatix_outdoor_roi_wrapper_t          outdoor_roi;
  af_chromatix_scene_detection_bv_wrapper_t   scene_detect_bv;
  boolean                             en_ignore_first_invalid_frame;
  boolean                             en_detect_peak_in_decrease;
  boolean                             en_frame_skip_goto_peak;
  boolean                             en_fine_search_forcely;
  boolean                             en_simple_prescan;
  boolean                             en_optimize_peak;

  /// please refer to af_tuning_focus_mode_search_limit_wrapper_t.
  af_adv_fd_tuning      fd_adv_tuning;
  af_stats_use_type                   stats_comp_type;
  boolean                             hv_stats_ratio_enable;
  float                               gravity_x_land_thres;
  float                               gravity_y_land_thres;
  float                               gravity_x_port_thres;
  float                               gravity_y_port_thres;
  float                               hv_weight;
  af_spot_light_filter_tuning   spot_light_filter;
  af_face_extension_tuning      face_extension_tuning;
  boolean                       face_trig_priority_enable;
  boolean                       exp_comp_enable;
  af_algo_params        *af_algo_params;
} af_tuning_algo_extended_adapter_t;

typedef struct _af_tuning_algo_adapter
{
    af_algo_adapter_t                           af_process_type;
    unsigned short                              position_near_end;
    unsigned short                              position_default_in_macro;
    unsigned short                              position_boundary;
    unsigned short                              position_default_in_normal;
    unsigned short                              position_far_end;
    unsigned short                              position_normal_hyperfocal;
    unsigned short                              position_macro_rgn;
    unsigned short                              undershoot_protect;
    unsigned short                              undershoot_adjust;
    float                                       min_max_ratio_th;
    int                                         led_af_assist_enable;
    long                                        led_af_assist_trigger_idx;
    int                                         lens_reset_frame_skip_cnt;
    float                                       low_light_gain_th;
    float                                       base_delay_adj_th;
    af_tuning_continuous_adapter_t              af_cont;
    af_tuning_exhaustive_adapter_t              af_exh;
    af_tuning_fullsweep_adapter_t               af_full;
    af_tuning_sp_adapter_t                      af_sp;
    af_tuning_single_adapter_t                  af_single;
    af_shake_resistant_adapter_t                af_shake_resistant;
    af_motion_sensor_adapter_t                  af_motion_sensor;
    af_fd_priority_caf_adapter_t                fd_prio;
    af_tuning_lens_sag_comp_adapter_t           lens_sag_comp;
    af_tuning_haf_adapter_t                     af_haf;
    af_tuning_vbt_adapter_t                     vbt;
    af_tuning_fovc_adapter_t                    fovc;
    af_tuning_multiwindow_adapter_t             af_multiwindow;
    af_tuning_focus_mode_search_limit_adapter_t search_limit[Q3A_AF_FOCUS_MODE_MAX];
    af_tuning_algo_extended_adapter_t           algo_extended;
} af_tuning_algo_adapter_t;

typedef struct _af_vfe_config_adapter
{
    unsigned short  fv_min;
    unsigned short  max_h_num;
    unsigned short  max_v_num;
    unsigned short  max_block_width;
    unsigned short  max_block_height;
    unsigned short  min_block_width;
    unsigned short  min_block_height;
    float           h_offset_ratio_normal_light;
    float           v_offset_ratio_normal_light;
    float           h_clip_ratio_normal_light;
    float           v_clip_ratio_normal_light;
    float           h_offset_ratio_low_light;
    float           v_offset_ratio_low_light;
    float           h_clip_ratio_low_light;
    float           v_clip_ratio_low_light;
    float           touch_scaling_factor_normal_light;
    float           touch_scaling_factor_low_light;
    int             bf_scale_factor;
    unsigned short  h_num_default;
    unsigned short  v_num_default;
} af_vfe_config_adapter_t;

typedef struct _af_vfe_legacy_hpf_adapter
{
    char      a00;
    char      a02;
    char      a04;
    char      a20;
    char      a21;
    char      a22;
    char      a23;
    char      a24;
} af_vfe_legacy_hpf_adapter_t;

typedef struct _af_vfe_hpf_adapter
{
    af_vfe_legacy_hpf_adapter_t   af_hpf;
    int                           bf_hpf_2x5[MAX_HPF_2x5_BUFF_SIZE];
    int                           bf_hpf_2x11[MAX_HPF_2x11_BUFF_SIZE];
} af_vfe_hpf_adapter_t;

typedef struct _af_vfe_sw_fir_hpf_adapter_t
{
    int                    a[FILTER_SW_LENGTH_FIR];
    double                 fv_min_hi;
    double                 fv_min_lo;
    unsigned long int      coeff_length;
} af_vfe_sw_fir_hpf_adapter_t;

typedef struct _af_vfe_sw_iir_hpf_adapter_t
{
    double                 a[FILTER_SW_LENGTH_IIR];
    double                 b[FILTER_SW_LENGTH_IIR];
    double                 fv_min_hi;
    double                 fv_min_lo;
    unsigned long int      coeff_length;
} af_vfe_sw_iir_hpf_adapter_t;

typedef struct af_vfe_sw_hpf_adapter_t
{
    unsigned short                filter_type;    /* af_sw_filter_type */
    af_vfe_sw_fir_hpf_adapter_t   fir;
    af_vfe_sw_iir_hpf_adapter_t   iir;
} af_vfe_sw_hpf_adapter_t;

typedef struct _baf_tuning_gamma_adapter_t
{
    unsigned char                         ch_sel;
    float                                 Y_coeffs[MAX_BAF_GAMMA_Y_ENTRY];
    unsigned char                         g_sel;
    unsigned char                         LUT_enable;
    int                                   gamma_LUT[MAX_BAF_GAMMA_LUT_ENTRY];
} baf_tuning_gamma_adapter_t;

typedef struct _baf_tuning_gamma_param_adapter_t
{
    af_baf_tuning_preset_adapter_t      preset_id;
    baf_tuning_gamma_adapter_t          gamma_custom;
} baf_tuning_gamma_param_adapter_t;

typedef struct _baf_tuning_coring_adapter_t
{
    unsigned int                          x_index[MAX_BAF_CORING_ENTRY];
    int                                   threshold;
    int                                   gain;
} baf_tuning_coring_adapter_t;

typedef struct _baf_tuning_HV_filter_adapter_t
{
    unsigned char                         fir_en;
    unsigned char                         iir_en;
    int                                   fir[MAX_BAF_FIR_ENTRY];
    float                                 iir[MAX_BAF_IIR_ENTRY];
    int                                   shift_bits;
    unsigned char                         scaler_sel;
} baf_tuning_HV_filter_adapter_t;

typedef struct _baf_HV_filter_adapter_t
{
    af_baf_tuning_preset_adapter_t                preset_id;
    baf_tuning_coring_adapter_t                   coring;
    baf_tuning_HV_filter_adapter_t                filter_custom;
} baf_HV_filter_adapter_t;

typedef struct _baf_tuning_scaler_adapter_t
{
    int                                   M;
    int                                   N;
} baf_tuning_scaler_adapter_t;

typedef struct _baf_tuning_scaler_coefficient_adapter_t
{
    int                                   a;
    int                                   b;
} baf_tuning_scaler_coefficient_adapter_t;

typedef struct _baf_tuning_filter_adapter_t
{
    baf_HV_filter_adapter_t                       HV_filter[MAX_BAF_FILTER];
    baf_HV_filter_adapter_t                       HV_filter_2nd_tier[MAX_BAF_FILTER_2ND_TIER];
    baf_tuning_scaler_adapter_t                   scaler;
    baf_tuning_scaler_coefficient_adapter_t       coefficient;
} baf_tuning_filter_adapter_t;

typedef struct _baf_tuning_roi_adapter_t
{
    unsigned char                         flag;
    unsigned int                          x;
    unsigned int                          y;
    unsigned int                          width;
    unsigned int                          height;
} baf_tuning_roi_adapter_t;

typedef struct _baf_tuning_roi_param_adapter_t
{
    af_baf_roi_pattern_adapter_t          preset_id;
    float                                 grid_size_h;
    float                                 grid_size_v;
    float                                 grid_gap_h;
    float                                 grid_gap_v;
    int                                   num_ROI;
    baf_tuning_roi_adapter_t              ROI_custom[MAX_BAF_ROI_NUM];
} baf_tuning_roi_param_adapter_t;

typedef struct _af_tuning_baf_adapter_t
{
    baf_tuning_roi_param_adapter_t                roi_config[Q3A_AF_SCENE_TYPE_MAX];
    baf_tuning_gamma_param_adapter_t              gamma[Q3A_AF_SCENE_TYPE_MAX];
    baf_tuning_filter_adapter_t                   filter[Q3A_AF_SCENE_TYPE_MAX];
} af_tuning_baf_adapter_t;

typedef struct _af_tuning_vfe_adapter
{
    unsigned short   fv_metric;
    af_vfe_config_adapter_t  config;
    af_vfe_hpf_adapter_t     hpf_default;
    af_vfe_hpf_adapter_t     hpf_face;
    af_vfe_hpf_adapter_t     hpf_low_light;
} af_tuning_vfe_adapter_t;

typedef struct _af_tuning_sw_stats_adapter
{
    af_vfe_sw_hpf_adapter_t   sw_hpf_default;
    af_vfe_sw_hpf_adapter_t   sw_hpf_face;
    af_vfe_sw_hpf_adapter_t   sw_hpf_lowlight;
    float                     sw_fv_min_lux_trig_hi;
    float                     sw_fv_min_lux_trig_lo;
} af_tuning_sw_stats_adapter_t;

typedef struct _af_header_info_adapter
{
    unsigned short      header_version;
    char                module_name[MAX_ACT_MOD_NAME_SIZE];
    char                actuator_name[MAX_ACT_NAME_SIZE];
    af_cam_name_adapter cam_name;
} af_header_info_adapter_t;


typedef struct _af_algo_tune_parms_adapter
{
    af_header_info_adapter_t      af_header_info;
    af_tuning_algo_adapter_t      af_algo;
    af_tuning_sw_stats_adapter_t  af_swaf_config;
    af_tuning_vfe_adapter_t       af_vfe[MAX_AF_KERNEL_NUM];
    af_tuning_baf_adapter_t       af_baf;
} af_algo_tune_parms_adapter_t;

//=============================================================================
//=============================================================================
//  AF related params end
//=============================================================================
//=============================================================================

//=============================================================================
//=============================================================================
//  AEC related params start
//=============================================================================
//=============================================================================
#define NUM_AEC_STATS                     16
#define MAX_EXPOSURE_TABLE_SIZE           700
#define MAX_SNAPSHOT_LUT_SIZE             10
#define MAX_AEC_TRIGGER_ZONE              6
#define MAX_LED_MIX_LEVEL                 16
#define MAX_LED_POWER_LEVEL               6
#define MAX_CALIB_NUM                     12
#define CALIB_RANGE_NUM                   2
#define MAX_EV_ARRAY_SIZE                 49
/* Lux index temporal filter factor */
#define LUX_INDEX_TEMPORAL_FILTER_FACTOR  0.50f
/* Number of table entries for dual camera calibration compensationratios */
#define AEC_DUAL_CAM_COMP_TABLE_SIZE      6

typedef struct
{
    unsigned short gain;                        // Gain
    unsigned int   line_count;                  // Line Count
} exposure_entry_adapter;

typedef struct
{
    unsigned short          valid_entries;                              // Number of entries in the exposure table with digital EV feature disabled
    int                     aec_enable_digital_gain_for_EV_lowlight;    // Flag to enable\disable the digital EV feature
    unsigned short          total_entries_with_digital_gain_for_EV;     // Number of entries in the exposure table with digital EV feature enabled
    unsigned short          fix_fps_aec_table_index;                    // Index upto which the frame rate is fixed (linear AFR kicks in beyond this point)
    exposure_entry_adapter  exposure_entries[MAX_EXPOSURE_TABLE_SIZE];  // Exposure table with gain and line count entries
} aec_exposure_table_adapter;

/**
* Lux trigger definition
*/
typedef struct
{
    int start;
    int end;
} aec_lux_trigger_adapter;

typedef enum
{
    Q3A_AEC_BAYER_CHNL_R,
    Q3A_AEC_BAYER_CHNL_G,
    Q3A_AEC_BAYER_CHNL_B,
    Q3A_AEC_BAYER_CHNL_MAX
} aec_bayer_channel_adapter_t;

/**
* Luma target tuning
*/
typedef struct
{
    unsigned int           luma_target;           // Luma target
} aec_luma_target_triggered_adapter;

typedef struct
{
    int                                num_zones;                                  // Number of zones actually used
    aec_lux_trigger_adapter            triggers[MAX_AEC_TRIGGER_ZONE - 1];         // Trigger point to enter the next zone
    aec_luma_target_triggered_adapter  triggered_params[MAX_AEC_TRIGGER_ZONE];     // Parameters that can be tuned per-zone
} aec_luma_target_adapter;

/**
* This structure defines the metering tables
*/
typedef struct
{
    float    AEC_weight_center_weighted[NUM_AEC_STATS][NUM_AEC_STATS];          // Center-weighted metering table (0x304)
    float    AEC_weight_spot_metering[NUM_AEC_STATS][NUM_AEC_STATS];            // Spot-metering table (0x304)
} aec_metering_table_adapter;

//=============================================================================
// Feature name: AEC index compensation
// Applicable operation mode:  Snapshot, Preview and Camcorder
// When this feature is enabled, AEC output normalized AWB exposure index and
// lux index, regardless of the sensor resolution mode.
//
// Variable name: awb_exp_idx_comp_enable
// Flag to enable the AWB exposure index compensation feature
// Default value: 0
// Range: 0 and 1
//
// Variable name: lux_idx_comp_enable
// Flag to enable the lux index compensation feature
// Default value: 0
// Range: 0 and 1
//
// Variable name: ref_sensitivity
// Reference sensitivity. A must tune value if any of the above enable flags are set.
// The value is the sensitivity of the first entry of the preview mode exposure table.
// The sensitivity is calculated as exposure time (in second) multipled by gain, and
// further multipled by binning factor (most of the time, binning factor is 1).
// Default value: 0.00001
// Range : [0 1]
//=============================================================================
typedef struct
{
    int      awb_exp_idx_comp_enable;           // Flag to enable\disable the AWB exposure index compensation feature
    int      lux_idx_comp_enable;               // Flag to enable\disable the lux index compensation feature
    float    ref_sensitivity;                   // reference sensitivity value
} aec_idx_comp_adapter;

//=============================================================================
// Feature name: AEC EV compensation
// Applicable operation mode:  Snapshot, Preview and Camcorder
// This structure enables tuning of the EV tables for both regular EV compensation
// and bracketing use cases.
//
// Variable name: steps_per_ev
// This variable defines the number of steps for each EV in the EV tables
// Due to s/w limitation, this variable needs to be changed simultaneously with
// the hard-coded values inside stats module code (aec_module.c) on LA.
// Default value: 6
// Range: 1 to 12 (integer)
//
// Variable name: min_ev_val
// The minimal EV value in the ev table. It is recommended to NOT change normally.
// Also this variable needs to be changed simultaneously with the hard-coded
// values inside stats module code (aec_module.c) on LA.
// Default value: -2
// Range: -2 to 0
//
// Variable name: max_ev_val
// The maximal EV value in the ev table. It is recommended to NOT change normally.
// Also this variable needs to be changed simultaneously with the hard-coded
// values inside stats module code (aec_module.c) on LA.
// Default value: 2
// Range: 0 to 2
//
// Variable name: ev_comp_table
// EV table that can be tuned for EV compensation in AEC auto mode. The EV table
// normally starts from -2, and ends at +2. With steps_per_ev set at 6, the table
// will need 25 entries. The maximum steps_per_ev is 12, which requires 49 entries
// in the EV table.
// Default value: {0.2500f, 0.2806f, 0.3150f, 0.3536f, 0.3969f, 0.4454f, 0.5000f,
//   0.5612f, 0.6299f, 0.7071f, 0.7937f, 0.8909f, 1.0000f, 1.1225f, 1.2599f, 1.4142f,
//   1.5874f, 1.7818f, 2.0000f, 2.2449f, 2.5198f, 2.8284f, 3.1748f, 3.5636f, 4.0000f}
// Range : [0.0f, 10.0f] for each entry
//
// Variable name: bracketing_table
// It is also an EV table, except that it is used in bracketing exposure calculation
// which is the multi-frame HDR use case. It can be tuned independently of regular
// AEC EV compensation.
// Default value: {0.2500f, 0.2806f, 0.3150f, 0.3536f, 0.3969f, 0.4454f, 0.5000f,
//   0.5612f, 0.6299f, 0.7071f, 0.7937f, 0.8909f, 1.0000f, 1.1225f, 1.2599f, 1.4142f,
//   1.5874f, 1.7818f, 2.0000f, 2.2449f, 2.5198f, 2.8284f, 3.1748f, 3.5636f, 4.0000f}
// Range : [0.0f, 10.0f] for each entry
//=============================================================================
typedef struct
{
    unsigned short steps_per_ev;
    short          min_ev_val;
    short          max_ev_val;
    float          ev_comp_table[MAX_EV_ARRAY_SIZE];
    float          bracketing_table[MAX_EV_ARRAY_SIZE];
} aec_ev_table_adapter;

//=============================================================================
// Feature name: AEC Luma Weight
// Applicable operation mode:  Snapshot, Preview and Camcorder
// Luma weight for calculating the luma is exposed. Also a compensation feature
// is introduced to allow AEC to compensate the r/g/b weights for luma
// calculation based on the current white balance decision. In addition, since
// WB gains are OTP corrected, the additional benefit of this feature is to also
// compensate luma variation caused by module-to-module variation as well.
//
// Variable name: r_weight
// The default weight used to multiply red channel to calculate luma. If color and
// sensor compensation is not enabled, this is the final red weight used by AEC.
// Default value: 0.2988
// Range: [0, 1]
//
// Variable name: g_weight
// The default weight used to multiply green channel to calculate luma. If color and
// sensor compensation is not enabled, this is the final green weight used by AEC.
// Default value: 0.5869
// Range: [0, 1]
//
// Variable name: b_weight
// The default weight used to multiply blue channel to calculate luma. If color and
// sensor compensation is not enabled, this is the final blue weight used by AEC.
// Default value: 0.1137
// Range: [0, 1]
//
// Variable name: color_and_sensor_comp_enable
// Enabling compensation for luma calculation under different lighting as well as
// different sensor modules. If this flag is set, "r_weight", "g_weight" and "b_weight"
// are modified dynamically based on the difference between current white balance gains
// and reference white balance gains as defined below.
// Default value: 0
// Range: 0 and 1
//
// Variable name: ref_rGain
// The reference white balance gain used to calculate the adjustment factor on
// r_weight. It is recommended to set this value to D50 red gain of the golden module.
// If feature is enabled, this value is must tune.
// Default value: 1.8
// Range: [1, 3]
//
// Variable name: ref_gGain
// The reference white balance gain used to calculate the adjustment factor on
// g_weight. It is recommended to set this value to D50 green gain of the golden module.
// If feature is enabled, this value is must tune.
// Default value: 1.0
// Range: [1, 3]
//
// Variable name: ref_bGain
// The reference white balance gain used to calculate the adjustment factor on
// b_weight. It is recommended to set this value to D50 blue gain of the golden module.
// If feature is enabled, this value is must tune.
// Default value: 1.5
// Range: [1, 3]
//=============================================================================
typedef struct
{
    float   r_weight;                          // Red weight for luma calculation
    float   g_weight;                          // Green weight for luma calculation
    float   b_weight;                          // Blue weight for luma calculation
    int     color_and_sensor_comp_enable;      // Enable color and sensor module compensation for luma calculation
    float   ref_rGain;                         // Reference red gain for golden module (recommend D50)
    float   ref_gGain;                         // Reference green gain for golden module (recommend D50)
    float   ref_bGain;                         // Reference blue gain for golden module (recommend D50)
} aec_luma_weight_adapter;

/** aec_aggressive_tuning:
*    @startup_frame_skip: Startup frame skip
*    @frame_skip: Frame skip
*    @luma_tol_percentage: Luma tolerance percentage
*    @max_frame_converge: Max frame convergence threshold for aec settled in instant capture
*    @min_settled_frame: Min settled frames needed to declare AEC is settled in instant capture
*
* Aggressive AEC tuning parameters
**/
typedef struct
{
    uint32 startup_frame_skip;
    uint32 frame_skip;
    float luma_tol_percentage;
    uint32 max_frame_converge;
    uint32 min_settled_frame;
    uint32 min_awb_settle_count;
} aec_aggressive_tuning;

/* Fast AEC tuning type */
typedef struct
{
    // Fast AEC start index
    int     start_index;                // Number of frames to skip after first AEC update during fast AEC
    int     startup_frame_skip;         // Number of frames to skip between subsequent exposure updates
    int     frame_skip;                 // Number of successive frames for which AEC has to be settled in order to declare AEC as 'settled'
    int     luma_settled_count;         // Flag that determines if digital gain is to be used during fast AEC
    boolean use_digital_gain;
} aec_fast_aec_tuning;

/** aec_lls_tuning:
*  converted tuning parameters for Low light shot
*
*  enable:
*  luxindex_thr_*lux: Lux index threshold for * lux setting
*  req_exp_time_*lux: Required exp time for * lux setting
*/
typedef struct {
    uint32                               enable;
    float                                luxindex_thr_500lux;
    float                                luxindex_thr_200lux;
    float                                luxindex_thr_100lux;
    float                                luxindex_thr_050lux;
    float                                luxindex_thr_010lux;
    float                                req_exp_time_500lux;
    float                                req_exp_time_200lux;
    float                                req_exp_time_100lux;
    float                                req_exp_time_050lux;
    float                                req_exp_time_010lux;
    float                                req_extend_max_gain;
} aec_lls_tuning;

//=============================================================================
// Variable name: pct_based_luma_tolerance_enable
// Set this flag to enable AEC algorithm to use percentage based luma tolerance.
// It provides more consistency in AEC for lowlight and EV-minus use cases. This
// is required if ADRC is enabled.
// Default value: 0
// Range: 0 and 1
//
// Variable name: luma_tolerance_pct
// Luma tolerance to be used in all preview/camcorder AEC convergence.
// The smaller the luma tolerance, the more consistent AEC will be. However,
// chances of oscillation could also be higher.
// Suggest NOT tune unless absolutely necessary.
// Default value: 0.03
// Data range: (0, 1]
//
// Variable name: iso_quantization_enable
// Set this flag to enable standardized ISO value reporting from AEC.
// Gains sent to sensor are not changed.
// Default value: 1
// Range: 0 and 1
//
// Variable name: lux_index_ref
// Reference luma target for calculating lux index as well as awb exposure index.
// No need to tune.
// Default value: 50
// Range : [0 255]
//
// Variable name: aec_subsampling_factor
// Subsampling factor for AEC to skip bayer stats for the purpose of power saving
// Default value: 4
// Range: 1 to 8
//=============================================================================
typedef struct
{
    unsigned int             aec_start_index;                     // Exposure index to be used upon camera startup
    unsigned short           luma_tolerance;                      // Tolerance range to deem AEC as settled
    int                      pct_based_luma_tolerance_enable;     // Enable to switch algorithm to use percentage based luma tolerance
    float                    luma_tolerance_pct;                  // Tolerance to deem AEC as settled, in percentage
    int                      frame_skip_startup;                  // Frame skip value to be used upon camera startup
    float                    aggressiveness_startup;              // Aggressiveness value to use upon camera startup
    float                    exposure_index_adj_step;             // Exposure step size (0x304)
    float                    ISO100_gain;                         // ISO 100 gain value (0x304)
    int                      antibanding_vs_ISO_priority;         // In manual ISO, whether anti-banding or ISO takes priority
    int                      iso_quantization_enable;             // Set this flag to enable standardized ISO
    float                    max_snapshot_exposure_time_allowed;  // Maximum snapshot exposure time (in seconds) (0x304)
    unsigned int             lux_index_ref;                       // A reference value to be used for lux index computation
    unsigned short           aec_subsampling_factor;              // AEC stats subsampling for power optimization
    aec_idx_comp_adapter     idx_comp;                            // AEC index compensation
    aec_ev_table_adapter     aec_ev_table;                        // AEC EV compensation table
    aec_luma_weight_adapter  aec_luma_weight;                     // Color and sensor compensation for luma calculation

    /* Extended Parameter */
    aec_fast_aec_tuning      fast_aec;                            // Fast AEC tuning
    aec_lls_tuning           low_light_shot;                      // Low Light Shot
    float                    flat_non_grey_th_low;                // Flat scene parameters
    float                    flat_non_grey_th_high;               // Flat scene parameters
    int                      enable_LV_test;                      // Customer LV tests
    boolean                  enable_stored_params;                // Flag to enable\disable warm start store/restore settings
    float                    warmstart_exp_range;                 // Warm-Start exposure range
    aec_aggressive_tuning    aggressive;                          // Aggressive AEC
    uint32                   ignore_hal_lock_before_preflash;     // Sync sequence between Preflash and HAL lock
    float                    lux_index_filter_factor;             // Temporal filtering of the lux index value
    int                      requested_rgn_skip_pattern;
    uint16                   fast_aec_forced_cnt;                 // Force AWB to run with this number during fast AEC
    boolean                  enable_cap_exp_index_for_flash_estimation; // Gain Limit For Flash Capture
    float                    cap_exp_index_ratio_for_flash_estimation;

    /* Extended Paramters for Dual Camera tuning */
    float                    dual_cam_exposure_mult;              // Reserved[3] - Dual Camera exposure multiplier
    float                    dual_cam_comp_ratio[AEC_DUAL_CAM_COMP_TABLE_SIZE]; // Reserved[4->9] - Compensation ratios for different AWB color temperatures
} aec_generic_tuning_adapter;

/**
* This structure defines the motion ISO tuning parameters
*/
typedef struct
{
    int       motion_iso_enable;           // Flag to enable\disable the motion ISO feature
    float     motion_iso_aggressiveness;   // Motion ISO aggressiveness
    float     motion_iso_threshold;        // Threshold value for the motion ISO compensation to kick in
    float     motion_iso_max_gain;         // Maximum gain that can be used for the motion ISO feature
} aec_motion_iso_adapter;

/**
* This structure defines the gain tradeoff and maximum exposure time entries for the snapshot look-up table (LUT)
*/
typedef struct
{
    unsigned short   lux_index;        // Lux index corresponding to each entry
    float            gain_trade_off;   // Gain tradeoff value
    float            max_exp_time;     // Maximum exposure time value (in seconds)
} snapshot_trade_off_table_adapter;

/**
* This structure defines the the snapshot look-up table (LUT)
*/
typedef struct
{
    int                               enable;                                     // Flag to enable\disable the snapshot LUT feature
    int                               exposure_stretch_enable;                    // Flag to enable\disable the exposure stretch feature
    unsigned char                     valid_entries;                              // Number of valid entries in the snapshot LUT
    snapshot_trade_off_table_adapter  snapshot_ae_table[MAX_SNAPSHOT_LUT_SIZE];   // Snapshot look-up table (LUT)
} aec_snapshot_exposure_adapter;

/**
* LED mix entry contains: current combinations, reference point information, and flux value
*/
typedef struct
{
    unsigned short LED1_setting;
    unsigned short LED2_setting;
    float          rg_ratio;
    float          bg_ratio;
    float          flux;
} LED_mix_adapter;

/**
* This structure specifies a particular entry in led_table that should be calibrated
*/
typedef struct
{
    unsigned short   led_power_index;
    unsigned short   led_mix_index;
} LED_mix_calibration_entry_adapter;

/**
* This structure defines single/dual led calibration
*/
typedef struct
{
    unsigned short                     calib_num;                                          // Number of combinations to calibrate
    LED_mix_calibration_entry_adapter  calib_entries[MAX_CALIB_NUM];                       // Specific entries to calibrate
    int                                corner_sample_screen_enable;                        // Enable screening of corner samples in calibration
    float                              rg_ratio_ok_range[MAX_CALIB_NUM][CALIB_RANGE_NUM];  // RG ratio range to be flagged as OK
    float                              bg_ratio_ok_range[MAX_CALIB_NUM][CALIB_RANGE_NUM];  // BG ratio range to be flagged as OK
    unsigned short                     fixed_CCTs[MAX_CALIB_NUM];                          // CCTs to be fed to ISP during calibration
    unsigned int                       fixed_exp_index[MAX_CALIB_NUM];                     // Fixed exposure indices for dual LED calibration
    float                              frame_err_allowance;                                // Frame-to-frame allowance of error during calibration
    unsigned short                     frame_num;                                          // Number of frames to average for each calibration
    unsigned short                     frame_num_max;                                      // Max number of frames to be used for each calibration
} LED_mix_calibration_adapter;

/**
* Unified LED tuning
*/
typedef struct
{
    int                          dual_led_enable;            // 1 to enable dual LED, 0 to enable single LED
    float                        dual_led_intersect_slope;   // Slope is used to do intersection for dual LED estimation
    unsigned short               led_power_size;             // Number of power levels for LED tuning (de-rating)
    unsigned short               led_mix_size;               // Number of dual LED mixing entries
    LED_mix_adapter              cct_table[MAX_LED_POWER_LEVEL][MAX_LED_MIX_LEVEL];
    LED_mix_calibration_adapter  cct_table_calibration;      // Dual LED calibration feature
} aec_led_match_table_adapter;

typedef struct
{
    float                        pre_flux;                    // Pre-flash/torch flux value
    float                        flux_hi;                     // High power LED flux value
    float                        flux_med;                    // Medium power LED flux value
    float                        flux_low;                    // Low power LED flux value
} aec_led_flux_adapter;

typedef struct
{
    // For general flash and LED tuning
    aec_lux_trigger_adapter     wled_trigger;                  // Lux index above which flash is fired in the AUTO mode
    int                         force_led_af_in_led_on_mode;   // Set the flag to force AF always run when LED is set to ON
    boolean                     led_flux_used;                 // Indicates whether led_flux field is used. If false, use the led_table_tuning field instead
    aec_led_flux_adapter        led_flux;                      // Stores LED flux values.
    aec_led_match_table_adapter led_table_tuning;              // For both dual and single LED tuning, with de-rating support

    int                         smart_flash_est_enable;        // Enable smart LED estimation for AEC
    float                       smart_flash_est_strength;      // Amount of flexibility/strength allowed for smart LED (0-1)
    int                         smart_flash_est_awb_enable;    // Enable smart LED estimation also for manual AWB
    float                       smart_flash_est_awb_strength;  // Amount of flexibility/strength allowed for AWB smart LED (0-1)

    int                         flash_target_enable;           // Enable a dedicated set of flash luma target
    aec_luma_target_adapter     flash_target;                  // Luma target for flash (similar as regular luma target)
    float                       target_interp_k_start;         // Below this k value, current luma target is used for flash
    float                       target_interp_k_end;           // Above this k value, flash target will be used

    int                         smart_flash_target_enable;     // Enable smart decision of luma target (Target may increase)
    int                         max_target_offset;             // Maximum allowed for the luma target to increase

    /* Extended Parameters */
    int                         enable_led_sat_prevent;        // GenericTuning - Reserve[0] - Saturation prevention
    int                         enable_speckle_handling;       // GenericTuning - Reserve[1] - speckle reflection handling
    float                       k_floor_pct;                   // GenericTuning - Reserve[2] - LED Bias calculation related

    int                         bright_level_diff_thres;       // reserve[0]
    int                         bright_level_thres;            // reserve[1]
    float                       bright_sat_diff_thres;         // reserve[2]
    float                       bright_sat_thres;              // reserve[3]
    int                         conv_min_luma;                 // reserve[4]
    int                         conv_min_adjust;               // reserve[5]
    int                         off_stats_min;                 // reserve[6]
    float                       sat_pct_thres;                 // reserve[7]
    float                       k_stats_pct;                   // reserve[8]
    float                       k_stats_min_diff;              // reserve[9]
    int                         high_stat_target;              // reserve[10]
    float                       led_conv_smooth_rate;          // reserve[11]
    float                       sat_prevent_strength;          // reserve[12]
    float                       sat_pct_start;                 // reserve[13]
    float                       sat_pct_end;                   // reserve[14]
} aec_flash_tuning_adapter;

/**
* This structure defines the touch AEC tuning parameters (0x304)
*/
typedef struct
{
    int     touch_roi_enable;       // Flag to enable\disable the touch AEC feature
    float   touch_roi_weight;       // Determines the influence of the touched region frame luma value on the overall frame luma calculation

    /* Extended Parameters - Advanced Tuning */
    float   temporal_filter_weight;
    float   speed_adjust_ratio;
    float   center_weight_strength;
    boolean touch_LED_metering_enable;

    /* Extended Parameters - Touch EV tuning */
    boolean touch_EV_enable;
    uint16  luma_delta_thres;
    float   luma_avg_pct_thres;
    float   luma_avg_min;
} aec_touch_adapter;

/**
* This structure defines the face detection AEC tuning parameters (0x304: AEC_face_priority_adapter)
*/
typedef struct
{
    int     aec_face_enable;         // Flag to enable\disable the face detection AEC feature
    float   aec_face_weight;         // Determines the influence of the face frame luma value on the overall frame luma calculation

    /* Extended parameters - Advanced Tuning */
    float   status_update_delay_time_enter;
    float   status_update_delay_time_exit;
    float   speed_adjust_delay_time_enter;
    float   speed_adjust_delay_time_exit;
    float   speed_adjust_ratio;
    float   temporal_filter_weight;
    float   center_weight_strength;

    /* Extended Paramters - Backlit Compensation */
    boolean backlit_comp_enable;
    float   backlit_weight_adjust_hi;
    float   backlit_weight_adjust_low;
    float   backlit_luma_ratio_thres_hi;
    float   backlit_luma_ratio_thres_low;
} aec_face_priority_adapter;

typedef struct
{
    int     frame_skip;           // Number of frames skipped between each exposure update
    float   aggressiveness;       // Aggressivenes value which moving from a dark scene to a bright scene

    /* Extended Parameters */
    float   bright_ratio_adjust;
    float   dark_ratio_adjust;
    float   low_luma_cap_ratio;
    boolean enable_speed_adjust;
} aec_fast_conv_adapter;

//=============================================================================
//
// Variable name: ht_tolerance_adjust_factor
// In order for holding time feature to activate, AEC needs to be stable for a
//   period of time. A different threshold/tolerance can be used to define whether
//   AEC is stable for holding time feature. This factor is used to adjust/relax
//   the luma tolerance used in normal mode for the purpose of holding time trigger.
// The higher this value, the easier holding time can be triggered.
// Default value: 2.0
// Range: [1.0, 4.0]
//
//=============================================================================
typedef struct
{
    int     slow_convergence_enable;   // Flag to enable slow convergence for video
    int     frame_skip;                // Number of frames skipped between each exposure update
    float   conv_speed;                // Convergence speed (0-1)
    int     ht_enable;                 // Flag to enable\disable the holding time logic
    float   ht_tolerance_adjust_factor;// This factor is used to relax luma tolerance for triggering holding time
    float   ht_thres;                  // Holding time activation threshold for luma settle (in seconds)
    float   ht_max;                    // Maximum amount of holding time allowed (in seconds)
    int     ht_gyro_enable;            // Enable to use gyro for shortening holding time

    /* Extended parameters - Speed related advanced tuning */
    float   ref_frame_rate;
    float   step_dark;
    float   step_bright;
    float   step_regular;
    float   luma_tol_ratio_dark;
    float   luma_tol_ratio_bright;
    float   raw_step_adjust_cap;
    int     adjust_skip_luma_tolerance;
    float   dark_region_num_thres;
    float   bright_region_num_thres;

    /* Extended Parameters holding time advanced tuning */
    float   ht_luma_thres_low;
    float   ht_luma_thres_high;
    float   ht_luma_val_low;
    float   ht_luma_val_high;
    float   ht_gyro_thres_low;
    float   ht_gyro_thres_high;
    float   ht_gyro_val_low;
    float   ht_gyro_val_high;
} aec_slow_conv_adapter;

/**
* Bright region tuning parameters
*/
typedef struct
{
    unsigned int          bright_region_thres;  // Threshold for a region to be considered a bright region (Q8)
    float                 bright_weight;        // Weight to increase or decrease the influence of bright regions
} aec_bright_region_triggered_adapter;

typedef struct
{
    int                                  bright_region_enable;                    // Flag to enable\disable the bright region feature
    int                                  num_zones;                               // Number of zones actually used
    aec_lux_trigger_adapter              triggers[MAX_AEC_TRIGGER_ZONE - 1];        // Trigger point to enter the next zone
    aec_bright_region_triggered_adapter  triggered_params[MAX_AEC_TRIGGER_ZONE];  // Parameters that can be tuned per-zone
    float                                min_tweight;                             // Minimal bias table weight for a stats to be included
    float                                max_bright_pct;                          // Maximum number of bright regions to be counted
} aec_bright_region_adapter;

/**
* Dark region tuning parameters
*/
typedef struct
{
    unsigned int          dark_region_low_thres;  // Below this threshold means it is dark (Q8)
    unsigned int          dark_region_high_thres; // Above this threshold means it is not dark (Q8)
    float                 dark_weight;            // Weight to increase or decrease the influence of dark region
} aec_dark_region_triggered_adapter;

typedef struct
{
    int                                dark_region_enable;                        // Flag to enable\disable the dark region feature
    int                                num_zones;                                 // Number of zones actually used
    aec_lux_trigger_adapter            triggers[MAX_AEC_TRIGGER_ZONE - 1];          // Trigger point to enter the next zone
    aec_dark_region_triggered_adapter  triggered_params[MAX_AEC_TRIGGER_ZONE];    // Parameters that can be tuned per-zone
    float                              min_tweight;                               // Minimal bias table weight for a stats to be include
    float                              max_dark_pct;                              // Maximum number of dark regions to be counted
} aec_dark_region_adapter;

/**
* This structure defines the AEC extreme color tuning parameters
*/
typedef struct
{
    float                             adj_ratio[Q3A_AEC_BAYER_CHNL_MAX];         // adjust ratios for all channels
} aec_extreme_color_triggered_adapter;

typedef struct
{
    int                                  extreme_color_enable;                   // Flag to enable\disable the extreme colour feature
    int                                  num_zones;                              // Number of zones actually used
    aec_lux_trigger_adapter              triggers[MAX_AEC_TRIGGER_ZONE - 1];       // Trigger point to enter the next zone
    aec_extreme_color_triggered_adapter  triggered_params[MAX_AEC_TRIGGER_ZONE]; // Parameters that can be tuned per-zone

    float                                red_th;                                 // Extreme red threshold of r/g ratio
    float                                red_interp_pct;                         // Extreme red thresh interpolation range
    float                                green_rg_th;                            // Extreme green threshold of r/g ratio
    float                                green_rg_interp_pct;                    // Extreme green r/g thresh interpolation range
    float                                green_bg_th;                            // Extreme green threshold of b/g ratio
    float                                green_bg_interp_pct;                    // Extreme green b/g thresh interpolation range
    float                                green_cntr_th;                          // Extreme green thresh for the distance to (1,1) on (r/g)&(b/g) domain
    float                                green_cntr_interp_pct;                  // Extreme green center thresh interpolation range
    float                                blue_th;                                // Threshold for extreme blue detection
    float                                blue_interp_pct;                        // Extreme blue thresh interpolation range

    float                                luma_high_th;                           // High threshold for region luma
    float                                luma_low_th;                            // Low threshold for region luma
    float                                color_stats_pct_high_th;                // High threshold for percentage of extreme color stats
    float                                color_stats_pct_low_th;                 // Low threshold for percentage of extreme color stats

    /* Extended Parameters */
    float                                dbg_print_period;                       // Debug message print period
} aec_extreme_color_adapter;

typedef struct
{
    /* Flat scene detection tuning */
    int     hist_flat_detector_enable;          // Flag to enable\disable the histogram based flat scene detection feature
    int     start_level;                        // Starting point for the histogram peak detection
    int     end_level;                          // Ending point for the histogram peak detection
    int     range;                              // Range for the histogram peak detection
    float   delta_th_low;                       // Low threshold value for peak detection
    float   delta_th_high;                      // High threshold value for peak detection
    float   bright_flat_det_th;                 // Threshold value for bright flat scene detection
    float   dark_flat_det_th;                   // Threshold value for dark flat scene detection
    float   bright_flat_tail_det;               // Threshold value for tail detection for bright flat scenes
    float   dark_flat_tail_det;                 // Threshold value for tail detection for dark flat scenes
    /* Flat scene compensation tuning */
    int     bright_flat_compensation_enable;   // Flag to enable\disable the bright flat scene compensation
    int     dark_flat_compensation_enable;     // Flag to enable\disable the dark flat scene compensation
    float   flat_white_grey_vs_nongrey_th;     // Threshold to determine if flat area is brighter than colored regions
    float   flat_dark_grey_vs_nongrey_th;      // Threshold to determine if flat area is darker than colored regions
    float   near_grey_tolerance;               // Regions with R/G and B/G within the tolerance percentage are considered grey regions
    float   bright_flat_compensation_ratio;    // Ratio for the bright flat scene compensation
    float   dark_flat_compensation_ratio;      // Ratio for the dark flat scene compensation
    int     flat_indoor_disable_start_idx;     // Above this index, flat compensation start to get disabled
    int     flat_indoor_disable_end_idx;       // Above this index, flat compensation fully disabled
} aec_flat_scene_adapter;

//=============================================================================
// Variable name: start
// Start percentage for the range of pixels used to calculate
// either saturation position, or darkness position
// Default Value: 0.01
// Data range: [0 1]
//
// Variable name: end
// End percentage for the range of pixels used to calculate
// either saturation position, or darkness position
// Default Value: 0.04
// Data range: [0 1]
//=============================================================================
typedef struct
{
    float start;
    float end;
} aec_hist_pct_range_adapter;

//=============================================================================
// Variable name: hist_sat_pushup_range
// The saturated percentage range for push-up compensation (flat/low dynamic range scenes).
// It is recommended to use a bigger range for push-up for stability reasons.
// Default Value: {{0.02, 0.08}, {0.02, 0.08}, {0.02, 0.08}}
//
// Variable name: hist_sat_pushdown_range
// The saturated percentage range for push-down compensation (high dynamic range/overexposed scenes).
// It is recommended to use a smaller range for push-down, so that it can more effectively
// prevent smaller area of saturation.
// Default Value: {{0.01, 0.04}, {0.01, 0.04}, {0.01, 0.04}}
//
// Variable name: hist_dark range
// The dark area percentage range used to calculate darkness position and affect luma target.
// It is generally not recommended to use this particular part due to stability concerns.
// Default Value: {{0, 0.15}, {0, 0.15}, {0, 0.15}}
//
// Variable name: max_drc_gain
// Maximal DRC gain allowed for the particular lux zone. This variable is only valid when
// adrc mode is not set to disable.
// Default Value: {4.0, 4.0, 4.0}
// Range: [1.0, 8.0]
//
// Variable name: adrc_conv_speed
// Defines the convergence speed for ADRC tradeoff. The bigger, ADRC converges faster.
// If it is small, then the highlight may take more time to show up, while if it is too big,
// then there could be stability issues.
// No need to tune normally.
// Default Value: {0.15, 0.15, 0.15}
// Range: (0, 0.5]
//
// Variable name: gtm_percentage
// Percentage of current DRC gain to be applied to GTM. This is only applicable if chipset has
// GTM module support.
// Default Value: {0.5, 0.5, 0.5}
// Range: [0, 1]
//
// Variable name: ltm_percentage
// Percentage of current DRC gain to be applied to LTM. This is only applicable if chipset has
// LTM module support.
// Default Value: {0.5, 0.5, 0.5}
// Range: [0, 1]
//
// Variable name: la_percentage
// Percentage of current DRC gain to be applied to GTM. This is only applicable if chipset has
// LA module support.
// Default Value: {0,0,0}
// Range: [0, 1]
//=============================================================================
/**
* This structure defines the histogram based AEC metering parameters
* default & range values described in aec_hist_metering_adapter.
*/
typedef struct
{
    float                       max_target_adjust_ratio;                         // Maximum target adjust allowed
    float                       min_target_adjust_ratio;                         // Minimum target adjust allowed
    unsigned short              sat_low_ref;                                     // Low reference/control point for saturation
    unsigned short              sat_high_ref;                                    // High reference/control point for saturation
    unsigned short              dark_low_ref;                                    // Low reference/control point for darkness
    unsigned short              dark_high_ref;                                   // High reference/control point for darkness
    aec_hist_pct_range_adapter  hist_sat_pushup_range[Q3A_AEC_BAYER_CHNL_MAX];   // Percentage of bright pixels used in determine saturation
    aec_hist_pct_range_adapter  hist_sat_pushdown_range[Q3A_AEC_BAYER_CHNL_MAX]; // Percentage of bright pixels used in determine saturation
    aec_hist_pct_range_adapter  hist_dark_range[Q3A_AEC_BAYER_CHNL_MAX];         // Percentage of dark pixels used to determine darkness
    float                       max_drc_gain;                                    // Maximal DRC gain allowed
    float                       adrc_conv_speed;                                 // ADRC tradeoff convergence speed
    float                       gtm_percentage;                                  // Percentage of total current DRC gain to be allocated to GTM
    float                       ltm_percentage;                                  // Percentage of total current DRC gain to be allocated to LTM
    float                       la_percentage;                                   // Percentage of total current DRC gain to be allocated to LA
    float                       gamma_percentage;                                // Percentage of total current DRC gain to be allocated to Gamma
} aec_hist_metering_triggered_adapter;

typedef struct
{
    boolean                         adrc_antibanding_priority_enable;
    int                             atb_cutoff_idx_50hz;
    int                             atb_cutoff_idx_60hz;
    aec_lux_trigger_adapter         atb_trigger;
} adrc_atb_ctrl_tuning_adapter;

//=============================================================================
// Variable name: adrc_enable
// ADRC is combined with histogram AEC for tuning interface structuring. Set this flag to
// either disable or enable ADRC. This replaced the "mode" parameter in previous header
// revision. GTM/LTM/LA are independently controlled via percentage variables, and hence
// there is no need to distinguish local or global modes.
// Default Value: 0
// Range: 0 and 1
//
// Variable name: dark_end_control_enable
// Dark end part of the histogram AEC is sometimes unstable (hist_dark_range etc.), and hence
// it can be disabled with this flag. It'll save computation if it is not used and disabled.
// Default Value: 0
// Range: 0 and 1
//
//=============================================================================
typedef struct
{
    int                                   hist_metering_enable;                       // Flag to enable histogram based AEC metering
    int                                   adrc_enable;                                // Enable or disable ADRC
    int                                   dark_end_control_enable;                    // This parameter will be used to disable dark end control
    int                                   num_zones;                                  // Number of zones actually used
    aec_lux_trigger_adapter               triggers[MAX_AEC_TRIGGER_ZONE - 1];           // Trigger point to enter the next zone
    aec_hist_metering_triggered_adapter   triggered_params[MAX_AEC_TRIGGER_ZONE];     // Parameters that can be tuned per-zone
    float                                 target_filter_factor;                       // Temporal filter for stability

    /* Extended Paramters */
    adrc_atb_ctrl_tuning_adapter          adrc_atb_ctrl;
    boolean                               gtm_enable;
    boolean                               ltm_enable;
    boolean                               la_enable;
    boolean                               gamma_enable;
    boolean                               dark_pushup_enable;
    boolean                               clamp_sat_pos_enable;

    /* Touch ROI or Dynamic ROI Metering for AEC
       Using Reserved params of AEC HDR */
    boolean touch_roi_hist_adjust_enable; // reserve[0] of AEC HDR
    boolean face_roi_hist_adjust_enable;  // reserve[1] of AEC HDR

    float   dark_roi_max_target_adjust_ratio[MAX_AEC_TRIGGER_ZONE]; // reserve[2] of AEC HDR
    float   dark_roi_min_target_adjust_ratio[MAX_AEC_TRIGGER_ZONE]; // reserve[3] of AEC HDR
    float   dark_roi_dark_low_ref[MAX_AEC_TRIGGER_ZONE];            // reserve[4] of AEC HDR
    float   dark_roi_sat_high_ref[MAX_AEC_TRIGGER_ZONE];            // reserve[5] of AEC HDR

    float   bright_roi_max_target_adjust_ratio[MAX_AEC_TRIGGER_ZONE]; // reserve[6] of AEC HDR
    float   bright_roi_min_target_adjust_ratio[MAX_AEC_TRIGGER_ZONE]; // reserve[7] of AEC HDR
    float   bright_roi_dark_low_ref[MAX_AEC_TRIGGER_ZONE];            // reserve[8] of AEC HDR
    float   bright_roi_sat_high_ref[MAX_AEC_TRIGGER_ZONE];            // reserve[9] of AEC HDR

    float   dark_roi_luma_ratio_start[MAX_AEC_TRIGGER_ZONE];          // reserve[10] of AEC HDR
    float   dark_roi_luma_ratio_end[MAX_AEC_TRIGGER_ZONE];            // reserve[11] of AEC HDR
    float   bright_roi_luma_ratio_start[MAX_AEC_TRIGGER_ZONE];        // reserve[12] of AEC HDR
    float   bright_roi_luma_ratio_end[MAX_AEC_TRIGGER_ZONE];          // reserve[13] of AEC HDR
} aec_hist_metering_adapter;

//=============================================================================
// Feature name: Histogram Stretch feature.
// Applicable operation mode:  Snapshot, Preview and Camcorder.
// When this feature is enabled, 3A will send dark clamp to be subtracted to
// VFE ACE (Color Conversion) module per frame, ACE driver will compute the new
// RGB2Y coefficients based on strech gain computed.
//
// Variable name: dark_percentage
// Histogram counts will be accumulated from Pixel Value 0 and increasing,when
// the occurrence percentage is greater than or equal to dark_percentage, the
// corresponding pixel K0 is used as offset. K0 is subtracted in VFE Color conversion
// ( RGB to Y ) process after clamping with max_dark_clamp
// Default value: {0.005, 0.005, 0.005}
// Range: [0.001, 0.05]
//
// Variable name: max_dark_clamp
// RGB to Y offset programmed with minimum of (max_dark_clamp,K0)
// Default value: {3, 3, 3}
// Range: 0 to 6
//
//=============================================================================
typedef struct
{
    float       dark_percentage;
    int         max_dark_clamp;
} aec_Y_hist_stretch_triggered_adapter;

//=============================================================================
// Variable name: enable
// Enable histogram stretch feature to be applied in ACE.
// Default value: 0
// Range: 0 and 1
//
// Variable name: num_zones
// Number of lux trigger zones used for bright region tuning.
// Default value: 3
// Range: 1 to 6
//
// Variable name: triggers
// Defines each lux trigger to be used for separating the zones.
//   Number of triggers should be 1 less than num_zones, and it is arranged in
//   sequence to align with the zones. For example, if we have 3 zones (zone_1,
//   zone_2 & zone_3), then triggers should contain in sequence, trigger for
//   zone_1 to zone_2, and trigger for zone_2 to zone_3.
// Default value: Same as luma target triggers
//=============================================================================
typedef struct
{
    int                                    enable;                                 // Enable histogram stretch feature in ACE
    int                                    num_zones;                              // Number of zones actually used
    aec_lux_trigger_adapter                triggers[MAX_AEC_TRIGGER_ZONE - 1];       // Trigger point to enter the next zone
    aec_Y_hist_stretch_triggered_adapter   triggered_params[MAX_AEC_TRIGGER_ZONE]; // Parameters that can be tuned per-zone
} aec_Y_hist_stretch_adapter;

typedef struct
{
    aec_lux_trigger_adapter  hdr_indoor_trigger;            // For toggling exposure ratio
    float maxIdealHDRgain;                                  // Making max HDR gain as configurable
} aec_hdr_tuning_adapter;

//=============================================================================
// Feature name: Fast AEC convergence logic
// Description: This structure includes the tuning parameters for the fast AEC convergence logic
// Applicable operation modes: Camera start, camera switch (RFC <-> FFC), switch
// between preview and camcorder mode and vice versa
//
// Variable name: start_index
// The initial exposure index when sensor starts streaming in fast AEC mode
// Default value: 240.
// Data range: 0 to 500.
//
// Variable name: frame_skip
// This parameter specifies the number of frames to skip between each exposure update
// in the fast AEC mode. A higher value indicates that more frames are skipped between
// updates and a higher value is used when it takes a long time for the previous exposure
// update to be reflected in the stats
// Default: 3
// Range : 0 to 10
//
// Variable name: luma_settled_count
// This parameter specifies the number of successive frames where AEC is settled before the 'settled'
// flag  can be set to TRUE in the fast AEC mode. A higher value ensures accuracy at the expense
// of latency. A smaller value could result in some discrepancy observed in back-to-back runs
// Default value: 2
// Range: 0 to 3
//=============================================================================
typedef struct
{
    unsigned int    start_index;           // Exposure index to be used upon start of fast AEC
    int             frame_skip;            // Number of frames skipped between each exposure update during fast AEC
    unsigned short  luma_settled_count;    // Number of successive frames for AEC to be flag as 'settled'
} aec_fast_aec_adapter;

typedef struct
{
    aec_generic_tuning_adapter     aec_generic;                                     // Generic AEC tuning parameters
    aec_exposure_table_adapter     aec_exposure_table;                              // Exposure table
    aec_snapshot_exposure_adapter  aec_snapshot_lut;                                // Snapshot look-up table (LUT) tuning parameters
    aec_luma_target_adapter        aec_luma_target;                                 // Defines the various luma targets
    aec_metering_table_adapter     aec_metering_tables;                             // Metering tables (center-weighted\spot-metering)
    aec_fast_conv_adapter          aec_fast_convergence;                            // Fast convergence tuning parameters
    aec_slow_conv_adapter          aec_slow_convergence;                            // Slow convergence tuning parameters
    aec_flash_tuning_adapter       aec_led_flash;                                   // AEC LED flash tuning parameters
    aec_touch_adapter              aec_touch;                                       // Touch AEC tuning parameters
    aec_face_priority_adapter      aec_face_detection;                              // Face detection AEC tuning parameters
    aec_bright_region_adapter      aec_bright_region;                               // Bright region tuning parameters
    aec_dark_region_adapter        aec_dark_region;                                 // Dark region tuning parameters
    aec_extreme_color_adapter      aec_extreme_color;                               // Extreme colour tuning parameters
    aec_flat_scene_adapter         aec_flat_scene;                                  // Flat scene tuning parameters
    aec_hist_metering_adapter      aec_hist_metering;                               // Histogram based metering tuning parameters
    aec_Y_hist_stretch_adapter     aec_Y_hist_stretch;                              // Y histogram stretch feature in ACE
    aec_hdr_tuning_adapter         aec_hdr;                                         // HDR AEC tuning parameters
    aec_motion_iso_adapter         aec_preview_motion_iso;                          // Preview motion ISO tuning parameters
    aec_motion_iso_adapter         aec_snapshot_motion_iso;                         // Snapshot motion ISO tuning parameters
    aec_fast_aec_adapter           aec_fast_aec;                                    // Fast AEC convergence tuning parameters
} aec_algo_tuning_adapter;

//=============================================================================
//=============================================================================
//  AEC related params end
//=============================================================================
//=============================================================================

typedef struct
{
    // TODO: temporarily use the old types until the wrapper has been extended for these types as well
    ChromatixVersionInfoType                    chromatix_version_info;
    AAA_version_type                            aaa_version;
    awb_algo_struct_type                        AWB;
    aec_algo_tuning_adapter                     AEC;
    chromatix_auto_flicker_detection_data_type  AFD;
    AAA_ASD_struct_type                         ASD;
    af_algo_tune_parms_adapter_t                AF;
    chromatix_aaa_sync_type                     aaa_sync_data;
    Chromatix_EIS_algo_type                     EIS;
} chromatix_3a_parms_wrapper;

//=============================================================================
//=============================================================================
//  Function Declarations
//=============================================================================
//=============================================================================

#ifdef __cplusplus
extern "C"
{
#endif
//void af_tuning_pdaf_reserve_parameters_read(void* p_params, af_tuning_algo_t* af_tuning_algo);
    boolean parse_AF_chromatix(void* af_tuning_info, af_algo_tune_parms_adapter_t* tuning_adapter);

    boolean parse_AEC_chromatix(void* aec_tuning_info, aec_algo_tuning_adapter* tuning_adapter);

    boolean parse_chromatix(void* chromatix_ptr, chromatix_3a_parms_wrapper* chromatix_adapter);

#ifdef __cplusplus
}
#endif

#endif /*__Q3A_CHROMATIX_WRAPPER_H__*/


