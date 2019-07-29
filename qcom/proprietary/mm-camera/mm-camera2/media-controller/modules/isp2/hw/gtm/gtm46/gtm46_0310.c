/*
* Copyright (c) 2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#include "isp_sub_module_log.h"
#include "isp_sub_module_util.h"
#include "gtm46.h"
#include "gtm_curve.h"

static uint32_t gtm_xin_tbl[] = {
  0,    2,    4,    7,    13,    21,    31,    45,
  62,   83,   107,  137,  169,   201,   233,   265,
  297,  329,  361,  428,  463,   499,   538,   578,
  621,  665,  712,  760,  811,   863,   919,   976,
  1037, 1098, 1164, 1230, 1300,  1371,  1447,  1524,
  1605, 1687, 1774, 1861, 1954,  2047,  2146,  2245,
  2456, 2679, 2915, 3429, 3999,  4630,  5323,  6083,
  6911, 7812, 8787, 9840, 10975, 12193, 13499, 14894,
  16383};

/** gtm46_configure_from_chromatix_bit:
 *
 *  @isp_sub_module: isp sub module handle
 *
 *  Enable / disable module based on chromatix bit
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean gtm46_configure_from_chromatix_bit(
  isp_sub_module_t *isp_sub_module)
{
  chromatix_parms_type      *chromatix_ptr = NULL;
  chromatix_VFE_common_type *chromatix_common = NULL;

  chromatix_common =
    (chromatix_VFE_common_type *)isp_sub_module->chromatix_ptrs.chromatixComPtr;
  chromatix_ptr =
    (chromatix_parms_type *)isp_sub_module->chromatix_ptrs.chromatixPtr;

  isp_sub_module->submod_enable = FALSE;
  isp_sub_module->chromatix_module_enable = FALSE;

  if (chromatix_ptr->chromatix_VFE.chromatix_gtm.gtm_enable > 0 ||
      chromatix_ptr->chromatix_VFE.chromatix_gtm_v2.gtm_v2_enable > 0) {
      isp_sub_module->submod_enable = TRUE;
      isp_sub_module->chromatix_module_enable = TRUE;
  }

  return TRUE;
}
/** gtm46_interpolate_v2:
 *
 *
 **/
static void gtm46_interpolate(gtm46_algo_params_t *in1,
  gtm46_algo_params_t *in2, gtm46_algo_params_t *out, float ratio)
{
  out->reserve_params.GTM_maxval_th =
    LINEAR_INTERPOLATION_INT(in1->reserve_params.GTM_maxval_th,
                             in2->reserve_params.GTM_maxval_th, ratio);
  out->reserve_params.GTM_key_min_th =
    LINEAR_INTERPOLATION_INT(in1->reserve_params.GTM_key_min_th,
                             in2->reserve_params.GTM_key_min_th, ratio);
  out->reserve_params.GTM_key_max_th =
    LINEAR_INTERPOLATION_INT(in1->reserve_params.GTM_key_max_th,
                             in2->reserve_params.GTM_key_max_th, ratio);
  out->reserve_params.GTM_key_hist_bin_weight =
    LINEAR_INTERPOLATION(in1->reserve_params.GTM_key_hist_bin_weight,
                         in2->reserve_params.GTM_key_hist_bin_weight, ratio);
  out->reserve_params.GTM_Yout_maxval =
    LINEAR_INTERPOLATION_INT(in1->reserve_params.GTM_Yout_maxval,
                             in2->reserve_params.GTM_Yout_maxval, ratio);
  out->core_params.GTM_a_middletone =
    LINEAR_INTERPOLATION(in1->core_params.GTM_a_middletone,
                         in2->core_params.GTM_a_middletone, ratio);
  out->core_params.GTM_temporal_w =
    LINEAR_INTERPOLATION(in1->core_params.GTM_temporal_w,
                         in2->core_params.GTM_temporal_w, ratio);
  out->core_params.GTM_middletone_w =
    LINEAR_INTERPOLATION(in1->core_params.GTM_middletone_w,
                         in2->core_params.GTM_middletone_w, ratio);
  out->core_params.GTM_max_percentile=
    LINEAR_INTERPOLATION(in1->core_params.GTM_max_percentile,
                         in2->core_params.GTM_max_percentile, ratio);
  out->core_params.GTM_min_percentile =
    LINEAR_INTERPOLATION(in1->core_params.GTM_min_percentile,
                         in2->core_params.GTM_min_percentile, ratio);
  out->reserve_params.GTM_minval_th =
    LINEAR_INTERPOLATION(in1->reserve_params.GTM_minval_th,
                         in2->reserve_params.GTM_minval_th, ratio);
}

/** gtm46_v2_process_regions:
 *
 *  @gtm_v1: pointer to the regions used in the v1 version.
 *           NULL value indicates use of v2 version
 *  @gtm_exp_v2: regions used in v2 version. If this is NULL
 *           use the v1 version.
 *  @local_algo_params: the output algo params that are
 *           computed/interpolated
 *  Use the control type (gain, lux index, exp ratio) to compute
 *  or interpolate the algo parameters.
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean gtm46_v2_process_regions(gtm46_t *gtm,
  chromatix_GTM_type *gtm_v1, chromatix_GTM_v2_type *gtm_v2,
  chromatix_GTM_v2_exp_type *gtm_exp_v2,
  gtm46_algo_params_t *local_algo_params) {

  boolean                  ret               = TRUE;
  trigger_point_type      *gtm_trigger       = NULL;
  trigger_point2_type     *gtm_trigger2      = NULL;
  float                    start             = 0.0;
  float                    end               = 0.0;
  float                    ratio             = 0.0;
  float                    aec_reference     = 0.0;
  uint32_t                 i                 = 0;
  uint8_t                  trigger_index     = MAX_SETS_FOR_TONE_NOISE_ADJ + 1;
  uint8_t                  control_type      = CONTROL_GAIN;
  boolean                  use_gtm_v2        = TRUE;
  gtm46_algo_params_t      algo_params_1 , algo_params_2;

  memset(&algo_params_1, 0, sizeof(algo_params_1));
  memset(&algo_params_2, 0, sizeof(algo_params_2));

  RETURN_IF_NULL(local_algo_params);

  if (gtm_exp_v2) {
    use_gtm_v2 = TRUE;
    /* For v2 version the input corresponds to 6 regions that
       need to triggered based on exp_ratio */
    control_type = gtm_v2->control_gtm_v2;
  } else {
    use_gtm_v2 = FALSE;
    control_type = gtm_v1->control_gtm;
  }

  for (i = 0; i < MAX_SETS_FOR_TONE_NOISE_ADJ; i++) {

    if (i == MAX_SETS_FOR_TONE_NOISE_ADJ - 1) {
      /* falls within region 6 but we do not use trigger points in the region */
      ratio         = 0.0;
      trigger_index = MAX_SETS_FOR_TONE_NOISE_ADJ - 1;
      break;
    }

    switch (control_type) {
    case CONTROL_LUX_IDX:
      /* lux index based */
      aec_reference = gtm->aec_update.lux_idx;
      if (use_gtm_v2) {
        if (i >= MAX_SETS_FOR_GTM_EXP_ADJ - 1) {
            ratio         = 0.0;
            trigger_index = MAX_SETS_FOR_GTM_EXP_ADJ - 1;
            break;
        }
        gtm_trigger   = &(gtm_v2->gtm_v2[i].gtm_v2_trigger);
      }else{
        gtm_trigger   = &(gtm_v1->gtm[i].gtm_trigger);
      }
      start         = gtm_trigger->lux_index_start;
      end           = gtm_trigger->lux_index_end;
      ISP_DBG("lux base, lux idx %f",aec_reference);
      break;
    case CONTROL_GAIN:
      /* Gain based */
      aec_reference = gtm->aec_update.real_gain;
      if (use_gtm_v2) {
        if (i >= MAX_SETS_FOR_GTM_EXP_ADJ - 1) {
            ratio         = 0.0;
            trigger_index = MAX_SETS_FOR_GTM_EXP_ADJ - 1;
            break;
        }
        gtm_trigger   = &(gtm_v2->gtm_v2[i].gtm_v2_trigger);
      }else{
        gtm_trigger   = &(gtm_v1->gtm[i].gtm_trigger);
      }

      start         = gtm_trigger->gain_start;
      end           = gtm_trigger->gain_end;
      break;
    case CONTROL_AEC_EXP_SENSITIVITY_RATIO:
      /* AEC sensitivity ratio based*/
      aec_reference = gtm->aec_update.hdr_sensitivity_ratio;
      if (use_gtm_v2) {
        if (i >= MAX_SETS_FOR_GTM_EXP_ADJ - 1) {
            ratio         = 0.0;
            trigger_index = MAX_SETS_FOR_GTM_EXP_ADJ - 1;
            break;
        }
        gtm_trigger2 = &(gtm_exp_v2[i].aec_sensitivity_ratio_v2);
      } else {
        gtm_trigger2 = &(gtm_v1->gtm[i].aec_sensitivity_ratio);
      }
      start         = gtm_trigger2->start;
      end           = gtm_trigger2->end;
      ISP_DBG("exposure base, exp_ratio %f",aec_reference);
      break;
    default:
      ret = FALSE;
      goto error;
    }

    /* index is within interpolation range, find ratio */
    if (aec_reference >= start && aec_reference < end) {
      ratio = (end - aec_reference)/(end - start);
      trigger_index = i;
      ISP_HIGH("%s [%f - %f - %f] = %f", __func__, start, aec_reference, end,
        ratio);
      break;
    }
  }

  if (trigger_index + 1 >= MAX_SETS_FOR_TONE_NOISE_ADJ) {
    ISP_ERR("invalid trigger_index, no interpolation");
    ret = FALSE;
    goto error;
  }

  if (use_gtm_v2) {
    algo_params_1.core_params = gtm_exp_v2[trigger_index].gtm_core_v2;
    algo_params_1.reserve_params = gtm_exp_v2[trigger_index].gtm_reserve_v2;
    algo_params_2.core_params = gtm_exp_v2[trigger_index + 1].gtm_core_v2;
    algo_params_2.reserve_params = gtm_exp_v2[trigger_index + 1].gtm_reserve_v2;
    gtm->mod_enable = TRUE;
  } else {
    algo_params_1.core_params = gtm_v1->gtm[trigger_index].gtm_core;
    algo_params_1.reserve_params = gtm_v1->gtm[trigger_index].gtm_reserve;
    algo_params_2.core_params = gtm_v1->gtm[trigger_index + 1].gtm_core;;
    algo_params_2.reserve_params = gtm_v1->gtm[trigger_index + 1].gtm_reserve;
    /* Region enable/disable is only available for v1 version. If the region
       is disabled skip all the update operations */
    gtm->mod_enable = gtm_v1->gtm[trigger_index].rgn_enable;
  }

  /* Update the params only if the region is enabled */
  if (gtm->mod_enable) {
    gtm46_interpolate(&algo_params_1,
                      &algo_params_2,
                      local_algo_params,
                      ratio);
  }

error:
  return ret;
}

/** gtm46_compute_algo_params:
 *
 *  Call the function to compute the algo parameters.
 *  The v1 version has 6 regions that can be used to
 *  get/interpolate the parameters.
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean gtm46_compute_algo_params(gtm46_t *gtm,
  isp_gtm_algo_params_t *algo_param, chromatix_GTM_type *chromatix_gtm) {

  boolean              ret               = TRUE;
  gtm46_algo_params_t  local_algo_params;

  RETURN_IF_NULL(algo_param);

  if (!gtm->is_aec_update_valid) {
    /* default to NORMAL_LIGHT when aec_update isn't available yet */
    algo_param->params = chromatix_gtm->gtm[2];
  } else {
    ret = gtm46_v2_process_regions(gtm, chromatix_gtm, NULL, NULL,
        &local_algo_params);
  }

  algo_param->params.gtm_core = local_algo_params.core_params;
  algo_param->params.gtm_reserve = local_algo_params.reserve_params;

  return ret;
}

/** gtm46_compute_algo_params:
 *
 *  Call the function to compute the algo parameters.
 *  The v2 version has 3x6 regions that can be used to
 *  get/interpolate the parameters.
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean gtm46_v2_compute_algo_params(gtm46_t *gtm,
  isp_gtm_algo_params_t *algo_param, chromatix_GTM_v2_type *chromatix_gtm_v2) {
  boolean              ret = TRUE;
  trigger_ratio_t      trigger_ratio;
  gtm46_algo_params_t  algo_params_1, algo_params_2;
  gtm46_algo_params_t  local_algo_params;

  RETURN_IF_NULL(algo_param);
  memset(&algo_params_1, 0, sizeof(gtm46_algo_params_t));
  memset(&algo_params_2, 0, sizeof(gtm46_algo_params_t));


  /* interpolate to get the chromatix parameters for algo */
  ret = isp_sub_module_util_get_aec_ratio_bright_low(
    chromatix_gtm_v2->control_gtm_v2,
    (void *)&chromatix_gtm_v2->gtm_v2[GTM_BRIGHT_LIGHT].gtm_v2_trigger,
    (void *)&chromatix_gtm_v2->gtm_v2[GTM_LOW_LIGHT].gtm_v2_trigger,
    &gtm->aec_update, &trigger_ratio, 0);
  if (ret != TRUE) {
    ISP_ERR("get aec ratio error");
    return TRUE;
  }
  /* The GTM V2 version has 3x6 regions i.e. has
     [MAX_SETS_FOR_GTM_EXP_ADJ][MAX_SETS_FOR_TONE_NOISE_ADJ]
     The inner/2nd dimension is triggered based on exposure ratio/aec sensitivity ratio
     (aec_sensitivity_ratio_v2) and has 6 regions.
     The outer is based on gain or lux index (gtm_v2_trigger) and has 3 regions.
  */
  switch (trigger_ratio.lighting) {
  case TRIGGER_NORMAL:
    ret = gtm46_v2_process_regions(gtm, NULL, chromatix_gtm_v2,
    chromatix_gtm_v2->gtm_v2[GTM_NORMAL_LIGHT].gtm_exp_data,
    &local_algo_params);
    break;
  case TRIGGER_LOWLIGHT:
    /* Compute the params based on exp_ratio for each lighting*/
    ret = gtm46_v2_process_regions(gtm, NULL, chromatix_gtm_v2,
    chromatix_gtm_v2->gtm_v2[GTM_NORMAL_LIGHT].gtm_exp_data,
    &algo_params_1);
    ret |= gtm46_v2_process_regions(gtm, NULL, chromatix_gtm_v2,
    chromatix_gtm_v2->gtm_v2[GTM_LOW_LIGHT].gtm_exp_data,
    &algo_params_2);
    /* Interpolate the algo params obtained above based on
       gain/lux index */
    gtm46_interpolate(&algo_params_1,
                      &algo_params_2,
                      &local_algo_params,
                      trigger_ratio.ratio);
    break;
  case TRIGGER_OUTDOOR:
    /* Compute the params based on exp_ratio for each lighting*/
    ret = gtm46_v2_process_regions(gtm, NULL, chromatix_gtm_v2,
    chromatix_gtm_v2->gtm_v2[GTM_NORMAL_LIGHT].gtm_exp_data,
    &algo_params_1);
    ret |= gtm46_v2_process_regions(gtm, NULL, chromatix_gtm_v2,
    chromatix_gtm_v2->gtm_v2[GTM_BRIGHT_LIGHT].gtm_exp_data,
    &algo_params_2);
    /* Interpolate the algo params obtained above based on
       gain/lux index */
    gtm46_interpolate(&algo_params_1,
                       &algo_params_2,
                       &local_algo_params,
                       trigger_ratio.ratio);
    break;
  }

  algo_param->params.gtm_core = local_algo_params.core_params;
  algo_param->params.gtm_reserve = local_algo_params.reserve_params;
  return ret;
}

boolean gtm46_fill_dmi_table(
  mct_module_t *module,
  gtm46_t               *gtm,
  isp_gtm_algo_params_t *algo_param,
  isp_sub_module_t      *isp_sub_module,
  chromatix_parms_type  *chromatix_ptr,
  mct_event_t *event
  )
{
    trigger_point_type      *gtm_trigger       = NULL;
    trigger_point2_type     *gtm_trigger2      = NULL;
    float                    start             = 0.0;
    float                    end               = 0.0;
    float                    ratio             = 0.0;
    uint32_t                 i                 = 0;
    float                    aec_reference     = 1.0;
    uint8_t                  trigger_index     = MAX_SETS_FOR_TONE_NOISE_ADJ + 1;
    boolean                  default_curve_valid = TRUE;
    boolean                  ret = TRUE;
    chromatix_GTM_type      *chromatix_gtm     = NULL;
    chromatix_GTM_v2_type   *chromatix_gtm_v2 =  NULL;

    chromatix_gtm = &chromatix_ptr->chromatix_VFE.chromatix_gtm;
    chromatix_gtm_v2 = &chromatix_ptr->chromatix_VFE.chromatix_gtm_v2;
    gtm->mod_enable = isp_sub_module->submod_enable;

    if (gtm->enable_adrc) {
      gtm46_derive_and_pack_lut_adrc_based(gtm);
    } else if (gtm->use_gtm_fix_curve == 0 ){
        algo_param->is_valid = 1;
        if (gtm->algo_update_valid){
          algo_param->is_prev_key_valid = gtm->algo_output.is_key_valid;
          algo_param->prev_key          = gtm->algo_output.key;
          algo_param->prev_max_v_hist   = gtm->algo_output.max_v_hist;
          algo_param->prev_min_v_hist   = gtm->algo_output.min_v_hist;
        } else {
          algo_param->is_prev_key_valid = 1;
          algo_param->prev_key = 170.06;
          algo_param->prev_max_v_hist = 2863;
          algo_param->prev_min_v_hist = 0;
        }
        /*enable temporal filter once confirm with system team*/
        algo_param->temporal_filter_enable   = TRUE;;

        if (chromatix_gtm_v2->gtm_v2_enable) {
            if (!gtm->is_aec_update_valid) {
              /* default to NORMAL_LIGHT when aec_update isn't available yet */
              algo_param->params.gtm_core =
                chromatix_gtm_v2->gtm_v2[NORMAL_LIGHT].gtm_exp_data[2].gtm_core_v2;
              algo_param->params.gtm_reserve =
                chromatix_gtm_v2->gtm_v2[NORMAL_LIGHT].gtm_exp_data[2].gtm_reserve_v2;
            } else
              ret = gtm46_v2_compute_algo_params(gtm, algo_param, chromatix_gtm_v2);
        } else {
            if (!gtm->is_aec_update_valid) {
                  algo_param->params = chromatix_gtm->gtm[2];
            } else
              ret = gtm46_compute_algo_params(gtm, algo_param, chromatix_gtm);
        }
        if (ret == FALSE) {
          ISP_ERR("failed: compute_algo_params ret %d, gtm_v2_enable:%d",
            ret, chromatix_gtm_v2->gtm_v2_enable);
          return FALSE;
        }

        if (!gtm->algo_update_valid){
         /* Calculate the default curve */
          if (!gtm46_calculate_default_cuve(gtm,algo_param)){
            ISP_ERR("failed in calculating default curve \n");
            default_curve_valid = FALSE;
          }
        }

        /* pack DMI table */
        gtm46_pack_dmi(gtm->algo_output.gtm_yout, gtm_xin_tbl,
          gtm->dmi_tbl, gtm->base, gtm->slope);
      } else {
        /* pack DMI table */
        gtm46_pack_dmi_fix_curve(gtm->dmi_tbl,
          yratio_base, yratio_slope);
      }

  if (!gtm->mod_enable) {
    ISP_ERR("Region not enabled in GTM. Skip updates");
    return FALSE;
  }

  return ret;
}

