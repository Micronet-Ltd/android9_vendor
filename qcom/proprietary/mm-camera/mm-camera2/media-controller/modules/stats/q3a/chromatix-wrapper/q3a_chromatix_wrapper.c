/* q3a_chromatix_wrapper.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "q3a_platform.h"
#include "q3a_chromatix_wrapper.h"

/* Since we don't have access to the C11 compile-time static_assert() method, this
   is a naive alternate implementation to do the same. The macro asserts the condition
   at compile time and enforces a compile error if assertion fails. Compilation errors
   such as below indicate that the compile-time assertion has failed and needs to be
   examined to determine the cause of error:
       error: size of array 'MY_ASSERT' is negative */
#define STATIC_ASSERT(cond) { char MY_ASSERT[ (!!(cond))*2 - 1 ]; }

/* This is a helper macro to memcopy an array while enforcing source
   and destination compile-time size checks. */
#define COPY_ARRAY(dest, dest_array_count, source, source_array_count)         \
    STATIC_ASSERT(sizeof(dest[0]) == sizeof(source[0]));                       \
    STATIC_ASSERT((size_t)(dest_array_count) == (size_t)(source_array_count)); \
    Q3A_MEMCPY_S(dest,                                                         \
                 sizeof(dest[0]) * dest_array_count,                           \
                 source,                                                       \
                 sizeof(source[0]) * source_array_count);

/* This is a helper macro to memcopy a single field while enforcing source
   and destination compile-time size checks. Similar to a safe-memcopy but
   with compile-time size checks rather than run-time size checks */
#define COPY_FIELD(dest, source)                                               \
    STATIC_ASSERT(sizeof(dest) == sizeof(source));                             \
    Q3A_MEMCPY_S(&dest, sizeof(dest), &source, sizeof(source));

/*=============================================================================
 *  Local AEC Definitions
 *=============================================================================*/

/* Warm start (store restore) exposure index range percentage */
#define WARM_START_EXP_RANGE_PERC                     (60)

/* Startup frame-skip in aggressive AEC convergence */
#define AGGRESSIVE_AEC_STARTUP_FRAME_SKIP              (3)

/* Frame-skip in aggressive AEC convergence */
#define AGGRESSIVE_AEC_FRAME_SKIP                      (2)

/* Aggressive AEC luma tolerance percentage */
#define AGGRESSIVE_AEC_LUMA_TOLERANCE_PCT              (10)

/* Max frame convergence threshold for AEC settle in aggressive AEC */
#define AGGRESSIVE_AEC_MAX_FRAME_CONVERGENCE_THRESHOLD (13)

/* Min settled frames needed to declare AEC is settled */
#define AGGRESSIVE_AEC_MIN_FRAME_SETTLE_THRESHOLD      (1)

/* Min settled frames needed to declare AWB is converged */
#define AGGRESSIVE_AEC_MIN_AWB_SETTLE_COUNT            (3)

/* This count determines how many frames to skip for fast AEC
 * convergence, ideally only 2 frames are needed */
#define FAST_AEC_FRAME_SKIP                       3

/* This count determines how many frames are to be skipped when
 * fast AEC is run the first time */
#define FAST_AEC_STARTUP_FRAME_SKIP               4

/* Luma settled count value for fast AEC */
#define FAST_AEC_MIN_FRAME_SETTLE_THRESHOLD       1

/* Default maximum ideal HDR gain */
#define MAX_IDEAL_HDR_GAIN                        8


/*=============================================================================
 *  AF Methods
 *=============================================================================*/
void* af_get_algo_params(void);

static float af_pdaf_get_soft_scene_change_cnt(float soft_stable_cnt);
static int af_haf_fill_force_scan_cnt_th(int max_scan_cnt);


/** af_pdaf_get_soft_scene_change_cnt
* get scene change count from tuning
* default value is 1 and maximum threshold is 6
* @soft_scene_change_cnt: soft scene change count from tuning
*
*  Return: float
**/

static float af_pdaf_get_soft_scene_change_cnt(float soft_scene_change_cnt) {
  if (soft_scene_change_cnt < 1) {
    soft_scene_change_cnt = 1.0f;
  }
  else if (soft_scene_change_cnt > 6) {
    soft_scene_change_cnt = 6.0f;
  }
  return soft_scene_change_cnt;
}

static int af_haf_fill_force_scan_cnt_th(int max_scan_cnt){
  return (max_scan_cnt) ? max_scan_cnt : DEFAULT_MAX_SCAN_CNT_TH;
}

static void af_copy_single_threshold(single_threshold_adapter_t* dest, single_threshold_t* src) {
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  dest->change_ratio_inc   = src->change_ratio_inc;
  dest->change_ratio_dec   = src->change_ratio_dec;
#endif
  dest->flat_inc_thres     = src->flat_inc_thres;
  dest->flat_dec_thres     = src->flat_dec_thres;
  dest->macro_thres        = src->macro_thres;
  dest->drop_thres         = src->drop_thres;
  dest->hist_dec_dec_thres = src->hist_dec_dec_thres;
  dest->hist_inc_dec_thres = src->hist_inc_dec_thres;

  COPY_FIELD(dest->dec_dec_3frame, src->dec_dec_3frame);
  COPY_FIELD(dest->inc_dec_3frame, src->inc_dec_3frame);
  COPY_FIELD(dest->dec_dec, src->dec_dec);
  COPY_FIELD(dest->dec_dec_noise, src->dec_dec_noise);
  COPY_FIELD(dest->inc_dec, src->inc_dec);
  COPY_FIELD(dest->inc_dec_noise, src->inc_dec_noise);
  COPY_FIELD(dest->flat_threshold, src->flat_threshold);
}

static void af_copy_tuning_sad(af_tuning_sad_adapter_t* dest, af_tuning_sad_t* src) {
  dest->enable            = src->enable;
  dest->gain_min          = src->gain_min;
  dest->gain_max          = src->gain_max;
  dest->ref_gain_min      = src->ref_gain_min;
  dest->ref_gain_max      = src->ref_gain_max;
  dest->threshold_min     = src->threshold_min;
  dest->threshold_max     = src->threshold_max;
  dest->ref_threshold_min = src->ref_threshold_min;
  dest->ref_threshold_max = src->ref_threshold_max;
  dest->frames_to_wait    = src->frames_to_wait;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  dest->unstable_count_th     = src->unstable_count_th;
  dest->stable_count_th       = src->stable_count_th;
  dest->grid_region_start     = src->grid_region_start;
  dest->grid_region_end       = src->grid_region_end;
  dest->sadr_threshold_scaler = src->sadr_threshold_scaler;

  COPY_ARRAY(dest->aec_gain_input, AF_SCENE_CHANGE_MAX_ENTRIES,
             src->aec_gain_input,  AF_SCENE_CHANGE_MAX_ENTRIES);
  COPY_ARRAY(dest->sadr_threshold_scaler_multiplier, AF_SCENE_CHANGE_MAX_ENTRIES,
             src->sadr_threshold_scaler_multiplier,  AF_SCENE_CHANGE_MAX_ENTRIES);
#endif
}

boolean parse_AF_chromatix(void* af_tuning_info,
  af_algo_tune_parms_adapter_t* tuning_adapter_info)
{
  af_algo_tune_parms_t* tuning_info = (af_algo_tune_parms_t*)af_tuning_info;

  if (tuning_info == NULL)
  {
    AF_ERR("error: tuning_info is NULL");
    return false;
  }

  if (tuning_adapter_info == NULL)
  {
    AF_ERR("error: tuning_adapter_info is NULL");
    return false;
  }

//=============================================================================
// af_header_info_adapter_t: af_algo_tune_parms_t
//=============================================================================
  STATIC_ASSERT(sizeof(af_header_info_adapter_t) == sizeof(af_header_info_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_header_info,
    sizeof(af_header_info_adapter_t),
    &tuning_info->af_header_info,
    sizeof(af_header_info_t));

//=============================================================================
// af_tuning_algo_adapter_t:af_algo_tune_parms_t
//=============================================================================
  tuning_adapter_info->af_algo.af_process_type =
    (af_algo_adapter_t)tuning_info->af_algo.af_process_type;
  tuning_adapter_info->af_algo.position_near_end =
    tuning_info->af_algo.position_near_end;
  tuning_adapter_info->af_algo.position_default_in_macro =
    tuning_info->af_algo.position_default_in_macro;
  tuning_adapter_info->af_algo.position_boundary =
    tuning_info->af_algo.position_boundary;
  tuning_adapter_info->af_algo.position_default_in_normal =
    tuning_info->af_algo.position_default_in_normal;
  tuning_adapter_info->af_algo.position_far_end =
    tuning_info->af_algo.position_far_end;
  tuning_adapter_info->af_algo.position_normal_hyperfocal =
    tuning_info->af_algo.position_normal_hyperfocal;
  tuning_adapter_info->af_algo.position_macro_rgn =
    tuning_info->af_algo.position_macro_rgn;
  tuning_adapter_info->af_algo.undershoot_protect =
    tuning_info->af_algo.undershoot_protect;
  tuning_adapter_info->af_algo.undershoot_adjust =
    tuning_info->af_algo.undershoot_adjust;
  tuning_adapter_info->af_algo.min_max_ratio_th =
    tuning_info->af_algo.min_max_ratio_th;
  tuning_adapter_info->af_algo.led_af_assist_enable =
    tuning_info->af_algo.led_af_assist_enable;
  tuning_adapter_info->af_algo.led_af_assist_trigger_idx =
    tuning_info->af_algo.led_af_assist_trigger_idx;
  tuning_adapter_info->af_algo.lens_reset_frame_skip_cnt =
    tuning_info->af_algo.lens_reset_frame_skip_cnt;
  tuning_adapter_info->af_algo.low_light_gain_th =
    tuning_info->af_algo.low_light_gain_th;
  tuning_adapter_info->af_algo.base_delay_adj_th =
    tuning_info->af_algo.base_delay_adj_th;

  tuning_adapter_info->af_algo.af_haf.enable =
    tuning_info->af_algo.af_haf.enable;
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.algo_enable, AF_HAF_ALGO_MAX,
             tuning_info->af_algo.af_haf.algo_enable, AF_HAF_ALGO_MAX);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.stats_consume_enable, AF_FV_STATS_TYPE_MAX,
             tuning_info->af_algo.af_haf.stats_consume_enable, AF_FV_STATS_TYPE_MAX);

  tuning_adapter_info->af_algo.af_haf.lens_sag_comp_enable =
    tuning_info->af_algo.af_haf.lens_sag_comp_enable;
  tuning_adapter_info->af_algo.af_haf.hysteresis_comp_enable =
    tuning_info->af_algo.af_haf.hysteresis_comp_enable;
  tuning_adapter_info->af_algo.af_haf.actuator_shift_comp =
    tuning_info->af_algo.af_haf.actuator_shift_comp;
  tuning_adapter_info->af_algo.af_haf.actuator_hysteresis_comp =
    tuning_info->af_algo.af_haf.actuator_hysteresis_comp;
  tuning_adapter_info->af_algo.af_haf.actuator_sensitivity =
    tuning_info->af_algo.af_haf.actuator_sensitivity;
  tuning_adapter_info->af_algo.af_haf.stats_select =
    (af_fv_stats_adapter_t)tuning_info->af_algo.af_haf.stats_select;
  tuning_adapter_info->af_algo.af_haf.fine_srch_drop_thres =
    tuning_info->af_algo.af_haf.fine_srch_drop_thres;
  tuning_adapter_info->af_algo.af_haf.fine_step_size =
    tuning_info->af_algo.af_haf.fine_step_size;
  tuning_adapter_info->af_algo.af_haf.max_move_step =
    tuning_info->af_algo.af_haf.max_move_step;
  tuning_adapter_info->af_algo.af_haf.max_move_step_buffer =
    tuning_info->af_algo.af_haf.max_move_step_buffer;
  tuning_adapter_info->af_algo.af_haf.base_frame_delay =
    tuning_info->af_algo.af_haf.base_frame_delay;

//=============================================================================
// af_tuning_pdaf_adapter_t:: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  STATIC_ASSERT(sizeof(af_tuning_pd_roi_adapter_t) == sizeof(af_tuning_pd_roi_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.roi,
    sizeof(af_tuning_pd_roi_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.roi,
    sizeof(af_tuning_pd_roi_t));

  STATIC_ASSERT(sizeof(af_tuning_pd_focus_tbl_adapter_t) == sizeof(af_tuning_pd_focus_tbl_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.focus_tbl,
    sizeof(af_tuning_pd_focus_tbl_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.focus_tbl,
    sizeof(af_tuning_pd_focus_tbl_t));

  STATIC_ASSERT(sizeof(af_tuning_pd_conf_tbl_adapter_t) == sizeof(af_tuning_pd_conf_tbl_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.noise_tbl,
    sizeof(af_tuning_pd_conf_tbl_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.noise_tbl,
    sizeof(af_tuning_pd_conf_tbl_t));

  STATIC_ASSERT(sizeof(af_tuning_pd_noise_tbl_adapter_t) == sizeof(af_tuning_pd_noise_tbl_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.conf_tbl,
    sizeof(af_tuning_pd_noise_tbl_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.conf_tbl,
    sizeof(af_tuning_pd_noise_tbl_t));

  STATIC_ASSERT(sizeof(af_tuning_pd_stable_tbl_adapter_t) == sizeof(af_tuning_pd_stable_tbl_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.stable_tbl,
    sizeof(af_tuning_pd_stable_tbl_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.stable_tbl,
    sizeof(af_tuning_pd_stable_tbl_t));

  // af_tuning_pdaf_t: af_tuning_pd_focus_scan_t
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.focus_conv_frame_skip =
    tuning_info->af_algo.af_haf.pdaf.focus_scan.focus_conv_frame_skip;
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.enable_fine_scan =
    tuning_info->af_algo.af_haf.pdaf.focus_scan.enable_fine_scan;
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.min_fine_scan_range =
    tuning_info->af_algo.af_haf.pdaf.focus_scan.min_fine_scan_range;
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.fine_scan_step_size =
    tuning_info->af_algo.af_haf.pdaf.focus_scan.fine_scan_step_size;
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.focus_done_threshold =
    tuning_info->af_algo.af_haf.pdaf.focus_scan.focus_done_threshold;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.enable_fine_scan_extension =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[2];
  tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.first_run_not_conf_cnt_add =
    (int)tuning_info->af_algo.af_haf.pdaf.reserve[9];
#else
    tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.enable_fine_scan_extension =
      tuning_info->af_algo.af_haf.pdaf.focus_scan.enable_fine_scan_extension;
    tuning_adapter_info->af_algo.af_haf.pdaf.focus_scan.first_run_not_conf_cnt_add =
      tuning_info->af_algo.af_haf.pdaf.focus_scan.first_run_not_conf_cnt_add;
#endif

  // af_tuning_pdaf_t: af_tuning_pd_monitor_t
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.wait_after_focus_cnt =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.wait_after_focus_cnt;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.wait_conf_recover_cnt =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.wait_conf_recover_cnt;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.defocused_threshold =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.defocused_threshold;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.depth_stable_threshold =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.depth_stable_threshold;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.defocus_stable_filter_len =
    (int)tuning_info->af_algo.af_haf.pdaf.reserve[0];
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.enable_trigger_suppr_by_scene_change =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[1];
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.defocus_dof_multiplier =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[8];
#else
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.defocus_stable_filter_len =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.defocus_stable_filter_len;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.enable_trigger_suppr_by_scene_change =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.enable_trigger_suppr_by_scene_change;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.defocus_dof_multiplier =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.defocus_dof_multiplier;
  tuning_adapter_info->af_algo.af_haf.pdaf.scene_monitor.inc_defocus_thres_scaler =
    tuning_info->af_algo.af_haf.pdaf.scene_monitor.inc_defocus_thres_scaler;
#endif


// af_tuning_pdaf_t: af_tuning_pd_pdlib_adapter_t,
// af_tuning_pd_preproc_adapter_t, af_tuning_pd_internal_adapter_t
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.enable =
    tuning_info->af_algo.af_haf.pdaf.pd_lib.enable;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.max_shift =
    tuning_info->af_algo.af_haf.pdaf.pd_lib.max_shift;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.resize_ratio =
    tuning_info->af_algo.af_haf.pdaf.pd_lib.resize_ratio;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.confidence_scale =
    tuning_info->af_algo.af_haf.pdaf.pd_lib.confidence_scale;
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.smooth_filter, 7,
             tuning_info->af_algo.af_haf.pdaf.pd_lib.smooth_filter, 7);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.sobel_filter, 9,
             tuning_info->af_algo.af_haf.pdaf.pd_lib.sobel_filter, 9);
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_lib.defocus_confidence_th =
    tuning_info->af_algo.af_haf.pdaf.pd_lib.defocus_confidence_th;


  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.enable =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.enable;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.enable_do_not_use_defocus_sample_by_pd =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.enable_do_not_use_defocus_sample_by_pd;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.enable_inc_min_conf_thres_by_pd =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.enable_inc_min_conf_thres_by_pd;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_do_not_use_defocus_sample =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_do_not_use_defocus_sample;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_inc_min_conf_thres =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_inc_min_conf_thres;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.inc_min_conf_thres_by_pd_scaler =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.inc_min_conf_thres_by_pd_scaler;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_std_Threshold =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_std_Threshold;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_defocushi_err_sc_check =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_defocushi_err_sc_check;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_avgdefocushi_err_sc_check =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_avgdefocushi_err_sc_check;
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_gain_thr_sc_check =
    tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_gain_thr_sc_check;
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_defocushi_sc_check, AF_PD_MAX_DEFOCUS_RANGE,
             tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_defocushi_sc_check, AF_PD_MAX_DEFOCUS_RANGE);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_avgdefocushi_sc_check, AF_PD_MAX_DEFOCUS_RANGE,
             tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_avgdefocushi_sc_check, AF_PD_MAX_DEFOCUS_RANGE);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_defocushi_sc_check, AF_PD_MAX_DEFOCUS_RANGE,
             tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_defocushi_sc_check, AF_PD_MAX_DEFOCUS_RANGE);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_avgdefocushi_sc_check, AF_PD_MAX_DEFOCUS_RANGE,
             tuning_info->af_algo.af_haf.pdaf.pd_preproc.pd_lowlight_avgdefocushi_sc_check, AF_PD_MAX_DEFOCUS_RANGE);

  STATIC_ASSERT(sizeof(af_tuning_pd_internal_adapter_t) == sizeof(af_tuning_pd_internal_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.pdaf.pd_int,
    sizeof(af_tuning_pd_internal_adapter_t),
    &tuning_info->af_algo.af_haf.pdaf.pd_int,
    sizeof(af_tuning_pd_internal_t));
#endif

// af_tuning_pdaf_t: reserved parameter and af_tuning_pd_extended_info_adapter_t
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.enable_do_not_use_defocus_sample_by_pd =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[3];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.enable_inc_min_conf_thres_by_pd =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[4];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_do_not_use_defocus_sample =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[5];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.pd_max_limit_inc_min_conf_thres =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[6];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_preproc.inc_min_conf_thres_by_pd_scaler =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[7];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_int.inc_defocus_thres_prev_error =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[10];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_rgn_thres_multiplier =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[11];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_int.fine_scan_range_enhance =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[12];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.enable_soft_conf_thresh =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[13];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_conf_thresh_delta =
    (int)tuning_info->af_algo.af_haf.pdaf.reserve[14];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_conf_focus_pnt =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[15];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.enable_soft_rgn_thres_high =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[16];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_rgn_thres_high_multiplier =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[17];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_scene_change_cnt =
    af_pdaf_get_soft_scene_change_cnt((float)tuning_info->af_algo.af_haf.pdaf.reserve[18]);
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.is_2PD_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[30];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_min_move_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[31];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_converge_suppress_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[32];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_stable_counter_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[33];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_thresh_x =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[34];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_thresh_y =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[35];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dpd_fallback_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[36];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.touch_pd_off_center_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[37];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[38];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[39];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_filter_len =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[40];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[41];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_len =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[42];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_conf_thres_high =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[43];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_fallback_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[45];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_fallback_roi_size =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[46];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.fast_enter_lsp_in_low_conf_and_hunting_enable=
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[47];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.fast_enter_lsp_in_low_conf_and_hunting_threshold=
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[48];

  /* reserve parameters 40 - 50 for conf_table_high */
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.num_entries =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[50];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[0].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[51];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[0].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[52];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[1].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[53];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[1].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[54];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[2].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[55];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[2].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[56];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[3].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[57];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[3].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[58];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[4].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[59];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[4].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[60];

#else
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_rgn_thres_multiplier =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[11];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.enable_soft_conf_thresh =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[13];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_conf_thresh_delta =
    (int)tuning_info->af_algo.af_haf.pdaf.reserve[14];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_conf_focus_pnt =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[15];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.enable_soft_rgn_thres_high =
    (boolean)tuning_info->af_algo.af_haf.pdaf.reserve[16];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_rgn_thres_high_multiplier =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[17];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.soft_scene_change_cnt =
    af_pdaf_get_soft_scene_change_cnt((float)tuning_info->af_algo.af_haf.pdaf.reserve[18]);
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.is_2PD_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[30];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_min_move_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[31];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_converge_suppress_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[32];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_stable_counter_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[33];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_thresh_x =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[34];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_2pd_roi_move_thresh_y =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[35];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.dpd_fallback_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[36];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.touch_pd_off_center_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[37];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[38];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_thresh =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[39];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conv_done_temporal_filter_len =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[40];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[41];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_len =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[42];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_temporal_conf_thres_high =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[43];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_fallback_enable =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[45];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.face_pd_fallback_roi_size =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[46];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.fast_enter_lsp_in_low_conf_and_hunting_enable=
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[47];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.fast_enter_lsp_in_low_conf_and_hunting_threshold=
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[48];
  
  /* reserve parameters 40 - 50 for conf_table_high */
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.num_entries =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[50];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[0].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[51];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[0].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[52];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[1].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[53];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[1].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[54];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[2].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[55];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[2].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[56];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[3].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[57];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[3].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[58];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[4].noise_gain =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[59];
  tuning_adapter_info->af_algo.af_haf.pdaf.pd_extended.conf_table_high.entries[4].min_conf =
    (float)tuning_info->af_algo.af_haf.pdaf.reserve[60];



#endif

//=============================================================================
// af_tuning_tof_adapter_t:: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  tuning_adapter_info->af_algo.af_haf.tof.outdoor_lux_idx =
    tuning_info->af_algo.af_haf.tof.outdoor_lux_idx;
  tuning_adapter_info->af_algo.af_haf.tof.jump_to_start_limit =
    tuning_info->af_algo.af_haf.tof.jump_to_start_limit;
  tuning_adapter_info->af_algo.af_haf.tof.tof_step_size =
    tuning_info->af_algo.af_haf.tof.tof_step_size;
  tuning_adapter_info->af_algo.af_haf.tof.normal_light_cnt =
    tuning_info->af_algo.af_haf.tof.normal_light_cnt;
  tuning_adapter_info->af_algo.af_haf.tof.lowlight_cnt =
    tuning_info->af_algo.af_haf.tof.lowlight_cnt;
  tuning_adapter_info->af_algo.af_haf.tof.num_monitor_samples =
    tuning_info->af_algo.af_haf.tof.num_monitor_samples;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.tof.tof_extended.resv_enabled =
    tuning_info->af_algo.af_haf.tof.reserve[0];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_near_steps_far_converge_point =
    tuning_info->af_algo.af_haf.tof.reserve[1];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_far_steps_far_converge_point =
    tuning_info->af_algo.af_haf.tof.reserve[2];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.panning_stable_cnt =
    tuning_info->af_algo.af_haf.tof.reserve[3];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.panning_stable_thres =
    tuning_info->af_algo.af_haf.tof.reserve[4];
  tuning_adapter_info->af_algo.af_haf.tof.tof_flag.enable_sad_when_tof_is_conf =
    tuning_info->af_algo.af_haf.tof.reserve[5];
  tuning_adapter_info->af_algo.af_haf.tof.tof_extended.frames_to_wait_stable =
    tuning_info->af_algo.af_haf.tof.reserve[6];
  tuning_adapter_info->af_algo.af_haf.af_gyro.num_of_samples =
    tuning_info->af_algo.af_haf.tof.reserve[7];
  tuning_adapter_info->af_algo.af_haf.tof.sensitivity =
    tuning_info->af_algo.af_haf.tof.reserve[10];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres[AF_DISTANCE_ENTRY_ZERO] =
    tuning_info->af_algo.af_haf.tof.reserve[11];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres[AF_DISTANCE_ENTRY_ONE] =
    tuning_info->af_algo.af_haf.tof.reserve[12];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres[AF_DISTANCE_ENTRY_TWO] =
    tuning_info->af_algo.af_haf.tof.reserve[13];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres[AF_DISTANCE_ENTRY_THREE] =
    tuning_info->af_algo.af_haf.tof.reserve[14];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres[AF_DISTANCE_ENTRY_FOUR] =
    tuning_info->af_algo.af_haf.tof.reserve[15];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_std_dev_thres =
    tuning_info->af_algo.af_haf.tof.reserve[16];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.enable =
    tuning_info->af_algo.af_haf.tof.reserve[17];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.frames_to_wait =
    tuning_info->af_algo.af_haf.tof.reserve[18];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.gain_max =
    tuning_info->af_algo.af_haf.tof.reserve[19];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.gain_min =
    tuning_info->af_algo.af_haf.tof.reserve[20];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.ref_gain_max =
    tuning_info->af_algo.af_haf.tof.reserve[21];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.ref_gain_min =
    tuning_info->af_algo.af_haf.tof.reserve[22];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.ref_threshold_max =
    tuning_info->af_algo.af_haf.tof.reserve[23];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.ref_threshold_min =
    tuning_info->af_algo.af_haf.tof.reserve[24];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.threshold_max =
    tuning_info->af_algo.af_haf.tof.reserve[25];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad.threshold_min =
    tuning_info->af_algo.af_haf.tof.reserve[26];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region[AF_DISTANCE_ENTRY_ZERO] =
    tuning_info->af_algo.af_haf.tof.reserve[27];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region[AF_DISTANCE_ENTRY_ONE] =
    tuning_info->af_algo.af_haf.tof.reserve[28];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region[AF_DISTANCE_ENTRY_TWO] =
    tuning_info->af_algo.af_haf.tof.reserve[29];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region[AF_DISTANCE_ENTRY_THREE] =
    tuning_info->af_algo.af_haf.tof.reserve[30];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region[AF_DISTANCE_ENTRY_FOUR] =
    tuning_info->af_algo.af_haf.tof.reserve[31];
  tuning_adapter_info->af_algo.af_haf.tof.focus_converge.far_converge_point =
    tuning_info->af_algo.af_haf.tof.reserve[32];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.far_distance_fine_step_size =
    tuning_info->af_algo.af_haf.tof.reserve[33];
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.far_distance_unstable_cnt_factor =
    tuning_info->af_algo.af_haf.tof.reserve[34];
  tuning_adapter_info->af_algo.af_haf.tof.focus_converge.jump_to_start_limit_low_light =
    tuning_info->af_algo.af_haf.tof.reserve[35];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.tof_step_size_low_light =
    tuning_info->af_algo.af_haf.tof.reserve[36];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_near_steps_near_direction =
    tuning_info->af_algo.af_haf.tof.reserve[37];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_far_steps_near_direction =
    tuning_info->af_algo.af_haf.tof.reserve[38];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_near_steps_far_direction =
    tuning_info->af_algo.af_haf.tof.reserve[39];
  tuning_adapter_info->af_algo.af_haf.tof.fine_scan.num_far_steps_far_direction =
    tuning_info->af_algo.af_haf.tof.reserve[40];
  tuning_adapter_info->af_algo.af_haf.tof.focus_converge.start_pos_for_tof =
    tuning_info->af_algo.af_haf.tof.reserve[41];
  tuning_adapter_info->af_algo.af_haf.tof.tof_extended.tof_delay_flag_enable =
    tuning_info->af_algo.af_haf.tof.reserve[42];
  tuning_adapter_info->af_algo.af_haf.tof.frame_delay.frame_delay_focus_converge_bright_light =
    tuning_info->af_algo.af_haf.tof.reserve[43];
  tuning_adapter_info->af_algo.af_haf.tof.frame_delay.frame_delay_focus_converge_low_light =
    tuning_info->af_algo.af_haf.tof.reserve[44];
  tuning_adapter_info->af_algo.af_haf.tof.frame_delay.frame_delay_fine_search_bright_light =
    tuning_info->af_algo.af_haf.tof.reserve[45];
  tuning_adapter_info->af_algo.af_haf.tof.frame_delay.frame_delay_fine_search_low_light =
    tuning_info->af_algo.af_haf.tof.reserve[46];
  tuning_adapter_info->af_algo.af_haf.tof.frame_delay.frame_delay_outdoor_light =
    tuning_info->af_algo.af_haf.tof.reserve[47];
  tuning_adapter_info->af_algo.af_haf.tof.tof_flag.aec_wait_check_flag_enable =
    tuning_info->af_algo.af_haf.tof.reserve[48];
  tuning_adapter_info->af_algo.af_haf.tof.tof_flag.use_custom_sensitivity =
    tuning_info->af_algo.af_haf.tof.reserve[49];
  tuning_adapter_info->af_algo.af_haf.tof.tof_filter.median_filter_enable =
    tuning_info->af_algo.af_haf.tof.reserve[50];
  tuning_adapter_info->af_algo.af_haf.tof.tof_filter.median_filter_num_of_samples =
    tuning_info->af_algo.af_haf.tof.reserve[51];
  tuning_adapter_info->af_algo.af_haf.tof.tof_filter.average_filter_enable =
    tuning_info->af_algo.af_haf.tof.reserve[52];
  tuning_adapter_info->af_algo.af_haf.tof.tof_filter.average_filter_num_of_samples =
    tuning_info->af_algo.af_haf.tof.reserve[53];
  tuning_adapter_info->af_algo.af_haf.tof.tof_flag.run_tof_in_outdoor_light_flag =
    tuning_info->af_algo.af_haf.tof.reserve[54];
  tuning_adapter_info->af_algo.af_haf.tof.tof_flag.far_scene_coarse_srch_enable =
    tuning_info->af_algo.af_haf.tof.far_scene_coarse_srch_enable;
  tuning_adapter_info->af_algo.af_haf.tof.focus_converge.jump_to_start_limit =
    tuning_info->af_algo.af_haf.tof.jump_to_start_limit;
#else
  tuning_adapter_info->af_algo.af_haf.tof.sensitivity =
    tuning_info->af_algo.af_haf.tof.sensitivity;
  tuning_adapter_info->af_algo.af_haf.tof.jump_to_start_limit_low_light =
    tuning_info->af_algo.af_haf.tof.jump_to_start_limit_low_light;
  tuning_adapter_info->af_algo.af_haf.tof.start_pos_for_tof =
    tuning_info->af_algo.af_haf.tof.start_pos_for_tof;
  tuning_adapter_info->af_algo.af_haf.tof.far_converge_point =
    tuning_info->af_algo.af_haf.tof.far_converge_point;
  STATIC_ASSERT(sizeof(af_tof_filter_adapter_t) == sizeof(af_tof_filter_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.tof.tof_filter,
    sizeof(af_tof_filter_adapter_t),
    &tuning_info->af_algo.af_haf.tof.tof_filter,
    sizeof(af_tof_filter_t));
  STATIC_ASSERT(sizeof(af_tof_flag_adapter_t) == sizeof(af_tof_flag_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.tof.tof_flag,
    sizeof(af_tof_flag_adapter_t),
    &tuning_info->af_algo.af_haf.tof.tof_flag,
    sizeof(af_tof_flag_t));
  STATIC_ASSERT(sizeof(af_tof_frame_delay_adapter_t) == sizeof(af_tof_frame_delay_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.tof.frame_delay,
    sizeof(af_tof_frame_delay_adapter_t),
    &tuning_info->af_algo.af_haf.tof.frame_delay,
    sizeof(af_tof_frame_delay_t));

  af_copy_tuning_sad(&tuning_adapter_info->af_algo.af_haf.tof.scene_change.af_sad,
                     &tuning_info->af_algo.af_haf.tof.scene_change.af_sad);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres, AF_TOF_MAX_DISTANCE_ENTRIES,
             tuning_info->af_algo.af_haf.tof.scene_change.scene_change_distance_thres, AF_TOF_MAX_DISTANCE_ENTRIES);
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.scene_change_distance_std_dev_thres =
    tuning_info->af_algo.af_haf.tof.scene_change.scene_change_distance_std_dev_thres;
  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.tof.scene_change.distance_region, AF_TOF_MAX_DISTANCE_ENTRIES,
             tuning_info->af_algo.af_haf.tof.scene_change.distance_region, AF_TOF_MAX_DISTANCE_ENTRIES);
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.far_distance_unstable_cnt_factor =
    tuning_info->af_algo.af_haf.tof.scene_change.far_distance_unstable_cnt_factor;
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.panning_stable_cnt =
    tuning_info->af_algo.af_haf.tof.scene_change.panning_stable_cnt;
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.panning_stable_thres =
    tuning_info->af_algo.af_haf.tof.scene_change.panning_stable_thres;
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.normal_light_cnt =
    tuning_info->af_algo.af_haf.tof.scene_change.normal_light_cnt;
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.lowlight_cnt =
    tuning_info->af_algo.af_haf.tof.scene_change.lowlight_cnt;
  tuning_adapter_info->af_algo.af_haf.tof.scene_change.num_monitor_samples =
    tuning_info->af_algo.af_haf.tof.scene_change.num_monitor_samples;

  STATIC_ASSERT(sizeof(af_tof_focus_converge_adapter_t) == sizeof(af_tof_focus_converge_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.tof.focus_converge,
    sizeof(af_tof_focus_converge_adapter_t),
    &tuning_info->af_algo.af_haf.tof.focus_converge,
    sizeof(af_tof_focus_converge_t));
  STATIC_ASSERT(sizeof(af_tof_fine_scan_adapter_t) == sizeof(af_tof_fine_scan_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.tof.fine_scan,
    sizeof(af_tof_fine_scan_adapter_t),
    &tuning_info->af_algo.af_haf.tof.fine_scan,
    sizeof(af_tof_fine_scan_t));
  tuning_adapter_info->af_algo.af_haf.tof.tof_extended.resv_enabled =
    tuning_info->af_algo.af_haf.tof.reserve[0];
  tuning_adapter_info->af_algo.af_haf.tof.tof_extended.frames_to_wait_stable =
    tuning_info->af_algo.af_haf.tof.reserve[6];
  tuning_adapter_info->af_algo.af_haf.tof.tof_extended.tof_delay_flag_enable =
    tuning_info->af_algo.af_haf.tof.reserve[42];
#endif

//=============================================================================
// af_tuning_dciaf_adapter_t:: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  tuning_adapter_info->af_algo.af_haf.dciaf.monitor_enable =
    tuning_info->af_algo.af_haf.dciaf.monitor_enable;
  tuning_adapter_info->af_algo.af_haf.dciaf.monitor_freq =
    tuning_info->af_algo.af_haf.dciaf.monitor_freq;
  tuning_adapter_info->af_algo.af_haf.dciaf.search_freq =
    tuning_info->af_algo.af_haf.dciaf.search_freq;
  tuning_adapter_info->af_algo.af_haf.dciaf.baseline_mm =
    tuning_info->af_algo.af_haf.dciaf.baseline_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.aux_direction =
    (af_cam_orientation_adapter_t)tuning_info->af_algo.af_haf.dciaf.aux_direction;
  tuning_adapter_info->af_algo.af_haf.dciaf.macro_est_limit_cm =
    tuning_info->af_algo.af_haf.dciaf.macro_est_limit_cm;
  tuning_adapter_info->af_algo.af_haf.dciaf.alignment_check_enable =
    tuning_info->af_algo.af_haf.dciaf.alignment_check_enable;
  tuning_adapter_info->af_algo.af_haf.dciaf.jump_to_start_limit =
    tuning_info->af_algo.af_haf.dciaf.jump_to_start_limit;
  tuning_adapter_info->af_algo.af_haf.dciaf.num_near_steps =
    tuning_info->af_algo.af_haf.dciaf.num_near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.num_far_steps =
    tuning_info->af_algo.af_haf.dciaf.num_far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_step_size =
    tuning_info->af_algo.af_haf.dciaf.dciaf_step_size;
  tuning_adapter_info->af_algo.af_haf.dciaf.motion_data_use_enable =
    tuning_info->af_algo.af_haf.dciaf.motion_data_use_enable;
  tuning_adapter_info->af_algo.af_haf.dciaf.scene_change_lens_pos_th =
    tuning_info->af_algo.af_haf.dciaf.scene_change_lens_pos_th;
  tuning_adapter_info->af_algo.af_haf.dciaf.panning_stable_lens_pos_th =
    tuning_info->af_algo.af_haf.dciaf.panning_stable_lens_pos_th;
  tuning_adapter_info->af_algo.af_haf.dciaf.panning_stable_duration_ms_bright =
    tuning_info->af_algo.af_haf.dciaf.panning_stable_duration_ms_bright;
  tuning_adapter_info->af_algo.af_haf.dciaf.panning_stable_duration_ms_low =
    tuning_info->af_algo.af_haf.dciaf.panning_stable_duration_ms_low;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x0309)
  tuning_adapter_info->af_algo.af_haf.dciaf.maxLowLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.maxLowLightWaitMs;
  tuning_adapter_info->af_algo.af_haf.dciaf.maxNormalLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.maxNormalLightWaitMs;
  tuning_adapter_info->af_algo.af_haf.dciaf.jumpWithNoSkip =
    tuning_info->af_algo.af_haf.dciaf.jumpWithNoSkip;
  tuning_adapter_info->af_algo.af_haf.dciaf.SkipframesBeforeFineSearch =
    tuning_info->af_algo.af_haf.dciaf.SkipframesBeforeFineSearch;
  tuning_adapter_info->af_algo.af_haf.dciaf.dist_thresh_macro_region_in_mm =
    tuning_info->af_algo.af_haf.dciaf.dist_thresh_macro_region_in_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.dist_thresh_non_macro_factor =
    tuning_info->af_algo.af_haf.dciaf.dist_thresh_non_macro_factor;
  tuning_adapter_info->af_algo.af_haf.dciaf.lowLightThreshold =
    tuning_info->af_algo.af_haf.dciaf.lowLightThreshold;
  tuning_adapter_info->af_algo.af_haf.dciaf.useFineSearchExtension =
    tuning_info->af_algo.af_haf.dciaf.useFineSearchExtension;
  tuning_adapter_info->af_algo.af_haf.dciaf.macroRegionThreshold_in_mm =
    tuning_info->af_algo.af_haf.dciaf.macroRegionThreshold_in_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.farRegionThresholdin_mm =
    tuning_info->af_algo.af_haf.dciaf.farRegionThresholdin_mm;

  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo, Q3A_AF_DCIAF_MAX_RANGE,
             tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo, AF_DCIAF_MAX_RANGE);

  COPY_ARRAY(tuning_adapter_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo, Q3A_AF_DCIAF_MAX_RANGE,
             tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo, AF_DCIAF_MAX_RANGE);
#endif

// reserve/dciaf_extended : af_tuning_dciaf_adapter_t:: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.enable =
    tuning_info->af_algo.af_haf.dciaf.reserve[0];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.maxLowLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.reserve[1];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.maxNormalLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.reserve[2];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.jumpWithNoSkip =
    tuning_info->af_algo.af_haf.dciaf.reserve[3];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.SkipframesBeforeFineSearch =
    tuning_info->af_algo.af_haf.dciaf.reserve[4];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.dist_thresh_macro_region_in_mm =
    tuning_info->af_algo.af_haf.dciaf.reserve[5];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.dist_thresh_non_macro_factor =
    tuning_info->af_algo.af_haf.dciaf.reserve[6];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightThreshold =
    tuning_info->af_algo.af_haf.dciaf.reserve[7];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.useFineSearchExtension =
    tuning_info->af_algo.af_haf.dciaf.reserve[8];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.macroRegionThreshold_in_mm =
    tuning_info->af_algo.af_haf.dciaf.reserve[9];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.farRegionThresholdin_mm =
    tuning_info->af_algo.af_haf.dciaf.reserve[10];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.reserve[11];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[12];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[13];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.reserve[14];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[15];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[16];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.reserve[17];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[18];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[19];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.reserve[20];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[21];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[22];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.reserve[23];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[24];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[25];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.reserve[26];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[27];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.reserve[28];
#else
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.enable =
    tuning_info->af_algo.af_haf.dciaf.reserve[0];
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.maxLowLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.maxLowLightWaitMs;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.maxNormalLightWaitMs =
    tuning_info->af_algo.af_haf.dciaf.maxNormalLightWaitMs;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.jumpWithNoSkip =
    tuning_info->af_algo.af_haf.dciaf.jumpWithNoSkip;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.SkipframesBeforeFineSearch =
    tuning_info->af_algo.af_haf.dciaf.SkipframesBeforeFineSearch;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.dist_thresh_macro_region_in_mm =
    tuning_info->af_algo.af_haf.dciaf.dist_thresh_macro_region_in_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.dist_thresh_non_macro_factor =
    tuning_info->af_algo.af_haf.dciaf.dist_thresh_non_macro_factor;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightThreshold =
    tuning_info->af_algo.af_haf.dciaf.lowLightThreshold;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.useFineSearchExtension =
    tuning_info->af_algo.af_haf.dciaf.useFineSearchExtension;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.macroRegionThreshold_in_mm =
    tuning_info->af_algo.af_haf.dciaf.macroRegionThreshold_in_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.farRegionThresholdin_mm =
    tuning_info->af_algo.af_haf.dciaf.farRegionThresholdin_mm;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MID_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MID_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_MID_RANGE].near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_FAR_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_FAR_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.normalLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.normalLightFineSearchInfo[AF_DCIAF_FAR_RANGE].near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MACRO_RANGE].near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MID_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MID_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_MID_RANGE].near_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_FAR_RANGE].stepSize;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_FAR_RANGE].far_steps;
  tuning_adapter_info->af_algo.af_haf.dciaf.dciaf_extended.lowLightFineSearchInfo[Q3A_AF_DCIAF_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.dciaf.lowLightFineSearchInfo[AF_DCIAF_FAR_RANGE].near_steps;
#endif

//=============================================================================
// af_fine_srch_extension_t: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
// af_tuning_sad_t: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
// af_tuning_gyro_t: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x0309)
  STATIC_ASSERT(sizeof(af_fine_srch_extension_adapter_t) == sizeof(af_fine_srch_extension_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.fine_srch_extension,
    sizeof(af_fine_srch_extension_adapter_t),
    &tuning_info->af_algo.af_haf.fine_srch_extension,
    sizeof(af_fine_srch_extension_t));

  af_copy_tuning_sad(&tuning_adapter_info->af_algo.af_haf.af_sad,
                     &tuning_info->af_algo.af_haf.af_sad);

  STATIC_ASSERT(sizeof(af_tuning_gyro_adapter_t) == sizeof(af_tuning_gyro_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_haf.af_gyro,
    sizeof(af_tuning_gyro_adapter_t),
    &tuning_info->af_algo.af_haf.af_gyro,
    sizeof(af_tuning_gyro_t));
  tuning_adapter_info->af_algo.af_haf.scene_change_during_FS =
    tuning_info->af_algo.af_haf.scene_change_during_FS;
  tuning_adapter_info->af_algo.af_haf.face_caf_high_priority_enable =
    tuning_info->af_algo.af_haf.face_caf_high_priority_enable;
#endif

//=============================================================================
// reserve: af_tuning_haf_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  tuning_adapter_info->af_algo.af_haf.haf_extended.max_fine_srch_extension_cnt =
    tuning_info->af_algo.af_haf.reserve[0];
  tuning_adapter_info->af_algo.af_haf.haf_extended.num_near_steps =
    tuning_info->af_algo.af_haf.reserve[1];
  tuning_adapter_info->af_algo.af_haf.haf_extended.num_far_steps =
    tuning_info->af_algo.af_haf.reserve[2];
  tuning_adapter_info->af_algo.af_haf.haf_extended.step_size =
    tuning_info->af_algo.af_haf.reserve[3];
  tuning_adapter_info->af_algo.af_haf.haf_extended.decrease_drop_ratio =
    tuning_info->af_algo.af_haf.reserve[4];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_init_pos =
    tuning_info->af_algo.af_haf.reserve[5];
  tuning_adapter_info->af_algo.af_haf.haf_extended.idx =
    tuning_info->af_algo.af_haf.reserve[6];
  tuning_adapter_info->af_algo.af_haf.haf_extended.depth_conf_cnt_th =
    tuning_info->af_algo.af_haf.reserve[7];
  tuning_adapter_info->af_algo.af_haf.haf_extended.spotlight_enabled =
    tuning_info->af_algo.af_haf.reserve[10];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_INF_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[11];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_MARCO_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[12];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_INC_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[13];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_DEC_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[14];
  tuning_adapter_info->af_algo.af_haf.haf_extended.exp_index_th =
    tuning_info->af_algo.af_haf.reserve[15];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_start_h =
    tuning_info->af_algo.af_haf.reserve[16];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_end_h =
    tuning_info->af_algo.af_haf.reserve[17];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_start_v =
    tuning_info->af_algo.af_haf.reserve[18];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_end_v =
    tuning_info->af_algo.af_haf.reserve[19];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_start_h =
    tuning_info->af_algo.af_haf.reserve[20];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_end_h =
    tuning_info->af_algo.af_haf.reserve[21];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_start_v =
    tuning_info->af_algo.af_haf.reserve[22];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_end_v =
    tuning_info->af_algo.af_haf.reserve[23];
  tuning_adapter_info->af_algo.af_haf.haf_extended.bright_region_luma_delta_background =
    tuning_info->af_algo.af_haf.reserve[24];
  tuning_adapter_info->af_algo.af_haf.haf_extended.bright_region_luma_delta_spotlight_zone_low_grey =
    tuning_info->af_algo.af_haf.reserve[25];
  tuning_adapter_info->af_algo.af_haf.haf_extended.bright_region_luma_delta_spotlight_zone_dark_grey =
    tuning_info->af_algo.af_haf.reserve[26];
  tuning_adapter_info->af_algo.af_haf.haf_extended.region_low_grey_th =
    tuning_info->af_algo.af_haf.reserve[27];
  tuning_adapter_info->af_algo.af_haf.haf_extended.region_dark_grey_th =
    tuning_info->af_algo.af_haf.reserve[28];
  tuning_adapter_info->af_algo.af_haf.haf_extended.background_low_grey_pct_th =
    tuning_info->af_algo.af_haf.reserve[29];
  tuning_adapter_info->af_algo.af_haf.haf_extended.background_dark_grey_pct_th =
    tuning_info->af_algo.af_haf.reserve[30];
  tuning_adapter_info->af_algo.af_haf.haf_extended.background_luma_ave_th =
    tuning_info->af_algo.af_haf.reserve[31];
  tuning_adapter_info->af_algo.af_haf.haf_extended.square_cluster_increase_size =
    tuning_info->af_algo.af_haf.reserve[32];
  tuning_adapter_info->af_algo.af_haf.haf_extended.square_cluster_pct_th =
    tuning_info->af_algo.af_haf.reserve[33];
  tuning_adapter_info->af_algo.af_haf.haf_extended.vertical_cluster_increase_size =
    tuning_info->af_algo.af_haf.reserve[34];
  tuning_adapter_info->af_algo.af_haf.haf_extended.vertical_cluster_pct_th =
    tuning_info->af_algo.af_haf.reserve[35];
  tuning_adapter_info->af_algo.af_haf.haf_extended.horizontal_cluster_increase_size =
    tuning_info->af_algo.af_haf.reserve[36];
  tuning_adapter_info->af_algo.af_haf.haf_extended.horizontal_cluster_pct_th =
    tuning_info->af_algo.af_haf.reserve[37];
  tuning_adapter_info->af_algo.af_haf.haf_extended.spotlight_detect_cnt_th =
    tuning_info->af_algo.af_haf.reserve[38];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_enable =
    tuning_info->af_algo.af_haf.reserve[40];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_h_num =
    tuning_info->af_algo.af_haf.reserve[41];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_v_num =
    tuning_info->af_algo.af_haf.reserve[42];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_center_roi_size =
    tuning_info->af_algo.af_haf.reserve[43];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_corner_roi_size =
    tuning_info->af_algo.af_haf.reserve[44];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_delay =
    tuning_info->af_algo.af_haf.reserve[45];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enableROIShifting =
    tuning_info->af_algo.af_haf.reserve[46];
  tuning_adapter_info->af_algo.af_haf.haf_extended.roi_shift_type =
    (af_roi_shift_type)(int)tuning_info->af_algo.af_haf.reserve[47];
  tuning_adapter_info->af_algo.af_haf.follower.enable =
    tuning_info->af_algo.af_haf.reserve[48];
  tuning_adapter_info->af_algo.af_haf.follower.gravity_comp_on =
    tuning_info->af_algo.af_haf.reserve[49];
  tuning_adapter_info->af_algo.af_haf.follower.jump_to_start_limit =
    tuning_info->af_algo.af_haf.reserve[50];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightThreshold =
    tuning_info->af_algo.af_haf.reserve[51];
  tuning_adapter_info->af_algo.af_haf.follower.useFineSearchExtension =
    tuning_info->af_algo.af_haf.reserve[52];
  tuning_adapter_info->af_algo.af_haf.follower.activeConvergenceFollow =
    tuning_info->af_algo.af_haf.reserve[53];
  tuning_adapter_info->af_algo.af_haf.follower.lensPosThresholdForActiveConvergenceFollow =
    tuning_info->af_algo.af_haf.reserve[54];
  tuning_adapter_info->af_algo.af_haf.follower.macroRegionThreshold_in_mm =
    tuning_info->af_algo.af_haf.reserve[55];
  tuning_adapter_info->af_algo.af_haf.follower.farRegionThresholdin_mm =
    tuning_info->af_algo.af_haf.reserve[56];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[57];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[58];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[59];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[60];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[61];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[62];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[63];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[64];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[65];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[66];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[67];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[68];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[69];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[70];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[71];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[72];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[73];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[74];
  tuning_adapter_info->af_algo.af_haf.haf_extended.fine_start_move_buffer =
   (int)tuning_info->af_algo.af_haf.reserve[75];
  tuning_adapter_info->af_algo.af_haf.haf_extended.dof_multiplier =
    tuning_info->af_algo.af_haf.reserve[76];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_force_scan_end =
    (boolean)tuning_info->af_algo.af_haf.reserve[77];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_passive_scan_timeout =
    (boolean)tuning_info->af_algo.af_haf.reserve[78];
  tuning_adapter_info->af_algo.af_haf.haf_extended.max_scan_cnt_th =
    (int)af_haf_fill_force_scan_cnt_th((int)tuning_info->af_algo.af_haf.reserve[79]);
#else
  tuning_adapter_info->af_algo.af_haf.haf_extended.max_fine_srch_extension_cnt =
    tuning_info->af_algo.af_haf.reserve[0];
  tuning_adapter_info->af_algo.af_haf.haf_extended.num_near_steps =
    tuning_info->af_algo.af_haf.reserve[1];
  tuning_adapter_info->af_algo.af_haf.haf_extended.num_far_steps =
    tuning_info->af_algo.af_haf.reserve[2];
  tuning_adapter_info->af_algo.af_haf.haf_extended.step_size =
    tuning_info->af_algo.af_haf.reserve[3];
  tuning_adapter_info->af_algo.af_haf.haf_extended.decrease_drop_ratio =
    tuning_info->af_algo.af_haf.reserve[4];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_init_pos =
    tuning_info->af_algo.af_haf.reserve[5];
  tuning_adapter_info->af_algo.af_haf.haf_extended.idx =
    tuning_info->af_algo.af_haf.reserve[6];
  tuning_adapter_info->af_algo.af_haf.haf_extended.depth_conf_cnt_th =
    tuning_info->af_algo.af_haf.reserve[7];
  tuning_adapter_info->af_algo.af_haf.haf_extended.spotlight_enabled =
    tuning_info->af_algo.af_haf.reserve[10];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_INF_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[11];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_MARCO_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[12];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_INC_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[13];
  tuning_adapter_info->af_algo.af_haf.haf_extended.AF_SPOT_LIGHT_DEC_FLAT_THRS =
    tuning_info->af_algo.af_haf.reserve[14];
  tuning_adapter_info->af_algo.af_haf.haf_extended.exp_index_th =
    tuning_info->af_algo.af_haf.reserve[15];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_start_h =
    tuning_info->af_algo.af_haf.reserve[16];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_end_h =
    tuning_info->af_algo.af_haf.reserve[17];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_start_v =
    tuning_info->af_algo.af_haf.reserve[18];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_bright_region_zone_end_v =
    tuning_info->af_algo.af_haf.reserve[19];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_start_h =
    tuning_info->af_algo.af_haf.reserve[20];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_end_h =
    tuning_info->af_algo.af_haf.reserve[21];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_start_v =
    tuning_info->af_algo.af_haf.reserve[22];
  tuning_adapter_info->af_algo.af_haf.haf_extended.af_background_zone_end_v =
    tuning_info->af_algo.af_haf.reserve[23];
  tuning_adapter_info->af_algo.af_haf.haf_extended.bright_region_luma_delta_background =
    tuning_info->af_algo.af_haf.reserve[24];
  tuning_adapter_info->af_algo.af_haf.haf_extended.bright_region_luma_delta_spotlight_zone_low_grey =
    tuning_info->af_algo.af_haf.reserve[25];
  tuning_adapter_info->af_algo.af_haf.haf_extended.bright_region_luma_delta_spotlight_zone_dark_grey =
    tuning_info->af_algo.af_haf.reserve[26];
  tuning_adapter_info->af_algo.af_haf.haf_extended.region_low_grey_th =
    tuning_info->af_algo.af_haf.reserve[27];
  tuning_adapter_info->af_algo.af_haf.haf_extended.region_dark_grey_th =
    tuning_info->af_algo.af_haf.reserve[28];
  tuning_adapter_info->af_algo.af_haf.haf_extended.background_low_grey_pct_th =
    tuning_info->af_algo.af_haf.reserve[29];
  tuning_adapter_info->af_algo.af_haf.haf_extended.background_dark_grey_pct_th =
    tuning_info->af_algo.af_haf.reserve[30];
  tuning_adapter_info->af_algo.af_haf.haf_extended.background_luma_ave_th =
    tuning_info->af_algo.af_haf.reserve[31];
  tuning_adapter_info->af_algo.af_haf.haf_extended.square_cluster_increase_size =
    tuning_info->af_algo.af_haf.reserve[32];
  tuning_adapter_info->af_algo.af_haf.haf_extended.square_cluster_pct_th =
    tuning_info->af_algo.af_haf.reserve[33];
  tuning_adapter_info->af_algo.af_haf.haf_extended.vertical_cluster_increase_size =
    tuning_info->af_algo.af_haf.reserve[34];
  tuning_adapter_info->af_algo.af_haf.haf_extended.vertical_cluster_pct_th =
    tuning_info->af_algo.af_haf.reserve[35];
  tuning_adapter_info->af_algo.af_haf.haf_extended.horizontal_cluster_increase_size =
    tuning_info->af_algo.af_haf.reserve[36];
  tuning_adapter_info->af_algo.af_haf.haf_extended.horizontal_cluster_pct_th =
    tuning_info->af_algo.af_haf.reserve[37];
  tuning_adapter_info->af_algo.af_haf.haf_extended.spotlight_detect_cnt_th =
    tuning_info->af_algo.af_haf.reserve[38];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_enable =
    tuning_info->af_algo.af_haf.reserve[40];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_h_num =
    tuning_info->af_algo.af_haf.reserve[41];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_v_num =
    tuning_info->af_algo.af_haf.reserve[42];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_center_roi_size =
    tuning_info->af_algo.af_haf.reserve[43];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_corner_roi_size =
    tuning_info->af_algo.af_haf.reserve[44];
  tuning_adapter_info->af_algo.af_haf.haf_extended.haf_roi_delay =
    tuning_info->af_algo.af_haf.reserve[45];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enableROIShifting =
    tuning_info->af_algo.af_haf.reserve[46];
  tuning_adapter_info->af_algo.af_haf.haf_extended.roi_shift_type =
    tuning_info->af_algo.af_haf.reserve[47];
  tuning_adapter_info->af_algo.af_haf.follower.enable =
    tuning_info->af_algo.af_haf.reserve[48];
  tuning_adapter_info->af_algo.af_haf.follower.gravity_comp_on =
    tuning_info->af_algo.af_haf.reserve[49];
  tuning_adapter_info->af_algo.af_haf.follower.jump_to_start_limit =
    tuning_info->af_algo.af_haf.reserve[50];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightThreshold =
    tuning_info->af_algo.af_haf.reserve[51];
  tuning_adapter_info->af_algo.af_haf.follower.useFineSearchExtension =
    tuning_info->af_algo.af_haf.reserve[52];
  tuning_adapter_info->af_algo.af_haf.follower.activeConvergenceFollow =
    tuning_info->af_algo.af_haf.reserve[53];
  tuning_adapter_info->af_algo.af_haf.follower.lensPosThresholdForActiveConvergenceFollow =
    tuning_info->af_algo.af_haf.reserve[54];
  tuning_adapter_info->af_algo.af_haf.follower.macroRegionThreshold_in_mm =
    tuning_info->af_algo.af_haf.reserve[55];
  tuning_adapter_info->af_algo.af_haf.follower.farRegionThresholdin_mm =
    tuning_info->af_algo.af_haf.reserve[56];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[57];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[58];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[59];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[60];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[61];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[62];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[63];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[64];
  tuning_adapter_info->af_algo.af_haf.follower.normalLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[65];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[66];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[67];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MACRO_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[68];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[69];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[70];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_MID_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[71];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].stepSize =
    tuning_info->af_algo.af_haf.reserve[72];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].far_steps =
    tuning_info->af_algo.af_haf.reserve[73];
  tuning_adapter_info->af_algo.af_haf.follower.lowLightFineSearchInfo[AF_FOLLOWER_FAR_RANGE].near_steps =
    tuning_info->af_algo.af_haf.reserve[74];
  tuning_adapter_info->af_algo.af_haf.haf_extended.fine_start_move_buffer =
   (int)tuning_info->af_algo.af_haf.reserve[75];
  tuning_adapter_info->af_algo.af_haf.haf_extended.dof_multiplier =
    tuning_info->af_algo.af_haf.reserve[76];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_force_scan_end =
    (boolean)tuning_info->af_algo.af_haf.reserve[77];
  tuning_adapter_info->af_algo.af_haf.haf_extended.enable_passive_scan_timeout =
    (boolean)tuning_info->af_algo.af_haf.reserve[78];
  tuning_adapter_info->af_algo.af_haf.haf_extended.max_scan_cnt_th =
    (int)af_haf_fill_force_scan_cnt_th((int)tuning_info->af_algo.af_haf.reserve[79]);
#endif

//=============================================================================
// af_tuning_continuous_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  tuning_adapter_info->af_algo.af_cont.enable =
    tuning_info->af_algo.af_cont.enable;
  tuning_adapter_info->af_algo.af_cont.scene_change_detection_ratio =
    tuning_info->af_algo.af_cont.scene_change_detection_ratio;
  tuning_adapter_info->af_algo.af_cont.panning_stable_fv_change_trigger =
    tuning_info->af_algo.af_cont.panning_stable_fv_change_trigger;
  tuning_adapter_info->af_algo.af_cont.panning_stable_fvavg_to_fv_change_trigger =
    tuning_info->af_algo.af_cont.panning_stable_fvavg_to_fv_change_trigger;
  tuning_adapter_info->af_algo.af_cont.panning_unstable_trigger_cnt =
    tuning_info->af_algo.af_cont.panning_unstable_trigger_cnt;
  tuning_adapter_info->af_algo.af_cont.panning_stable_trigger_cnt =
    tuning_info->af_algo.af_cont.panning_stable_trigger_cnt;
  tuning_adapter_info->af_algo.af_cont.downhill_allowance =
    tuning_info->af_algo.af_cont.downhill_allowance;
  tuning_adapter_info->af_algo.af_cont.uphill_allowance =
    tuning_info->af_algo.af_cont.uphill_allowance;
  tuning_adapter_info->af_algo.af_cont.base_frame_delay =
    tuning_info->af_algo.af_cont.base_frame_delay;
  tuning_adapter_info->af_algo.af_cont.scene_change_luma_threshold =
    tuning_info->af_algo.af_cont.scene_change_luma_threshold;
  tuning_adapter_info->af_algo.af_cont.luma_settled_threshold =
    tuning_info->af_algo.af_cont.luma_settled_threshold;
  tuning_adapter_info->af_algo.af_cont.noise_level_th =
    tuning_info->af_algo.af_cont.noise_level_th;
  tuning_adapter_info->af_algo.af_cont.search_step_size =
    tuning_info->af_algo.af_cont.search_step_size;
  tuning_adapter_info->af_algo.af_cont.init_search_type =
    (af_algo_adapter_t)tuning_info->af_algo.af_cont.init_search_type;
  tuning_adapter_info->af_algo.af_cont.search_type =
    (af_algo_adapter_t)tuning_info->af_algo.af_cont.search_type;
  tuning_adapter_info->af_algo.af_cont.low_light_wait =
    tuning_info->af_algo.af_cont.low_light_wait;
  tuning_adapter_info->af_algo.af_cont.max_indecision_cnt =
    tuning_info->af_algo.af_cont.max_indecision_cnt;
  tuning_adapter_info->af_algo.af_cont.flat_fv_confidence_level =
    tuning_info->af_algo.af_cont.flat_fv_confidence_level;

  af_copy_tuning_sad(&tuning_adapter_info->af_algo.af_cont.af_sad,
                     &tuning_info->af_algo.af_cont.af_sad);

  tuning_adapter_info->af_algo.af_cont.af_gyro.enable =
    tuning_info->af_algo.af_cont.af_gyro.enable;
  tuning_adapter_info->af_algo.af_cont.af_gyro.min_movement_threshold =
    tuning_info->af_algo.af_cont.af_gyro.min_movement_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.stable_detected_threshold =
    tuning_info->af_algo.af_cont.af_gyro.stable_detected_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.unstable_count_th =
    tuning_info->af_algo.af_cont.af_gyro.unstable_count_th;
  tuning_adapter_info->af_algo.af_cont.af_gyro.stable_count_th =
    tuning_info->af_algo.af_cont.af_gyro.stable_count_th;
  tuning_adapter_info->af_algo.af_cont.af_gyro.fast_pan_threshold =
    tuning_info->af_algo.af_cont.af_gyro.fast_pan_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.slow_pan_threshold =
    tuning_info->af_algo.af_cont.af_gyro.slow_pan_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.fast_pan_count_threshold =
    tuning_info->af_algo.af_cont.af_gyro.fast_pan_count_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.sum_return_to_orig_pos_threshold =
    tuning_info->af_algo.af_cont.af_gyro.sum_return_to_orig_pos_threshold;
  tuning_adapter_info->af_algo.af_cont.af_gyro.stable_count_delay =
    tuning_info->af_algo.af_cont.af_gyro.stable_count_delay;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  tuning_adapter_info->af_algo.af_cont.af_gyro.num_of_samples =
    tuning_info->af_algo.af_cont.af_gyro.num_of_samples;
#endif

  tuning_adapter_info->af_algo.af_cont.af_chroma.enable =
    tuning_info->af_algo.af_cont.af_chroma.enable;
  tuning_adapter_info->af_algo.af_cont.af_chroma.g_over_b_thres =
    tuning_info->af_algo.af_cont.af_chroma.g_over_b_thres;
  tuning_adapter_info->af_algo.af_cont.af_chroma.g_over_r_thres =
    tuning_info->af_algo.af_cont.af_chroma.g_over_r_thres;
  COPY_ARRAY(tuning_adapter_info->af_algo.af_cont.af_chroma.aec_gain_input, AF_SCENE_CHANGE_MAX_ENTRIES,
             tuning_info->af_algo.af_cont.af_chroma.aec_gain_input, AF_SCENE_CHANGE_MAX_ENTRIES);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_cont.af_chroma.g_over_b_threshold_scaler, AF_SCENE_CHANGE_MAX_ENTRIES,
             tuning_info->af_algo.af_cont.af_chroma.g_over_b_threshold_scaler, AF_SCENE_CHANGE_MAX_ENTRIES);
  COPY_ARRAY(tuning_adapter_info->af_algo.af_cont.af_chroma.g_over_r_threshold_scaler, AF_SCENE_CHANGE_MAX_ENTRIES,
             tuning_info->af_algo.af_cont.af_chroma.g_over_r_threshold_scaler, AF_SCENE_CHANGE_MAX_ENTRIES);
  tuning_adapter_info->af_algo.af_cont.af_chroma.unstable_count_th =
    tuning_info->af_algo.af_cont.af_chroma.unstable_count_th;
  tuning_adapter_info->af_algo.af_cont.af_chroma.num_of_samples =
    tuning_info->af_algo.af_cont.af_chroma.num_of_samples;

  tuning_adapter_info->af_algo.af_cont.af_spot_light.enable =
    tuning_info->af_algo.af_cont.af_spot_light.enable;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.far_flat_th =
    tuning_info->af_algo.af_cont.af_spot_light.far_flat_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.marco_flat_th =
    tuning_info->af_algo.af_cont.af_spot_light.marco_flat_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.inc_flat_th =
    tuning_info->af_algo.af_cont.af_spot_light.inc_flat_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.dec_flat_th =
    tuning_info->af_algo.af_cont.af_spot_light.dec_flat_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.spotlight_zone_start_h =
    tuning_info->af_algo.af_cont.af_spot_light.spotlight_zone_start_h;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.spotlight_zone_end_h =
    tuning_info->af_algo.af_cont.af_spot_light.spotlight_zone_end_h;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.spotlight_zone_start_v =
    tuning_info->af_algo.af_cont.af_spot_light.spotlight_zone_start_v;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.spotlight_zone_end_v =
    tuning_info->af_algo.af_cont.af_spot_light.spotlight_zone_end_v;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_zone_start_h =
    tuning_info->af_algo.af_cont.af_spot_light.background_zone_start_h;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_zone_end_h =
    tuning_info->af_algo.af_cont.af_spot_light.background_zone_end_h;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_zone_start_v =
    tuning_info->af_algo.af_cont.af_spot_light.background_zone_start_v;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_zone_end_v =
    tuning_info->af_algo.af_cont.af_spot_light.background_zone_end_v;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.exposure_index_trigger_th =
    tuning_info->af_algo.af_cont.af_spot_light.exposure_index_trigger_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_luma_avg_th =
    tuning_info->af_algo.af_cont.af_spot_light.background_luma_avg_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.region_low_grey_th =
    tuning_info->af_algo.af_cont.af_spot_light.region_low_grey_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.region_low_dark_th =
    tuning_info->af_algo.af_cont.af_spot_light.region_low_dark_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_low_grey_pct_th =
    tuning_info->af_algo.af_cont.af_spot_light.background_low_grey_pct_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.background_dark_grey_pct_th =
    tuning_info->af_algo.af_cont.af_spot_light.background_dark_grey_pct_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_background_zone =
    tuning_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_background_zone;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_spotlight_zone_low_grey =
    tuning_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_spotlight_zone_low_grey;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_spotlight_zone_dark_grey =
    tuning_info->af_algo.af_cont.af_spot_light.bright_region_luma_delta_spotlight_zone_dark_grey;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.square_cluster_inc_size =
    tuning_info->af_algo.af_cont.af_spot_light.square_cluster_inc_size;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.vertical_cluster_inc_size =
    tuning_info->af_algo.af_cont.af_spot_light.vertical_cluster_inc_size;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.horizontal_cluster_inc_size =
    tuning_info->af_algo.af_cont.af_spot_light.horizontal_cluster_inc_size;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.square_cluster_pct_th =
    tuning_info->af_algo.af_cont.af_spot_light.square_cluster_pct_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.vertical_cluster_pct_th =
    tuning_info->af_algo.af_cont.af_spot_light.vertical_cluster_pct_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.horizontal_cluster_pct_th =
    tuning_info->af_algo.af_cont.af_spot_light.horizontal_cluster_pct_th;
  tuning_adapter_info->af_algo.af_cont.af_spot_light.spotlight_detect_cnt_th =
    tuning_info->af_algo.af_cont.af_spot_light.spotlight_detect_cnt_th;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x0309)
  tuning_adapter_info->af_algo.af_cont.af_caf_trigger_after_taf =
    tuning_info->af_algo.af_cont.af_caf_trigger_after_taf;
#endif

//=============================================================================
// af_tuning_continuous_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  STATIC_ASSERT(sizeof(af_tuning_exhaustive_adapter_t) == sizeof(af_tuning_exhaustive_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_exh,
    sizeof(af_tuning_exhaustive_adapter_t),
    &tuning_info->af_algo.af_exh,
    sizeof(af_tuning_exhaustive_t));

//=============================================================================
// af_tuning_fullsweep_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  tuning_adapter_info->af_algo.af_full.num_steps_between_stat_points =
    tuning_info->af_algo.af_full.num_steps_between_stat_points;
  tuning_adapter_info->af_algo.af_full.frame_delay_inf =
    tuning_info->af_algo.af_full.frame_delay_inf;
  tuning_adapter_info->af_algo.af_full.frame_delay_norm =
    tuning_info->af_algo.af_full.frame_delay_norm;
  tuning_adapter_info->af_algo.af_full.frame_delay_final =
    tuning_info->af_algo.af_full.frame_delay_final;

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x0309)
  tuning_adapter_info->af_algo.af_full.PAAF_enable =
    tuning_info->af_algo.af_full.PAAF_enable;
#endif

//=============================================================================
// af_tuning_sp_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  STATIC_ASSERT(sizeof(af_tuning_sp_adapter_t) == sizeof(af_tuning_sp_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_sp,
    sizeof(af_tuning_sp_adapter_t),
    &tuning_info->af_algo.af_sp,
    sizeof(af_tuning_sp_t));

//=============================================================================
// af_tuning_single_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  COPY_ARRAY(tuning_adapter_info->af_algo.af_single.index, SINGLE_MAX_IDX,
             tuning_info->af_algo.af_single.index, SINGLE_MAX_IDX);
  tuning_adapter_info->af_algo.af_single.actuator_type =
    tuning_info->af_algo.af_single.actuator_type;
  tuning_adapter_info->af_algo.af_single.is_hys_comp_needed =
    tuning_info->af_algo.af_single.is_hys_comp_needed;
  tuning_adapter_info->af_algo.af_single.step_index_per_um =
    tuning_info->af_algo.af_single.step_index_per_um;
  STATIC_ASSERT(sizeof(af_step_size_table_adapter_t) == sizeof(step_size_table_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_single.TAF_step_table,
    sizeof(af_step_size_table_adapter_t),
    &tuning_info->af_algo.af_single.TAF_step_table,
    sizeof(step_size_table_t));
  STATIC_ASSERT(sizeof(af_step_size_table_adapter_t) == sizeof(step_size_table_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_single.CAF_step_table,
    sizeof(af_step_size_table_adapter_t),
    &tuning_info->af_algo.af_single.CAF_step_table,
    sizeof(step_size_table_t));
  tuning_adapter_info->af_algo.af_single.PAAF_enable =
    tuning_info->af_algo.af_single.PAAF_enable;
  af_copy_single_threshold(&tuning_adapter_info->af_algo.af_single.sw,
                           &tuning_info->af_algo.af_single.sw);
  af_copy_single_threshold(&tuning_adapter_info->af_algo.af_single.hw,
                           &tuning_info->af_algo.af_single.hw);;
  COPY_ARRAY(tuning_adapter_info->af_algo.af_single.BV_gain, AF_BV_COUNT,
             tuning_info->af_algo.af_single.BV_gain, 8);
  STATIC_ASSERT(sizeof(single_optic_adapter_t) == sizeof(single_optic_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_single.optics,
    sizeof(single_optic_adapter_t),
    &tuning_info->af_algo.af_single.optics,
    sizeof(single_optic_t));
  STATIC_ASSERT(sizeof(FVscore_threshold_adapter_t) == sizeof(FVscore_threshold_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_single.score_thresh,
    sizeof(FVscore_threshold_adapter_t),
    &tuning_info->af_algo.af_single.score_thresh,
    sizeof(FVscore_threshold_t));
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION != 0x0309)
  STATIC_ASSERT(sizeof(af_single_frame_delay_adapter_t) == sizeof(af_single_frame_delay_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_single.frame_delay,
    sizeof(af_single_frame_delay_adapter_t),
    &tuning_info->af_algo.af_single.frame_delay,
    sizeof(af_single_frame_delay_t));
#endif

//=============================================================================
// af_shake_resistant_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  STATIC_ASSERT(sizeof(af_shake_resistant_adapter_t) == sizeof(af_shake_resistant_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_shake_resistant,
    sizeof(af_shake_resistant_adapter_t),
    &tuning_info->af_algo.af_shake_resistant,
    sizeof(af_shake_resistant_t));

//=============================================================================
// af_motion_sensor_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  STATIC_ASSERT(sizeof(af_motion_sensor_adapter_t) == sizeof(af_motion_sensor_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.af_motion_sensor,
    sizeof(af_motion_sensor_adapter_t),
    &tuning_info->af_algo.af_motion_sensor,
    sizeof(af_motion_sensor_t));

//=============================================================================
// af_fd_priority_caf_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  STATIC_ASSERT(sizeof(af_fd_priority_caf_adapter_t) == sizeof(af_fd_priority_caf_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.fd_prio,
    sizeof(af_fd_priority_caf_adapter_t),
    &tuning_info->af_algo.fd_prio,
    sizeof(af_fd_priority_caf_t));

//=============================================================================
// af_tuning_lens_sag_comp_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  STATIC_ASSERT(sizeof(af_tuning_lens_sag_comp_adapter_t) == sizeof(af_tuning_lens_sag_comp_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.lens_sag_comp,
    sizeof(af_tuning_lens_sag_comp_adapter_t),
    &tuning_info->af_algo.lens_sag_comp,
    sizeof(af_tuning_lens_sag_comp_t));

//=============================================================================
// af_tuning_vbt_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  STATIC_ASSERT(sizeof(af_tuning_vbt_adapter_t) == sizeof(af_tuning_vbt_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_algo.vbt,
    sizeof(af_tuning_vbt_adapter_t),
    &tuning_info->af_algo.vbt,
    sizeof(af_tuning_vbt_t));

//=============================================================================
// af_tuning_fovc_adapter: af_tuning_algo_adapter_t
//=============================================================================
  tuning_adapter_info->af_algo.fovc.enable =
    tuning_info->af_algo.fovc.enable;
  tuning_adapter_info->af_algo.fovc.fudge_factor =
      tuning_info->af_algo.fovc.fudge_factor;
  tuning_adapter_info->af_algo.fovc.anchor_lens_pos1 =
      tuning_info->af_algo.fovc.anchor_lens_pos1;
  tuning_adapter_info->af_algo.fovc.anchor_lens_pos2 =
      tuning_info->af_algo.fovc.anchor_lens_pos2;
  tuning_adapter_info->af_algo.fovc.anchor_distance_mm_1 =
      tuning_info->af_algo.fovc.anchor_distance_mm_1;
  tuning_adapter_info->af_algo.fovc.anchor_distance_mm_2 =
      tuning_info->af_algo.fovc.anchor_distance_mm_2;
  tuning_adapter_info->af_algo.fovc.focus_converge_step_size_bright_light =
      tuning_info->af_algo.fovc.focus_converge_step_size_bright_light;
  tuning_adapter_info->af_algo.fovc.fine_search_step_size_bright_light =
      tuning_info->af_algo.fovc.fine_search_step_size_bright_light;
  tuning_adapter_info->af_algo.fovc.focus_converge_step_size_low_light =
      tuning_info->af_algo.fovc.focus_converge_step_size_low_light;
  tuning_adapter_info->af_algo.fovc.fine_search_step_size_low_light =
      tuning_info->af_algo.fovc.fine_search_step_size_low_light;

//=============================================================================
// af_tuning_multiwindow_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  tuning_adapter_info->af_algo.af_multiwindow.enable =
    tuning_adapter_info->af_algo.af_multiwindow.enable;
  tuning_adapter_info->af_algo.af_multiwindow.h_clip_ratio_normal_light =
    tuning_adapter_info->af_algo.af_multiwindow.h_clip_ratio_normal_light;
  tuning_adapter_info->af_algo.af_multiwindow.v_clip_ratio_normal_light =
    tuning_adapter_info->af_algo.af_multiwindow.v_clip_ratio_normal_light;
  tuning_adapter_info->af_algo.af_multiwindow.h_clip_ratio_low_light =
    tuning_adapter_info->af_algo.af_multiwindow.h_clip_ratio_low_light;
  tuning_adapter_info->af_algo.af_multiwindow.v_clip_ratio_low_light =
    tuning_adapter_info->af_algo.af_multiwindow.v_clip_ratio_low_light;
  tuning_adapter_info->af_algo.af_multiwindow.h_num_default =
    tuning_adapter_info->af_algo.af_multiwindow.h_num_default;
  tuning_adapter_info->af_algo.af_multiwindow.v_num_default =
    tuning_adapter_info->af_algo.af_multiwindow.v_num_default;
  tuning_adapter_info->af_algo.af_multiwindow.h_num_windows =
    tuning_adapter_info->af_algo.af_multiwindow.h_num_windows;
  tuning_adapter_info->af_algo.af_multiwindow.v_num_windows =
    tuning_adapter_info->af_algo.af_multiwindow.v_num_windows;

//=============================================================================
// af_tuning_focus_mode_search_limit_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  COPY_ARRAY(tuning_adapter_info->af_algo.search_limit, AF_FOCUS_MODE_MAX,
             tuning_info->af_algo.search_limit, AF_FOCUS_MODE_MAX);

//=============================================================================
// af_tuning_algo_extended_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  int i, j;
  for (i = 0, j = 0; i < AF_FOCUS_MODE_MAX; i++) {
    j = i * 3; // three parameters inside each limit structure
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].near_end =
      tuning_info->af_algo.reserve[j];
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].far_end =
      tuning_info->af_algo.reserve[j + 1];
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].default_pos =
      tuning_info->af_algo.reserve[j + 2];
  }
  tuning_adapter_info->af_algo.algo_extended.stats_comp_type =
    (af_stats_use_type)tuning_info->af_algo.reserve[15];
  tuning_adapter_info->af_algo.algo_extended.hv_stats_ratio_enable =
    (boolean)tuning_info->af_algo.reserve[16];
  tuning_adapter_info->af_algo.algo_extended.gravity_x_land_thres =
  tuning_info->af_algo.reserve[17];
  tuning_adapter_info->af_algo.algo_extended.gravity_y_land_thres =
  tuning_info->af_algo.reserve[18];
  tuning_adapter_info->af_algo.algo_extended.gravity_x_port_thres =
  tuning_info->af_algo.reserve[19];
  tuning_adapter_info->af_algo.algo_extended.gravity_y_port_thres =
  tuning_info->af_algo.reserve[20];
  tuning_adapter_info->af_algo.algo_extended.hv_weight =
  tuning_info->af_algo.reserve[21];
  /* robust fd reserve param */
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.robust_fd_enable =
    (unsigned char)tuning_info->af_algo.reserve[64];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.noface_cnt_thr =
    (unsigned int)tuning_info->af_algo.reserve[65];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.stable_cnt_thr =
    (unsigned int)tuning_info->af_algo.reserve[66];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.fd_roi_pos_change_th =
    (float)tuning_info->af_algo.reserve[67];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.face_lost_trigger_enable =
    (unsigned char)tuning_info->af_algo.reserve[68];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.face_lost_stable_th =
    (unsigned int)tuning_info->af_algo.reserve[69];

  tuning_adapter_info->af_algo.algo_extended.en_ignore_first_invalid_frame =
    tuning_info->af_algo.reserve[22];
  tuning_adapter_info->af_algo.algo_extended.step_back.enable =
    tuning_info->af_algo.reserve[23];
  tuning_adapter_info->af_algo.algo_extended.step_back.start_inf =
    tuning_info->af_algo.reserve[24];
  tuning_adapter_info->af_algo.algo_extended.step_back.start_mac =
    tuning_info->af_algo.reserve[25];
  tuning_adapter_info->af_algo.algo_extended.step_back.reverse_inf =
    tuning_info->af_algo.reserve[26];
  tuning_adapter_info->af_algo.algo_extended.step_back.reverse_mac =
    tuning_info->af_algo.reserve[27];
  tuning_adapter_info->af_algo.algo_extended.scan_range.fratio_min =
    tuning_info->af_algo.reserve[28];
  tuning_adapter_info->af_algo.algo_extended.scan_range.fratio_max =
    tuning_info->af_algo.reserve[29];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_prev_y1 =
    tuning_info->af_algo.reserve[30];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_next_y1 =
    tuning_info->af_algo.reserve[31];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_prev_y2 =
    tuning_info->af_algo.reserve[32];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_next_y2 =
    tuning_info->af_algo.reserve[33];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_prev_y1 =
    tuning_info->af_algo.reserve[34];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_next_y1 =
    tuning_info->af_algo.reserve[35];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_prev_y2 =
    tuning_info->af_algo.reserve[36];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_next_y2 =
    tuning_info->af_algo.reserve[37];
  tuning_adapter_info->af_algo.algo_extended.flat_detection.enable =
    tuning_info->af_algo.reserve[38];
  tuning_adapter_info->af_algo.algo_extended.flat_detection.threshold_bv =
    tuning_info->af_algo.reserve[39];
  tuning_adapter_info->af_algo.algo_extended.flat_threshold.increase =
    tuning_info->af_algo.reserve[40];
  tuning_adapter_info->af_algo.algo_extended.flat_threshold.decrease =
    tuning_info->af_algo.reserve[41];
  tuning_adapter_info->af_algo.algo_extended.en_detect_peak_in_decrease =
    tuning_info->af_algo.reserve[42];
  tuning_adapter_info->af_algo.algo_extended.en_frame_skip_goto_peak =
    tuning_info->af_algo.reserve[43];
  tuning_adapter_info->af_algo.algo_extended.en_fine_search_forcely =
    tuning_info->af_algo.reserve[44];
  tuning_adapter_info->af_algo.algo_extended.hys_offset.enable =
    tuning_info->af_algo.reserve[45];
  tuning_adapter_info->af_algo.algo_extended.hys_offset.offset =
    tuning_info->af_algo.reserve[46];
  tuning_adapter_info->af_algo.algo_extended.fd_roi.enable =
    tuning_info->af_algo.reserve[47];
  tuning_adapter_info->af_algo.algo_extended.fd_roi.ratio =
    tuning_info->af_algo.reserve[48];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.enable =
    tuning_info->af_algo.reserve[49];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.ratio_x =
    tuning_info->af_algo.reserve[50];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.ratio_y =
    tuning_info->af_algo.reserve[51];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.enable =
    tuning_info->af_algo.reserve[52];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[53];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[54];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
    tuning_info->af_algo.reserve[55];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[56];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[57];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
    tuning_info->af_algo.reserve[58];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[59];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[60];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
    tuning_info->af_algo.reserve[61];
  tuning_adapter_info->af_algo.algo_extended.en_simple_prescan =
    tuning_info->af_algo.reserve[62];
  tuning_adapter_info->af_algo.algo_extended.en_optimize_peak =
    tuning_info->af_algo.reserve[63];
  tuning_adapter_info->af_algo.algo_extended.spot_light_filter.enable =
    (int)tuning_info->af_algo.reserve[70];
  /*Use reserve field index 71~80 for IIR 2nd order filter for spot light*/
  for(i = 0, j = 71; i< 10 ; i++, j++) {
    tuning_adapter_info->af_algo.algo_extended.spot_light_filter.iir_2tap_filter[i]  =
    (float) tuning_info->af_algo.reserve[j];
  }
  /*Use reserve field index 81~90 IIR 4th order filter for spot light*/
  for(i = 0, j = 81; i< 10 ; i++, j++) {
    tuning_adapter_info->af_algo.algo_extended.spot_light_filter.iir_4tap_filter[i]  =
    (float) tuning_info->af_algo.reserve[j];
  }

  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_extension =
   (int)tuning_info->af_algo.reserve[91];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_scale_x =
    (float)tuning_info->af_algo.reserve[92];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_scale_y =
    (float)tuning_info->af_algo.reserve[93];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_symmetric_x =
    (float)tuning_info->af_algo.reserve[94];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_symmetric_y =
    (float)tuning_info->af_algo.reserve[95];
  tuning_adapter_info->af_algo.algo_extended.face_trig_priority_enable =
    (boolean)tuning_info->af_algo.reserve[98];
  tuning_adapter_info->af_algo.algo_extended.exp_comp_enable =
    (float)tuning_info->af_algo.reserve[99];

#else
  int i, j;
  for (i = 0, j = 0; i < AF_FOCUS_MODE_MAX; i++) {
    j = i * 3; // three parameters inside each limit structure
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].near_end =
      tuning_info->af_algo.reserve[j];
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].far_end =
      tuning_info->af_algo.reserve[j + 1];
    tuning_adapter_info->af_algo.algo_extended.search_limit[i].default_pos =
      tuning_info->af_algo.reserve[j + 2];
  }
  tuning_adapter_info->af_algo.algo_extended.stats_comp_type =
    (af_stats_use_type)tuning_info->af_algo.reserve[15];
  tuning_adapter_info->af_algo.algo_extended.hv_stats_ratio_enable =
    (boolean)tuning_info->af_algo.reserve[16];
  tuning_adapter_info->af_algo.algo_extended.gravity_x_land_thres =
  tuning_info->af_algo.reserve[17];
  tuning_adapter_info->af_algo.algo_extended.gravity_y_land_thres =
  tuning_info->af_algo.reserve[18];
  tuning_adapter_info->af_algo.algo_extended.gravity_x_port_thres =
  tuning_info->af_algo.reserve[19];
  tuning_adapter_info->af_algo.algo_extended.gravity_y_port_thres =
  tuning_info->af_algo.reserve[20];
  tuning_adapter_info->af_algo.algo_extended.hv_weight =
  tuning_info->af_algo.reserve[21];
  /* robust fd reserve param */
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.robust_fd_enable =
    (unsigned char)tuning_info->af_algo.reserve[64];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.noface_cnt_thr =
    (unsigned int)tuning_info->af_algo.reserve[65];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.stable_cnt_thr =
    (unsigned int)tuning_info->af_algo.reserve[66];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.fd_roi_pos_change_th =
    (float)tuning_info->af_algo.reserve[67];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.face_lost_trigger_enable =
    (unsigned char)tuning_info->af_algo.reserve[68];
  tuning_adapter_info->af_algo.algo_extended.fd_adv_tuning.face_lost_stable_th =
    (unsigned int)tuning_info->af_algo.reserve[69];

  tuning_adapter_info->af_algo.algo_extended.en_ignore_first_invalid_frame =
    tuning_info->af_algo.reserve[22];
  tuning_adapter_info->af_algo.algo_extended.step_back.enable =
    tuning_info->af_algo.reserve[23];
  tuning_adapter_info->af_algo.algo_extended.step_back.start_inf =
    tuning_info->af_algo.reserve[24];
  tuning_adapter_info->af_algo.algo_extended.step_back.start_mac =
    tuning_info->af_algo.reserve[25];
  tuning_adapter_info->af_algo.algo_extended.step_back.reverse_inf =
    tuning_info->af_algo.reserve[26];
  tuning_adapter_info->af_algo.algo_extended.step_back.reverse_mac =
    tuning_info->af_algo.reserve[27];
  tuning_adapter_info->af_algo.algo_extended.scan_range.fratio_min =
    tuning_info->af_algo.reserve[28];
  tuning_adapter_info->af_algo.algo_extended.scan_range.fratio_max =
    tuning_info->af_algo.reserve[29];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_prev_y1 =
    tuning_info->af_algo.reserve[30];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_next_y1 =
    tuning_info->af_algo.reserve[31];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_prev_y2 =
    tuning_info->af_algo.reserve[32];
  tuning_adapter_info->af_algo.algo_extended.scan_range.first_next_y2 =
    tuning_info->af_algo.reserve[33];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_prev_y1 =
    tuning_info->af_algo.reserve[34];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_next_y1 =
    tuning_info->af_algo.reserve[35];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_prev_y2 =
    tuning_info->af_algo.reserve[36];
  tuning_adapter_info->af_algo.algo_extended.scan_range.second_next_y2 =
    tuning_info->af_algo.reserve[37];
  tuning_adapter_info->af_algo.algo_extended.flat_detection.enable =
    tuning_info->af_algo.reserve[38];
  tuning_adapter_info->af_algo.algo_extended.flat_detection.threshold_bv =
    tuning_info->af_algo.reserve[39];
  tuning_adapter_info->af_algo.algo_extended.flat_threshold.increase =
    tuning_info->af_algo.reserve[40];
  tuning_adapter_info->af_algo.algo_extended.flat_threshold.decrease =
    tuning_info->af_algo.reserve[41];
  tuning_adapter_info->af_algo.algo_extended.en_detect_peak_in_decrease =
    tuning_info->af_algo.reserve[42];
  tuning_adapter_info->af_algo.algo_extended.en_frame_skip_goto_peak =
    tuning_info->af_algo.reserve[43];
  tuning_adapter_info->af_algo.algo_extended.en_fine_search_forcely =
    tuning_info->af_algo.reserve[44];
  tuning_adapter_info->af_algo.algo_extended.hys_offset.enable =
    tuning_info->af_algo.reserve[45];
  tuning_adapter_info->af_algo.algo_extended.hys_offset.offset =
    tuning_info->af_algo.reserve[46];
  tuning_adapter_info->af_algo.algo_extended.fd_roi.enable =
    tuning_info->af_algo.reserve[47];
  tuning_adapter_info->af_algo.algo_extended.fd_roi.ratio =
    tuning_info->af_algo.reserve[48];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.enable =
    tuning_info->af_algo.reserve[49];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.ratio_x =
    tuning_info->af_algo.reserve[50];
  tuning_adapter_info->af_algo.algo_extended.outdoor_roi.ratio_y =
    tuning_info->af_algo.reserve[51];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.enable =
    tuning_info->af_algo.reserve[52];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[53];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[54];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
    tuning_info->af_algo.reserve[55];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[56];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[57];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
    tuning_info->af_algo.reserve[58];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_unstable_th =
    tuning_info->af_algo.reserve[59];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_stable_th =
    tuning_info->af_algo.reserve[60];
  tuning_adapter_info->af_algo.algo_extended.scene_detect_bv.outdoor_trigger_cnt_th =
    tuning_info->af_algo.reserve[61];
  tuning_adapter_info->af_algo.algo_extended.en_simple_prescan =
    tuning_info->af_algo.reserve[62];
  tuning_adapter_info->af_algo.algo_extended.en_optimize_peak =
    tuning_info->af_algo.reserve[63];

  tuning_adapter_info->af_algo.algo_extended.spot_light_filter.enable =
    (int)tuning_info->af_algo.reserve[70];
  /*Use reserve field index 71~80 for IIR 2nd order filter for spot light*/
  for(i = 0, j = 71; i< 10 ; i++, j++) {
     tuning_adapter_info->af_algo.algo_extended.spot_light_filter.iir_2tap_filter[i]  =
     (float) tuning_info->af_algo.reserve[j];
  }
  /*Use reserve field index 81~90 IIR 4th order filter for spot light*/
  for(i = 0, j = 81; i< 10 ; i++, j++) {
    tuning_adapter_info->af_algo.algo_extended.spot_light_filter.iir_4tap_filter[i]  =
   (float) tuning_info->af_algo.reserve[j];
  }

  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_extension =
   (int)tuning_info->af_algo.reserve[91];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_scale_x =
    (float)tuning_info->af_algo.reserve[92];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_scale_y =
    (float)tuning_info->af_algo.reserve[93];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_symmetric_x =
    (float)tuning_info->af_algo.reserve[94];
  tuning_adapter_info->af_algo.algo_extended.face_extension_tuning.af_roi_face_symmetric_y =
    (float)tuning_info->af_algo.reserve[95];
  tuning_adapter_info->af_algo.algo_extended.face_trig_priority_enable =
    (boolean)tuning_info->af_algo.reserve[98];
  tuning_adapter_info->af_algo.algo_extended.exp_comp_enable =
    (float)tuning_info->af_algo.reserve[99];
#endif
//=============================================================================
// af_tuning_sw_stats_adapter_t: af_algo_tune_parms_t
//=============================================================================
  STATIC_ASSERT(sizeof(af_tuning_sw_stats_adapter_t) == sizeof(af_tuning_sw_stats_t));
  Q3A_MEMCPY_S(&tuning_adapter_info->af_swaf_config,
    sizeof(af_tuning_sw_stats_adapter_t),
    &tuning_info->af_swaf_config,
    sizeof(af_tuning_sw_stats_t));

//=============================================================================
// af_tuning_vfe_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  COPY_ARRAY(tuning_adapter_info->af_vfe, MAX_AF_KERNEL_NUM,
             tuning_info->af_vfe, MAX_AF_KERNEL_NUM);

//=============================================================================
// af_tuning_baf_adapter_t: af_tuning_algo_adapter_t
//=============================================================================
  COPY_ARRAY(tuning_adapter_info->af_baf.roi_config, Q3A_AF_SCENE_TYPE_MAX,
             tuning_info->af_baf.roi_config, AF_SCENE_TYPE_MAX);
  COPY_ARRAY(tuning_adapter_info->af_baf.gamma, Q3A_AF_SCENE_TYPE_MAX,
             tuning_info->af_baf.gamma, AF_SCENE_TYPE_MAX);
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
  COPY_ARRAY(tuning_adapter_info->af_baf.filter, Q3A_AF_SCENE_TYPE_MAX,
             tuning_info->af_baf.filter, AF_SCENE_TYPE_MAX);
#else
  {
  int i;
  for (i = 0; i < Q3A_AF_SCENE_TYPE_MAX; i++) {
    COPY_ARRAY(tuning_adapter_info->af_baf.filter[i].HV_filter, MAX_BAF_FILTER,
               tuning_info->af_baf.filter[i].HV_filter, MAX_BAF_FILTER);

    COPY_ARRAY(tuning_adapter_info->af_baf.filter[i].HV_filter_2nd_tier, MAX_BAF_FILTER_2ND_TIER,
               tuning_info->af_baf.filter[i].HV_filter_2nd_tier, MAX_BAF_FILTER_2ND_TIER);

    COPY_FIELD(tuning_adapter_info->af_baf.filter[i].scaler,
               tuning_info->af_baf.filter[i].scaler);

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0310)
    COPY_FIELD(tuning_adapter_info->af_baf.filter[i].coefficient,
               tuning_info->af_baf.filter[i].coefficient);
#endif
  }
  }
#endif

  //get af algo params
  tuning_adapter_info->af_algo.algo_extended.af_algo_params = (af_algo_params*)af_get_algo_params();

  return true;
}


/*=============================================================================
 *  AEC Methods
 *=============================================================================*/

boolean parse_AEC_chromatix(void* aec_tuning_info, aec_algo_tuning_adapter* tuning_adapter)
{
  AEC_algo_struct_type* tuning_info = (AEC_algo_struct_type*)aec_tuning_info;

  if (tuning_info == NULL)
  {
    AEC_ERR("error: tuning_info is NULL");
    return false;
  }

  if (tuning_adapter == NULL)
  {
    AEC_ERR("error: tuning_adapter is NULL");
    return false;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_generic
  *=============================================================================*/
  {
  aec_generic_tuning_adapter* dest = &tuning_adapter->aec_generic;
  aec_generic_tuning_type*    src  = &tuning_info->aec_generic;

  dest->aec_start_index =
    src->aec_start_index;
  dest->luma_tolerance =
    src->luma_tolerance;
  dest->pct_based_luma_tolerance_enable =
    src->pct_based_luma_tolerance_enable;
  dest->luma_tolerance_pct =
    src->luma_tolerance_pct;
  dest->frame_skip_startup =
    src->frame_skip_startup;
  dest->aggressiveness_startup =
    src->aggressiveness_startup;
  dest->exposure_index_adj_step =
    src->exposure_index_adj_step;
  dest->ISO100_gain =
    src->ISO100_gain;
  dest->antibanding_vs_ISO_priority =
    src->antibanding_vs_ISO_priority;
  dest->iso_quantization_enable =
    src->iso_quantization_enable;
  dest->max_snapshot_exposure_time_allowed =
    src->max_snapshot_exposure_time_allowed;
  dest->lux_index_ref =
    src->lux_index_ref;
  dest->aec_subsampling_factor =
    src->aec_subsampling_factor;

  dest->idx_comp.awb_exp_idx_comp_enable =
    src->idx_comp.awb_exp_idx_comp_enable;
  dest->idx_comp.lux_idx_comp_enable =
    src->idx_comp.lux_idx_comp_enable;
  dest->idx_comp.ref_sensitivity =
    src->idx_comp.ref_sensitivity;

  dest->aec_ev_table.steps_per_ev =
    src->aec_ev_table.steps_per_ev;
  dest->aec_ev_table.min_ev_val =
    src->aec_ev_table.min_ev_val;
  dest->aec_ev_table.max_ev_val =
    src->aec_ev_table.max_ev_val;

  COPY_ARRAY(dest->aec_ev_table.ev_comp_table, MAX_EV_ARRAY_SIZE,
             src->aec_ev_table.ev_comp_table, MAX_EV_ARRAY_SIZE);
  COPY_ARRAY(dest->aec_ev_table.bracketing_table, MAX_EV_ARRAY_SIZE,
             src->aec_ev_table.bracketing_table, MAX_EV_ARRAY_SIZE);

  dest->aec_luma_weight.r_weight =
    src->aec_luma_weight.r_weight;
  dest->aec_luma_weight.g_weight =
    src->aec_luma_weight.g_weight;
  dest->aec_luma_weight.b_weight =
    src->aec_luma_weight.b_weight;
  dest->aec_luma_weight.color_and_sensor_comp_enable =
    src->aec_luma_weight.color_and_sensor_comp_enable;
  dest->aec_luma_weight.ref_rGain =
    src->aec_luma_weight.ref_rGain;
  dest->aec_luma_weight.ref_gGain =
    src->aec_luma_weight.ref_gGain;
  dest->aec_luma_weight.ref_bGain =
    src->aec_luma_weight.ref_bGain;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && ((CHROMATIX_VERSION == 0x0309) || (CHROMATIX_VERSION == 0x0310))
  dest->dual_cam_exposure_mult = src->reserved[3];
  dest->dual_cam_comp_ratio[0] = src->reserved[4];
  dest->dual_cam_comp_ratio[1] = src->reserved[5];
  dest->dual_cam_comp_ratio[2] = src->reserved[6];
  dest->dual_cam_comp_ratio[3] = src->reserved[7];
  dest->dual_cam_comp_ratio[4] = src->reserved[8];
  dest->dual_cam_comp_ratio[5] = src->reserved[9];

  /* Fast AEC tuning parameters */
  dest->fast_aec.start_index        =
    (int)(((int)src->reserved[12] > 0) ? (int)(src->reserved[12]) : (int)(dest->aec_start_index));
  dest->fast_aec.startup_frame_skip =
    (int)(((int)src->reserved[13] > 0) ? (int)(src->reserved[13]) : (FAST_AEC_STARTUP_FRAME_SKIP));
  dest->fast_aec.frame_skip         =
    (int)(((int)src->reserved[14] > 0) ? (int)(src->reserved[14]) : (FAST_AEC_FRAME_SKIP));
  dest->fast_aec.luma_settled_count =
    (int)(((int)src->reserved[15] > 0) ? (int)(src->reserved[15]) : (FAST_AEC_MIN_FRAME_SETTLE_THRESHOLD));

  /* LLS: OIS feature related  */
  dest->low_light_shot.enable              = (uint32)src->reserved[20];
  dest->low_light_shot.luxindex_thr_500lux = src->reserved[21];
  dest->low_light_shot.luxindex_thr_200lux = src->reserved[22];
  dest->low_light_shot.luxindex_thr_100lux = src->reserved[23];
  dest->low_light_shot.luxindex_thr_050lux = src->reserved[24];
  dest->low_light_shot.luxindex_thr_010lux = src->reserved[25];
  dest->low_light_shot.req_exp_time_500lux = src->reserved[26];
  dest->low_light_shot.req_exp_time_200lux = src->reserved[27];
  dest->low_light_shot.req_exp_time_100lux = src->reserved[28];
  dest->low_light_shot.req_exp_time_050lux = src->reserved[29];
  dest->low_light_shot.req_exp_time_010lux = src->reserved[30];
  dest->low_light_shot.req_extend_max_gain = src->reserved[44];

  /* Additional flat scene parameters */
  dest->flat_non_grey_th_low  = (src->reserved[31] > 0 ? src->reserved[31] : 0.5f);
  dest->flat_non_grey_th_high = (src->reserved[32] > 0 ? src->reserved[32] : 0.8f);

  /* LV test enable flag */
  dest->enable_LV_test = (int)src->reserved[33];

  /* Temporal filtering of the lux index value */
  dest->lux_index_filter_factor = ((int)(src->reserved[34] * 100) > 0) ? (src->reserved[34]) : (LUX_INDEX_TEMPORAL_FILTER_FACTOR);


  /* Cap exposure index for flash capture */
  dest->enable_cap_exp_index_for_flash_estimation = (boolean)src->reserved[35];
  dest->cap_exp_index_ratio_for_flash_estimation = src->reserved[36];

  /* Warm Start Tuning Parameters */
  dest->enable_stored_params = (boolean)(((boolean)src->reserved[37] > 0) ? TRUE : FALSE);
  dest->warmstart_exp_range  =
    (float)((src->reserved[38] > 0.0f) ? (src->reserved[38]) : (WARM_START_EXP_RANGE_PERC)) / 100.0f;

  /* Aggressive AEC tuning parameters */
  dest->aggressive.startup_frame_skip  =
    (uint32)((src->reserved[39] > 0.0f) ? (src->reserved[39]) : (AGGRESSIVE_AEC_STARTUP_FRAME_SKIP));
  dest->aggressive.frame_skip          =
    (uint32)((src->reserved[40] > 0.0f) ? (src->reserved[40]) : (AGGRESSIVE_AEC_FRAME_SKIP));
  dest->aggressive.luma_tol_percentage =
    (float)((src->reserved[41] > 0.0f) ? (src->reserved[41]) : (AGGRESSIVE_AEC_LUMA_TOLERANCE_PCT)) / 100.0f;
  dest->aggressive.max_frame_converge  =
    (uint32)((src->reserved[42] > 0.0f) ? (src->reserved[42]) : (AGGRESSIVE_AEC_MAX_FRAME_CONVERGENCE_THRESHOLD));
  dest->aggressive.min_settled_frame   =
    (uint32)((src->reserved[43] > 0.0f) ? (src->reserved[43]) : (AGGRESSIVE_AEC_MIN_FRAME_SETTLE_THRESHOLD));
  dest->aggressive.min_awb_settle_count = AGGRESSIVE_AEC_MIN_AWB_SETTLE_COUNT;

  dest->fast_aec.use_digital_gain   = (((int)src->reserved[46] > 0) ? TRUE : FALSE);

  /* AEC HAL lock for preflash */
  dest->ignore_hal_lock_before_preflash = (uint32)src->reserved[47];

  dest->fast_aec_forced_cnt = (uint16_t)src->reserved[48];
  /* AEC requested region skip pattern */
  dest->requested_rgn_skip_pattern      = (int)src->reserved[49];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_exposure_table
  *=============================================================================*/
  {
  aec_exposure_table_adapter* dest = &tuning_adapter->aec_exposure_table;
  aec_exposure_table_type*    src  = &tuning_info->aec_exposure_table;

  dest->valid_entries =
    src->valid_entries;
  dest->aec_enable_digital_gain_for_EV_lowlight =
    src->aec_enable_digital_gain_for_EV_lowlight;
  dest->total_entries_with_digital_gain_for_EV =
    src->total_entries_with_digital_gain_for_EV;
  dest->fix_fps_aec_table_index =
    src->fix_fps_aec_table_index;

  COPY_ARRAY(dest->exposure_entries, MAX_EXPOSURE_TABLE_SIZE,
             src->exposure_entries, MAX_EXPOSURE_TABLE_SIZE);
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_snapshot_lut
  *=============================================================================*/
  {
  aec_snapshot_exposure_adapter* dest = &tuning_adapter->aec_snapshot_lut;
  aec_snapshot_exposure_type*    src  = &tuning_info->aec_snapshot_lut;

  dest->enable =
    src->enable;
  dest->exposure_stretch_enable =
    src->exposure_stretch_enable;
  dest->valid_entries =
    src->valid_entries;

  COPY_ARRAY(dest->snapshot_ae_table, MAX_SNAPSHOT_LUT_SIZE,
             src->snapshot_ae_table, MAX_SNAPSHOT_LUT_SIZE);
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_luma_target
  *=============================================================================*/
  {
  aec_luma_target_adapter* dest = &tuning_adapter->aec_luma_target;
  aec_luma_target_type*    src  = &tuning_info->aec_luma_target;

  dest->num_zones =
    src->num_zones;

  COPY_ARRAY(dest->triggers, (MAX_AEC_TRIGGER_ZONE - 1),
             src->triggers, (MAX_AEC_TRIGGER_ZONE - 1));
  COPY_ARRAY(dest->triggered_params, MAX_AEC_TRIGGER_ZONE,
             src->triggered_params, MAX_AEC_TRIGGER_ZONE);
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_metering_tables
  *=============================================================================*/
  {
  aec_metering_table_adapter* dest = &tuning_adapter->aec_metering_tables;
  aec_metering_table_type*    src  = &tuning_info->aec_metering_tables;

  COPY_ARRAY(dest->AEC_weight_center_weighted, (NUM_AEC_STATS * NUM_AEC_STATS),
             src->AEC_weight_center_weighted, (NUM_AEC_STATS * NUM_AEC_STATS));
  COPY_ARRAY(dest->AEC_weight_spot_metering, (NUM_AEC_STATS * NUM_AEC_STATS),
             src->AEC_weight_spot_metering, (NUM_AEC_STATS * NUM_AEC_STATS));
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_fast_convergence
  *=============================================================================*/
  {
  aec_fast_conv_adapter* dest = &tuning_adapter->aec_fast_convergence;
  aec_fast_conv_type*    src  = &tuning_info->aec_fast_convergence;

  dest->frame_skip =
    src->frame_skip;
  dest->aggressiveness =
    src->aggressiveness;

  /* Mapping reserved to extended paremters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->low_luma_cap_ratio  = src->reserved[0];
  dest->bright_ratio_adjust = src->reserved[1];
  dest->dark_ratio_adjust   = src->reserved[2];
  dest->enable_speed_adjust = (boolean)src->reserved[3];
#else
  // TODO: verify that these same reserved parameters are used for 310
  dest->low_luma_cap_ratio  = src->reserved[0];
  dest->bright_ratio_adjust = src->reserved[1];
  dest->dark_ratio_adjust   = src->reserved[2];
  dest->enable_speed_adjust = (boolean)src->reserved[3];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_slow_convergence
  *=============================================================================*/
  {
  aec_slow_conv_adapter* dest = &tuning_adapter->aec_slow_convergence;
  aec_slow_conv_type*    src  = &tuning_info->aec_slow_convergence;

  dest->slow_convergence_enable =
    src->slow_convergence_enable;
  dest->frame_skip =
    src->frame_skip;
  dest->conv_speed =
    src->conv_speed;
  dest->ht_enable =
    src->ht_enable;
  dest->ht_tolerance_adjust_factor =
    src->ht_tolerance_adjust_factor;
  dest->ht_thres =
    src->ht_thres;
  dest->ht_max =
    src->ht_max;
  dest->ht_gyro_enable =
    src->ht_gyro_enable;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->ref_frame_rate             = src->reserved[0];
  dest->step_dark                  = src->reserved[1] * (1 - dest->conv_speed)
                                   + src->reserved[2] * dest->conv_speed;
  dest->step_bright                = src->reserved[3] * (1 - dest->conv_speed)
                                   + src->reserved[4] * dest->conv_speed;
  dest->step_regular               = src->reserved[5] * (1 - dest->conv_speed)
                                   + src->reserved[6] * dest->conv_speed;
  dest->luma_tol_ratio_dark        = src->reserved[7] * (1 - dest->conv_speed)
                                   + src->reserved[8] * dest->conv_speed;
  dest->luma_tol_ratio_bright      = src->reserved[9] * (1 - dest->conv_speed)
                                   + src->reserved[10] * dest->conv_speed;
  dest->raw_step_adjust_cap        = src->reserved[11];
  dest->adjust_skip_luma_tolerance = (int)src->reserved[12];
  dest->bright_region_num_thres    = src->reserved[13];
  dest->dark_region_num_thres      = src->reserved[14];

  /* Mapping holding time control to extended tuning parameters */
  dest->ht_luma_thres_low          = src->reserved[15];
  dest->ht_luma_thres_high         = src->reserved[16];
  dest->ht_luma_val_low            = src->reserved[17];
  dest->ht_luma_val_high           = src->reserved[18];
  dest->ht_gyro_thres_low          = src->reserved[19];
  dest->ht_gyro_thres_high         = src->reserved[20];
  dest->ht_gyro_val_low            = src->reserved[21];
  dest->ht_gyro_val_high           = src->reserved[22];
#else
  // TODO: verify that these same reserved parameters are used for 310
  dest->ref_frame_rate             = src->reserved[0];
  dest->step_dark                  = src->reserved[1] * (1 - dest->conv_speed)
                                   + src->reserved[2] * dest->conv_speed;
  dest->step_bright                = src->reserved[3] * (1 - dest->conv_speed)
                                   + src->reserved[4] * dest->conv_speed;
  dest->step_regular               = src->reserved[5] * (1 - dest->conv_speed)
                                   + src->reserved[6] * dest->conv_speed;
  dest->luma_tol_ratio_dark        = src->reserved[7] * (1 - dest->conv_speed)
                                   + src->reserved[8] * dest->conv_speed;
  dest->luma_tol_ratio_bright      = src->reserved[9] * (1 - dest->conv_speed)
                                   + src->reserved[10] * dest->conv_speed;
  dest->raw_step_adjust_cap        = src->reserved[11];
  dest->adjust_skip_luma_tolerance = (int)src->reserved[12];
  dest->bright_region_num_thres    = src->reserved[13];
  dest->dark_region_num_thres      = src->reserved[14];

  /* Mapping holding time control to extended tuning parameters */
  dest->ht_luma_thres_low          = src->reserved[15];
  dest->ht_luma_thres_high         = src->reserved[16];
  dest->ht_luma_val_low            = src->reserved[17];
  dest->ht_luma_val_high           = src->reserved[18];
  dest->ht_gyro_thres_low          = src->reserved[19];
  dest->ht_gyro_thres_high         = src->reserved[20];
  dest->ht_gyro_val_low            = src->reserved[21];
  dest->ht_gyro_val_high           = src->reserved[22];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_led_flash
  *=============================================================================*/
  {
  aec_flash_tuning_adapter* dest = &tuning_adapter->aec_led_flash;
  aec_flash_tuning_type*    src  = &tuning_info->aec_led_flash;
  aec_generic_tuning_type*  src_generic  = &tuning_info->aec_generic;

  dest->wled_trigger.start =
    src->wled_trigger.start;
  dest->wled_trigger.end =
    src->wled_trigger.end;
  dest->force_led_af_in_led_on_mode =
    src->force_led_af_in_led_on_mode;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->led_flux_used = true;
  dest->led_flux.pre_flux =
    src->aec_led_pre_flux;
  dest->led_flux.flux_hi =
    src->aec_led_flux_hi;
  dest->led_flux.flux_med =
    src->aec_led_flux_med;
  dest->led_flux.flux_low =
    src->aec_led_flux_low;
#else
  dest->led_flux_used = false;
  dest->led_table_tuning.dual_led_enable =
    src->led_table_tuning.dual_led_enable;
  dest->led_table_tuning.dual_led_intersect_slope =
    src->led_table_tuning.dual_led_intersect_slope;
  dest->led_table_tuning.led_power_size =
    src->led_table_tuning.led_power_size;
  dest->led_table_tuning.led_mix_size =
    src->led_table_tuning.led_mix_size;

  COPY_ARRAY(dest->led_table_tuning.cct_table, (MAX_LED_POWER_LEVEL * MAX_LED_MIX_LEVEL),
             src->led_table_tuning.cct_table, (MAX_LED_POWER_LEVEL * MAX_LED_MIX_LEVEL));

  dest->led_table_tuning.cct_table_calibration.calib_num =
    src->led_table_tuning.cct_table_calibration.calib_num;

  COPY_ARRAY(dest->led_table_tuning.cct_table_calibration.calib_entries, MAX_CALIB_NUM,
             src->led_table_tuning.cct_table_calibration.calib_entries, MAX_CALIB_NUM);

  dest->led_table_tuning.cct_table_calibration.corner_sample_screen_enable =
    src->led_table_tuning.cct_table_calibration.corner_sample_screen_enable;

  COPY_ARRAY(dest->led_table_tuning.cct_table_calibration.rg_ratio_ok_range, (MAX_CALIB_NUM * CALIB_RANGE_NUM),
             src->led_table_tuning.cct_table_calibration.rg_ratio_ok_range, (MAX_CALIB_NUM * CALIB_RANGE_NUM));

  COPY_ARRAY(dest->led_table_tuning.cct_table_calibration.bg_ratio_ok_range, (MAX_CALIB_NUM * CALIB_RANGE_NUM),
             src->led_table_tuning.cct_table_calibration.bg_ratio_ok_range, (MAX_CALIB_NUM * CALIB_RANGE_NUM));

  COPY_ARRAY(dest->led_table_tuning.cct_table_calibration.fixed_CCTs, MAX_CALIB_NUM,
             src->led_table_tuning.cct_table_calibration.fixed_CCTs, MAX_CALIB_NUM);

  COPY_ARRAY(dest->led_table_tuning.cct_table_calibration.fixed_exp_index, MAX_CALIB_NUM,
             src->led_table_tuning.cct_table_calibration.fixed_exp_index, MAX_CALIB_NUM);

  dest->led_table_tuning.cct_table_calibration.frame_err_allowance =
    src->led_table_tuning.cct_table_calibration.frame_err_allowance;
  dest->led_table_tuning.cct_table_calibration.frame_num =
    src->led_table_tuning.cct_table_calibration.frame_num;
  dest->led_table_tuning.cct_table_calibration.frame_num_max =
    src->led_table_tuning.cct_table_calibration.frame_num_max;
#endif

  dest->smart_flash_est_enable =
    src->smart_flash_est_enable;
  dest->smart_flash_est_strength =
    src->smart_flash_est_strength;
  dest->smart_flash_est_awb_enable =
    src->smart_flash_est_awb_enable;
  dest->smart_flash_est_awb_strength =
    src->smart_flash_est_awb_strength;

  dest->flash_target_enable =
    src->flash_target_enable;
  dest->flash_target.num_zones =
    src->flash_target.num_zones;

  COPY_ARRAY(dest->flash_target.triggers, (MAX_AEC_TRIGGER_ZONE - 1),
             src->flash_target.triggers, (MAX_AEC_TRIGGER_ZONE - 1));

  COPY_ARRAY(dest->flash_target.triggered_params, MAX_AEC_TRIGGER_ZONE,
             src->flash_target.triggered_params, MAX_AEC_TRIGGER_ZONE);

  dest->target_interp_k_start =
    src->target_interp_k_start;
  dest->target_interp_k_end =
    src->target_interp_k_end;
  dest->smart_flash_target_enable =
    src->smart_flash_target_enable;
  dest->max_target_offset =
    src->max_target_offset;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->bright_level_diff_thres = (int)(src->reserved[0]);
  dest->bright_level_thres      = (int)(src->reserved[1]);
  dest->bright_sat_diff_thres   = src->reserved[2];
  dest->bright_sat_thres        = src->reserved[3];
  dest->conv_min_luma           = (int)(src->reserved[4]);
  dest->conv_min_adjust         = (int)(src->reserved[5]);
  dest->off_stats_min           = (int)(src->reserved[6]);
  dest->sat_pct_thres           = src->reserved[7];
  dest->k_stats_pct             = src->reserved[8];
  dest->k_stats_min_diff        = src->reserved[9];
  dest->high_stat_target        = (int)(src->reserved[10]);
  dest->led_conv_smooth_rate    = src->reserved[11] == 0 ? 1 : src->reserved[11];
  /* For LED bias Table adj with Saturation prevention */
  dest->enable_led_sat_prevent = src_generic->reserved[0];
  dest->enable_speckle_handling = src_generic->reserved[1];
  dest->k_floor_pct = src_generic->reserved[2];
  dest->sat_prevent_strength    = src->reserved[12];
  /* For LED bias Table adj with circular speckle handling */
  dest->sat_pct_start           = src->reserved[13];
  dest->sat_pct_end             = src->reserved[14];
#else
  // TODO: verify that these same reserved parameters are used for 310
  dest->bright_level_diff_thres = (int)(src->reserved[0]);
  dest->bright_level_thres      = (int)(src->reserved[1]);
  dest->bright_sat_diff_thres   = src->reserved[2];
  dest->bright_sat_thres        = src->reserved[3];
  dest->conv_min_luma           = (int)(src->reserved[4]);
  dest->conv_min_adjust         = (int)(src->reserved[5]);
  dest->off_stats_min           = (int)(src->reserved[6]);
  dest->sat_pct_thres           = src->reserved[7];
  dest->k_stats_pct             = src->reserved[8];
  dest->k_stats_min_diff        = src->reserved[9];
  dest->high_stat_target        = (int)(src->reserved[10]);
  dest->led_conv_smooth_rate    = src->reserved[11] == 0 ? 1 : src->reserved[11];
  /* For LED bias Table adj with Saturation prevention */
  dest->enable_led_sat_prevent = src_generic->reserved[0];
  dest->enable_speckle_handling = src_generic->reserved[1];
  dest->k_floor_pct = src_generic->reserved[2];
  dest->sat_prevent_strength    = src->reserved[12];
  /* For LED bias Table adj with circular speckle handling */
  dest->sat_pct_start           = src->reserved[13];
  dest->sat_pct_end             = src->reserved[14];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_touch
  *=============================================================================*/
  {
  aec_touch_adapter* dest = &tuning_adapter->aec_touch;
  aec_touch_type*    src  = &tuning_info->aec_touch;

  dest->touch_roi_enable =
    src->touch_roi_enable;
  dest->touch_roi_weight =
    src->touch_roi_weight;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->temporal_filter_weight         = src->reserved[0];
  dest->speed_adjust_ratio             = src->reserved[1];
  dest->center_weight_strength         = src->reserved[2];
  dest->touch_LED_metering_enable      = (boolean)(src->reserved[3]);

  /* Touch EV feature */
  dest->touch_EV_enable     = (boolean)tuning_info->aec_generic.reserved[16];
  dest->luma_delta_thres    = (uint16)tuning_info->aec_generic.reserved[17];
  dest->luma_avg_pct_thres  = tuning_info->aec_generic.reserved[18];
  dest->luma_avg_min        = tuning_info->aec_generic.reserved[19];
#else
  // TODO: verify that these same reserved parameters are used for 310
  dest->temporal_filter_weight         = src->reserved[0];
  dest->speed_adjust_ratio             = src->reserved[1];
  dest->center_weight_strength         = src->reserved[2];
  dest->touch_LED_metering_enable      = (boolean)(src->reserved[3]);

  /* Touch EV feature */
  dest->touch_EV_enable     = (boolean)tuning_info->aec_generic.reserved[16];
  dest->luma_delta_thres    = (uint16)tuning_info->aec_generic.reserved[17];
  dest->luma_avg_pct_thres  = tuning_info->aec_generic.reserved[18];
  dest->luma_avg_min        = tuning_info->aec_generic.reserved[19];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_face_detection
  *=============================================================================*/
  {
  aec_face_priority_adapter* dest = &tuning_adapter->aec_face_detection;
  aec_face_priority_type*    src  = &tuning_info->aec_face_detection;

  dest->aec_face_enable =
    src->aec_face_enable;
  dest->aec_face_weight =
    src->aec_face_weight;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->status_update_delay_time_enter     = src->reserved[0];
  dest->status_update_delay_time_exit      = src->reserved[1];
  dest->speed_adjust_delay_time_enter      = src->reserved[2];
  dest->speed_adjust_delay_time_exit       = src->reserved[3];
  dest->speed_adjust_ratio                 = src->reserved[4];
  dest->temporal_filter_weight             = src->reserved[5];
  dest->center_weight_strength             = src->reserved[6];

  /* Backlit compensation feature */
  dest->backlit_comp_enable                = (boolean)src->reserved[7];
  dest->backlit_weight_adjust_hi           = src->reserved[8];
  dest->backlit_weight_adjust_low          = src->reserved[9];
  dest->backlit_luma_ratio_thres_hi        = src->reserved[10];
  dest->backlit_luma_ratio_thres_low       = src->reserved[11];
#else
  // TODO: verify that these same reserved parameters are used for 310
  dest->status_update_delay_time_enter     = src->reserved[0];
  dest->status_update_delay_time_exit      = src->reserved[1];
  dest->speed_adjust_delay_time_enter      = src->reserved[2];
  dest->speed_adjust_delay_time_exit       = src->reserved[3];
  dest->speed_adjust_ratio                 = src->reserved[4];
  dest->temporal_filter_weight             = src->reserved[5];
  dest->center_weight_strength             = src->reserved[6];

  /* Backlit compensation feature */
  dest->backlit_comp_enable                = (boolean)src->reserved[7];
  dest->backlit_weight_adjust_hi           = src->reserved[8];
  dest->backlit_weight_adjust_low          = src->reserved[9];
  dest->backlit_luma_ratio_thres_hi        = src->reserved[10];
  dest->backlit_luma_ratio_thres_low       = src->reserved[11];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_bright_region
  *=============================================================================*/
  {
  aec_bright_region_adapter* dest = &tuning_adapter->aec_bright_region;
  aec_bright_region_type*    src  = &tuning_info->aec_bright_region;

  dest->bright_region_enable =
    src->bright_region_enable;
  dest->num_zones =
    src->num_zones;

  COPY_ARRAY(dest->triggers, (MAX_AEC_TRIGGER_ZONE - 1),
             src->triggers, (MAX_AEC_TRIGGER_ZONE - 1));

  COPY_ARRAY(dest->triggered_params, MAX_AEC_TRIGGER_ZONE,
             src->triggered_params, MAX_AEC_TRIGGER_ZONE);

  dest->min_tweight =
    src->min_tweight;
  dest->max_bright_pct =
    src->max_bright_pct;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_dark_region
  *=============================================================================*/
  {
  aec_dark_region_adapter* dest = &tuning_adapter->aec_dark_region;
  aec_dark_region_type*    src  = &tuning_info->aec_dark_region;

  dest->dark_region_enable =
    src->dark_region_enable;
  dest->num_zones =
    src->num_zones;

  COPY_ARRAY(dest->triggers, (MAX_AEC_TRIGGER_ZONE - 1),
             src->triggers, (MAX_AEC_TRIGGER_ZONE - 1));

  COPY_ARRAY(dest->triggered_params, MAX_AEC_TRIGGER_ZONE,
             src->triggered_params, MAX_AEC_TRIGGER_ZONE);

  dest->min_tweight =
    src->min_tweight;
  dest->max_dark_pct =
    src->max_dark_pct;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_extreme_color
  *=============================================================================*/
  {
  aec_extreme_color_adapter* dest = &tuning_adapter->aec_extreme_color;
  aec_extreme_color_type*    src  = &tuning_info->aec_extreme_color;

  dest->extreme_color_enable =
    src->extreme_color_enable;
  dest->num_zones =
    src->num_zones;

  COPY_ARRAY(dest->triggers, (MAX_AEC_TRIGGER_ZONE - 1),
             src->triggers, (MAX_AEC_TRIGGER_ZONE - 1));

  COPY_ARRAY(dest->triggered_params, MAX_AEC_TRIGGER_ZONE,
             src->triggered_params, MAX_AEC_TRIGGER_ZONE);

  dest->red_th =
    src->red_th;
  dest->red_interp_pct =
    src->red_interp_pct;
  dest->green_rg_th =
    src->green_rg_th;
  dest->green_rg_interp_pct =
    src->green_rg_interp_pct;
  dest->green_bg_th =
    src->green_bg_th;
  dest->green_bg_interp_pct =
    src->green_bg_interp_pct;
  dest->green_cntr_th =
    src->green_cntr_th;
  dest->green_cntr_interp_pct =
    src->green_cntr_interp_pct;
  dest->blue_th =
    src->blue_th;
  dest->blue_interp_pct =
    src->blue_interp_pct;
  dest->luma_high_th =
    src->luma_high_th;
  dest->luma_low_th =
    src->luma_low_th;
  dest->color_stats_pct_high_th =
    src->color_stats_pct_high_th;
  dest->color_stats_pct_low_th =
    src->color_stats_pct_low_th;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  /* The period of print stats for debug. It is given in percentage */
  dest->dbg_print_period        = src->reserved[0];
#else
  // TODO: verify that these same reserved parameters are used for 310
  /* The period of print stats for debug. It is given in percentage */
  dest->dbg_print_period        = src->reserved[0];
#endif
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_flat_scene
  *=============================================================================*/
  {
  aec_flat_scene_adapter* dest = &tuning_adapter->aec_flat_scene;
  aec_flat_scene_type*    src  = &tuning_info->aec_flat_scene;

  dest->hist_flat_detector_enable =
    src->hist_flat_detector_enable;
  dest->start_level =
    src->start_level;
  dest->end_level =
    src->end_level;
  dest->range =
    src->range;
  dest->delta_th_low =
    src->delta_th_low;
  dest->delta_th_high =
    src->delta_th_high;
  dest->bright_flat_det_th =
    src->bright_flat_det_th;
  dest->dark_flat_det_th =
    src->dark_flat_det_th;
  dest->bright_flat_tail_det =
    src->bright_flat_tail_det;
  dest->dark_flat_tail_det =
    src->dark_flat_tail_det;
  dest->bright_flat_compensation_enable =
    src->bright_flat_compensation_enable;
  dest->dark_flat_compensation_enable =
    src->dark_flat_compensation_enable;
  dest->flat_white_grey_vs_nongrey_th =
    src->flat_white_grey_vs_nongrey_th;
  dest->flat_dark_grey_vs_nongrey_th =
    src->flat_dark_grey_vs_nongrey_th;
  dest->near_grey_tolerance =
    src->near_grey_tolerance;
  dest->bright_flat_compensation_ratio =
    src->bright_flat_compensation_ratio;
  dest->dark_flat_compensation_ratio =
    src->dark_flat_compensation_ratio;
  dest->flat_indoor_disable_start_idx =
    src->flat_indoor_disable_start_idx;
  dest->flat_indoor_disable_end_idx =
    src->flat_indoor_disable_end_idx;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_hist_metering
  *=============================================================================*/
  {
  aec_hist_metering_adapter* dest = &tuning_adapter->aec_hist_metering;
  aec_hist_metering_type*    src  = &tuning_info->aec_hist_metering;
  aec_hdr_tuning_type*    src_aec_hdr  = &tuning_info->aec_hdr;

  dest->hist_metering_enable =
    src->hist_metering_enable;
  dest->adrc_enable =
    src->adrc_enable;
  dest->dark_end_control_enable =
    src->dark_end_control_enable;
  dest->num_zones =
    src->num_zones;

  COPY_ARRAY(dest->triggers, (MAX_AEC_TRIGGER_ZONE - 1),
             src->triggers, (MAX_AEC_TRIGGER_ZONE - 1));

  int i = 0, j = 0;
  /* Shallow copy the triggered_params field. */
  for (i = 0; i < MAX_AEC_TRIGGER_ZONE; i++) {
    dest->triggered_params[i].max_target_adjust_ratio =
      src->triggered_params[i].max_target_adjust_ratio;
    dest->triggered_params[i].min_target_adjust_ratio =
      src->triggered_params[i].min_target_adjust_ratio;
    dest->triggered_params[i].sat_low_ref =
      src->triggered_params[i].sat_low_ref;
    dest->triggered_params[i].sat_high_ref =
      src->triggered_params[i].sat_high_ref;
    dest->triggered_params[i].dark_low_ref =
      src->triggered_params[i].dark_low_ref;
    dest->triggered_params[i].dark_high_ref =
      src->triggered_params[i].dark_high_ref;
    dest->triggered_params[i].max_drc_gain =
      src->triggered_params[i].max_drc_gain;
    dest->triggered_params[i].adrc_conv_speed =
      src->triggered_params[i].adrc_conv_speed;
    dest->triggered_params[i].gtm_percentage =
      src->triggered_params[i].gtm_percentage;
    dest->triggered_params[i].ltm_percentage =
      src->triggered_params[i].ltm_percentage;
    dest->triggered_params[i].la_percentage =
      src->triggered_params[i].la_percentage;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
    dest->triggered_params[i].gamma_percentage = 0;
#else
    dest->triggered_params[i].gamma_percentage =
      src->triggered_params[i].gamma_percentage;
#endif

  COPY_ARRAY(dest->triggered_params[i].hist_sat_pushup_range, Q3A_AEC_BAYER_CHNL_MAX,
             src->triggered_params[i].hist_sat_pushup_range, BAYER_CHNL_MAX);

  COPY_ARRAY(dest->triggered_params[i].hist_sat_pushdown_range, Q3A_AEC_BAYER_CHNL_MAX,
             src->triggered_params[i].hist_sat_pushdown_range, BAYER_CHNL_MAX);

  COPY_ARRAY(dest->triggered_params[i].hist_dark_range, Q3A_AEC_BAYER_CHNL_MAX,
             src->triggered_params[i].hist_dark_range, BAYER_CHNL_MAX);
  }

  dest->target_filter_factor =
    src->target_filter_factor;

  /* Mapping reserved to extended tuning parameters */
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x0309)
  dest->gtm_enable          = (boolean)src->reserved[0];
  dest->ltm_enable          = (boolean)src->reserved[1];
  dest->la_enable           = (boolean)src->reserved[2];
  dest->gamma_enable        = (boolean)src->reserved[3];
  dest->dark_pushup_enable  = (boolean)src->reserved[4];
  dest->triggered_params[0].gamma_percentage = src->reserved[5];
  dest->triggered_params[1].gamma_percentage = src->reserved[6];
  dest->triggered_params[2].gamma_percentage = src->reserved[7];
  dest->triggered_params[3].gamma_percentage = src->reserved[8];
  dest->triggered_params[4].gamma_percentage = src->reserved[9];
  dest->triggered_params[5].gamma_percentage = src->reserved[10];

  /* Clamp ADRC with antibanding enabled */
  dest->adrc_atb_ctrl.adrc_antibanding_priority_enable = (boolean)src->reserved[11];
  dest->adrc_atb_ctrl.atb_cutoff_idx_50hz              = (int)src->reserved[12];
  dest->adrc_atb_ctrl.atb_cutoff_idx_60hz              = (int)src->reserved[13];
  dest->adrc_atb_ctrl.atb_trigger.start                = (int)src->reserved[14];
  dest->adrc_atb_ctrl.atb_trigger.end                  = (int)src->reserved[15];

  dest->clamp_sat_pos_enable = (int)src->reserved[16];
#else
  // TODO: verify that these same reserved parameters are used for 310
  dest->gtm_enable          = (boolean)src->reserved[0];
  dest->ltm_enable          = (boolean)src->reserved[1];
  dest->la_enable           = (boolean)src->reserved[2];
  dest->gamma_enable        = (boolean)src->reserved[3];
  dest->dark_pushup_enable  = (boolean)src->reserved[4];

  /* Clamp ADRC with antibanding enabled */
  dest->adrc_atb_ctrl.adrc_antibanding_priority_enable = (boolean)src->reserved[11];
  dest->adrc_atb_ctrl.atb_cutoff_idx_50hz              = (int)src->reserved[12];
  dest->adrc_atb_ctrl.atb_cutoff_idx_60hz              = (int)src->reserved[13];
  dest->adrc_atb_ctrl.atb_trigger.start                = (int)src->reserved[14];
  dest->adrc_atb_ctrl.atb_trigger.end                  = (int)src->reserved[15];

  dest->clamp_sat_pos_enable = (int)src->reserved[16];
#endif

  /* Histogram AEC for touch ROI metering
     * In 309/310, borrow parameters from sensor HDR reserved field */

    dest->touch_roi_hist_adjust_enable =
      (boolean)src_aec_hdr->reserved[0];
    dest->face_roi_hist_adjust_enable =
      (boolean)src_aec_hdr->reserved[1];

    dest->dark_roi_max_target_adjust_ratio[0]   =
      dest->dark_roi_max_target_adjust_ratio[1] =
      dest->dark_roi_max_target_adjust_ratio[2] =
      dest->dark_roi_max_target_adjust_ratio[3] =
      dest->dark_roi_max_target_adjust_ratio[4] =
      dest->dark_roi_max_target_adjust_ratio[5] =
      src_aec_hdr->reserved[2];
    dest->dark_roi_min_target_adjust_ratio[0]   =
      dest->dark_roi_min_target_adjust_ratio[1] =
      dest->dark_roi_min_target_adjust_ratio[2] =
      dest->dark_roi_min_target_adjust_ratio[3] =
      dest->dark_roi_min_target_adjust_ratio[4] =
      dest->dark_roi_min_target_adjust_ratio[5] =
      src_aec_hdr->reserved[3];
    dest->dark_roi_dark_low_ref[0]   =
      dest->dark_roi_dark_low_ref[1] =
      dest->dark_roi_dark_low_ref[2] =
      dest->dark_roi_dark_low_ref[3] =
      dest->dark_roi_dark_low_ref[4] =
      dest->dark_roi_dark_low_ref[5] =
      src_aec_hdr->reserved[4];
    dest->dark_roi_sat_high_ref[0]   =
      dest->dark_roi_sat_high_ref[1] =
      dest->dark_roi_sat_high_ref[2] =
      dest->dark_roi_sat_high_ref[3] =
      dest->dark_roi_sat_high_ref[4] =
      dest->dark_roi_sat_high_ref[5] =
      src_aec_hdr->reserved[5];
    dest->bright_roi_max_target_adjust_ratio[0]   =
      dest->bright_roi_max_target_adjust_ratio[1] =
      dest->bright_roi_max_target_adjust_ratio[2] =
      dest->bright_roi_max_target_adjust_ratio[3] =
      dest->bright_roi_max_target_adjust_ratio[4] =
      dest->bright_roi_max_target_adjust_ratio[5] =
      src_aec_hdr->reserved[6];
    dest->bright_roi_min_target_adjust_ratio[0]   =
      dest->bright_roi_min_target_adjust_ratio[1] =
      dest->bright_roi_min_target_adjust_ratio[2] =
      dest->bright_roi_min_target_adjust_ratio[3] =
      dest->bright_roi_min_target_adjust_ratio[4] =
      dest->bright_roi_min_target_adjust_ratio[5] =
      src_aec_hdr->reserved[7];
    dest->bright_roi_dark_low_ref[0]   =
      dest->bright_roi_dark_low_ref[1] =
      dest->bright_roi_dark_low_ref[2] =
      dest->bright_roi_dark_low_ref[3] =
      dest->bright_roi_dark_low_ref[4] =
      dest->bright_roi_dark_low_ref[5] =
      src_aec_hdr->reserved[8];
    dest->bright_roi_sat_high_ref[0]   =
      dest->bright_roi_sat_high_ref[1] =
      dest->bright_roi_sat_high_ref[2] =
      dest->bright_roi_sat_high_ref[3] =
      dest->bright_roi_sat_high_ref[4] =
      dest->bright_roi_sat_high_ref[5] =
      src_aec_hdr->reserved[9];
    dest->dark_roi_luma_ratio_start[0]   =
      dest->dark_roi_luma_ratio_start[1] =
      dest->dark_roi_luma_ratio_start[2] =
      dest->dark_roi_luma_ratio_start[3] =
      dest->dark_roi_luma_ratio_start[4] =
      dest->dark_roi_luma_ratio_start[5] =
      src_aec_hdr->reserved[10];
    dest->dark_roi_luma_ratio_end[0]   =
      dest->dark_roi_luma_ratio_end[1] =
      dest->dark_roi_luma_ratio_end[2] =
      dest->dark_roi_luma_ratio_end[3] =
      dest->dark_roi_luma_ratio_end[4] =
      dest->dark_roi_luma_ratio_end[5] =
      src_aec_hdr->reserved[11];
    dest->bright_roi_luma_ratio_start[0]   =
      dest->bright_roi_luma_ratio_start[1] =
      dest->bright_roi_luma_ratio_start[2] =
      dest->bright_roi_luma_ratio_start[3] =
      dest->bright_roi_luma_ratio_start[4] =
      dest->bright_roi_luma_ratio_start[5] =
      src_aec_hdr->reserved[12];
    dest->bright_roi_luma_ratio_end[0]   =
      dest->bright_roi_luma_ratio_end[1] =
      dest->bright_roi_luma_ratio_end[2] =
      dest->bright_roi_luma_ratio_end[3] =
      dest->bright_roi_luma_ratio_end[4] =
      dest->bright_roi_luma_ratio_end[5] =
      src_aec_hdr->reserved[13];


  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_Y_hist_stretch
  *=============================================================================*/
  {
  aec_Y_hist_stretch_adapter* dest = &tuning_adapter->aec_Y_hist_stretch;
  aec_Y_hist_stretch_type*    src  = &tuning_info->aec_Y_hist_stretch;

  dest->enable =
    src->enable;
  dest->num_zones =
    src->num_zones;

  COPY_ARRAY(dest->triggers, (MAX_AEC_TRIGGER_ZONE - 1),
             src->triggers, (MAX_AEC_TRIGGER_ZONE - 1));

  COPY_ARRAY(dest->triggered_params, MAX_AEC_TRIGGER_ZONE,
             src->triggered_params, MAX_AEC_TRIGGER_ZONE);
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_hdr
  *=============================================================================*/
  {
  aec_hdr_tuning_adapter* dest = &tuning_adapter->aec_hdr;
  aec_hdr_tuning_type*    src  = &tuning_info->aec_hdr;

  dest->hdr_indoor_trigger.start =
    src->hdr_indoor_trigger.start;
  dest->hdr_indoor_trigger.end =
    src->hdr_indoor_trigger.end;
  /* Max HDR gain is tunable parameter.In case customer does not set any value this parameter
   * will be loaded with MAX_IDEAL_HDR_GAIN(i.e '8')
   */
  dest->maxIdealHDRgain =
    (float)((src->reserved[14] >= 1.0f) ? (src->reserved[14]) : (MAX_IDEAL_HDR_GAIN));
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_preview_motion_iso
  *=============================================================================*/
  {
  aec_motion_iso_adapter* dest = &tuning_adapter->aec_preview_motion_iso;
  aec_motion_iso_type*    src  = &tuning_info->aec_preview_motion_iso;

  dest->motion_iso_enable =
    src->motion_iso_enable;
  dest->motion_iso_aggressiveness =
    src->motion_iso_aggressiveness;
  dest->motion_iso_threshold =
    src->motion_iso_threshold;
  dest->motion_iso_max_gain =
    src->motion_iso_max_gain;
  }

 /*=============================================================================
  * aec_algo_tuning_adapter::aec_snapshot_motion_iso
  *=============================================================================*/
  {
  aec_motion_iso_adapter* dest = &tuning_adapter->aec_snapshot_motion_iso;
  aec_motion_iso_type*    src  = &tuning_info->aec_snapshot_motion_iso;

  dest->motion_iso_enable =
    src->motion_iso_enable;
  dest->motion_iso_aggressiveness =
    src->motion_iso_aggressiveness;
  dest->motion_iso_threshold =
    src->motion_iso_threshold;
  dest->motion_iso_max_gain =
    src->motion_iso_max_gain;
  }
 /*=============================================================================
  * aec_algo_tuning_adapter::aec_fast_aec
  *=============================================================================*/
  {
  aec_fast_aec_adapter* dest = &tuning_adapter->aec_fast_aec;
  aec_fast_aec_type*    src  = &tuning_info->aec_fast_aec;

  dest->start_index =
    src->start_index;
  dest->frame_skip =
    src->frame_skip;
  dest->luma_settled_count =
    src->luma_settled_count;
  }

  return true;
}

boolean parse_chromatix(void* chromatix_ptr, chromatix_3a_parms_wrapper* chromatix_wrapper)
{
  chromatix_3a_parms_type* chromatix = (chromatix_3a_parms_type*)chromatix_ptr;
  boolean result = false;

  if (chromatix_ptr == NULL)
  {
    Q3A_ERR("error: chromatix_ptr is NULL");
    return result;
  }

  if (chromatix_wrapper == NULL)
  {
    Q3A_ERR("error: chromatix_wrapper is NULL");
    return result;
  }

  result = true;
  result &= parse_AF_chromatix((void*)&chromatix->AF_algo_data, &chromatix_wrapper->AF);
  result &= parse_AEC_chromatix((void*)&chromatix->AEC_algo_data, &chromatix_wrapper->AEC);

  //TODO: temporary copies - remove when the wrapper for the rest of 3A is implemented
  chromatix_wrapper->chromatix_version_info = chromatix->chromatix_version_info;
  chromatix_wrapper->aaa_version            = chromatix->aaa_version;
  chromatix_wrapper->AWB                    = chromatix->AWB_bayer_algo_data;
  chromatix_wrapper->AFD                    = chromatix->auto_flicker_detection_data;
  chromatix_wrapper->ASD                    = chromatix->ASD_3A_algo_data;
  chromatix_wrapper->aaa_sync_data          = chromatix->aaa_sync_data;
  chromatix_wrapper->EIS                    = chromatix->chromatix_EIS_data;

  return result;
}
