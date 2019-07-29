/* gamma_0309.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "module_gamma.h"
#include "isp_sub_module_util.h"
#include "isp_common.h"
#include "isp_sub_module_log.h"
#include "isp_defs.h"
#include "isp_pipeline_reg.h"
#include "gamma44.h"

/** gamma44_set_same_channel_data_flag:
 *
 *  This function sets same channel data flag
 *
 **/

boolean gamma44_set_same_channel_data_flag(isp_sub_module_t *isp_sub_module)
{
  if (!isp_sub_module) {
    ISP_ERR("failed: %p", isp_sub_module);
    return FALSE;
  }
  return TRUE;
}

/** gamma44_compute_contrast_table:
 *
 *    @sigmoid: sigmoid is curvature strength to be applied. This values depends
 *                  on contrast value from user.
 *    @lut_size: look up table size
 *    @input_table: input rgb table
 *    @output_table: output rgb table
 *
 *  This function updates gamma table depending on contrast
 *
 *  Return: TRUE  - Success
 *          FALSE - Input/Output table is NULL or input values are invalid
 **/
boolean gamma44_compute_contrast_table(double sigmoid, int lut_size,
  gamma_rgb_ds_t  *input_table_rgb, gamma_rgb_ds_t  *output_table_rgb)
{
  int             i = 0, x = 0;
  uint16_t       *gamma_table = NULL;
  int             entries = (lut_size >= 0) ? lut_size : ISP_GAMMA_NUM_ENTRIES;
  int             mid_pt = 0;
  if (input_table_rgb == NULL || output_table_rgb == NULL) {
    ISP_ERR(": input improper");
    return FALSE;
  }

  /* Generate Sigmoid Gamma Table entries*/
  mid_pt = isp_sub_module_util_power(2.0, GAMMA_HW_PACK_BIT) / 2;
  const double multiplier =
    isp_sub_module_util_power((double)mid_pt, (1.0 - sigmoid));

  ISP_DBG("sigmoid %f mid_pt %d multiplier %f entries %d",
    sigmoid, mid_pt, multiplier, entries);
  for (i = 0; i < ISP_GAMMA_NUM_ENTRIES; i++) {
    output_table_rgb->gamma_r[i] = gamma44_apply_contrast_sigmoid(sigmoid,
      multiplier, input_table_rgb->gamma_r[i], mid_pt);
    output_table_rgb->gamma_g[i] = gamma44_apply_contrast_sigmoid(sigmoid,
      multiplier, input_table_rgb->gamma_g[i], mid_pt);
    output_table_rgb->gamma_b[i] = gamma44_apply_contrast_sigmoid(sigmoid,
      multiplier, input_table_rgb->gamma_b[i], mid_pt);
    /* 8994 has 512 entries with output 14bits.
     * 8084 has 256 gamma entries with output 12bits, SW needs
     * to downsample gamma by 2 and right shift 2 bits of the table values
     */
    ISP_DBG("%d: input %d output %d", i, input_table_rgb->gamma_r[i],
      output_table_rgb->gamma_r[i]);
  }
  return TRUE;
}

/** gamma44_update_vfe_table:
 *    @gamma_config_cmd: configuration command
 *    @table_rgb: pointer to table
 *    @size: size of the table
 *
 *  This function updates gamma table depending on contrast
 *
 *  Return: TRUE  - Success
 *             FALSE - Input/Output table is NULL or contrast value is invalid
 **/

void gamma44_update_vfe_table(volatile ISP_GammaConfigCmdType *
  gamma_config_cmd, gamma_rgb_ds_t *table_rgb)
{
  int             i = 0;
  int             skipRatio = 0;

  if (!table_rgb) {
    ISP_ERR("failed: table_rgb %p", table_rgb);
    return;
  }
  uint16_t *table_r = &table_rgb->gamma_r[0];
  uint16_t *table_g = &table_rgb->gamma_g[0];
  uint16_t *table_b = &table_rgb->gamma_b[0];

  //Finally compute the LUT entries
  for (i = 0; i < ISP_GAMMA_NUM_ENTRIES - 1; i++) {
    gamma_config_cmd->Gamatbl.hw_table_r[i] =
      gamma44_get_hi_lo_gamma_bits(table_r, i);
    gamma_config_cmd->Gamatbl.hw_table_g[i] =
      gamma44_get_hi_lo_gamma_bits(table_g, i);
    gamma_config_cmd->Gamatbl.hw_table_b[i] =
      gamma44_get_hi_lo_gamma_bits(table_b, i);
  }

  gamma_config_cmd->Gamatbl.hw_table_r[ISP_GAMMA_NUM_ENTRIES - 1] =
    gamma44_get_last_gamma_value(table_r);
  gamma_config_cmd->Gamatbl.hw_table_g[ISP_GAMMA_NUM_ENTRIES - 1] =
    gamma44_get_last_gamma_value(table_g);
  gamma_config_cmd->Gamatbl.hw_table_b[ISP_GAMMA_NUM_ENTRIES - 1] =
    gamma44_get_last_gamma_value(table_b);
} /* gamma_update_vfe_table */

/** gamma44_copy_gamma
 *
 *  @dst: destination
 *  @src: src
 *  @size: size
 *
 *  This function copies rgb gamma table
 *
 *  Return: None
 */
void gamma44_copy_gamma(gamma_rgb_ds_t *dst,
  gamma_rgb_ds_t *src, size_t size)
{
  if (!dst || !src) {
    ISP_ERR("failed: %p %p", dst, src);
    return;
  }
  memcpy(dst->gamma_r, src->gamma_r, size * sizeof(uint16_t));
  memcpy(dst->gamma_g, src->gamma_g, size * sizeof(uint16_t));
  memcpy(dst->gamma_b, src->gamma_b, size * sizeof(uint16_t));
}

/** gamma44_chromatix_to_isp_gamma
 *
 *  @p_gamma_rgb: isp gamma rgb table
 *  @chromatix_gamma_table: chromoatix gamma table
 *
 *  This function initializes gamma table from chromatix
 *
 *  Return: None
 */
void gamma44_chromatix_to_isp_gamma(gamma_rgb_t *gamma_rgb,
  chromatix_gamma_table_type *chromatix_gamma_table)
{
  int           i = 0;

  if (!gamma_rgb || !chromatix_gamma_table) {
    ISP_ERR("failed: %p %p", gamma_rgb, chromatix_gamma_table);
    return;
  }

  memcpy(gamma_rgb->gamma_r, chromatix_gamma_table->gamma_R,
    GAMMA_CHROMATIX_LUT_SIZE * sizeof(uint16_t));
  memcpy(gamma_rgb->gamma_g, chromatix_gamma_table->gamma_G,
    GAMMA_CHROMATIX_LUT_SIZE * sizeof(uint16_t));
  memcpy(gamma_rgb->gamma_b, chromatix_gamma_table->gamma_B,
    GAMMA_CHROMATIX_LUT_SIZE * sizeof(uint16_t));
}

/** gamma44_interpolate
 *
 *    @tbl1: input table 1
 *    @tbl2: input table 2
 *    @out: out table
 *    @ratio:
 *
 *  Interpolate for each of tree colors RGB
 *  and summurise result in one table
 *
 *  Return: None
 */
void gamma44_interpolate(gamma_rgb_ds_t  *tbl1,
    gamma_rgb_ds_t  *tbl2, gamma_rgb_ds_t  *gamma_rgb,
  float ratio)
{
  int   i = 0;
  if (!tbl1 || !tbl2 || !gamma_rgb) {
    ISP_ERR("failed: %p %p %p", tbl1, tbl2, gamma_rgb);
    return;
  }

  TBL_INTERPOLATE_GAMMA(tbl1->gamma_r, tbl2->gamma_r, gamma_rgb->gamma_r,
    ratio, ISP_GAMMA_NUM_ENTRIES, i);
  TBL_INTERPOLATE_GAMMA(tbl1->gamma_g, tbl2->gamma_g, gamma_rgb->gamma_g,
    ratio, ISP_GAMMA_NUM_ENTRIES, i);
  TBL_INTERPOLATE_GAMMA(tbl1->gamma_b, tbl2->gamma_b, gamma_rgb->gamma_b,
    ratio, ISP_GAMMA_NUM_ENTRIES, i);
}

/** gamma44_dmi_hw_update:
 *    @gamma: gamma module
 *    @bank_sel: mem bank select
 *    @isp_sub_module: base module
 *
 *  This function updates DMI tables for gamma
 *
 *  Return:   TRUE - Success FALSE- failure
 **/
boolean gamma44_dmi_hw_update(void *data1,
  ISP_GammaLutSelect bank_sel, void *data2)
{
  gamma44_t *gamma = (gamma44_t *)data1;
  isp_sub_module_t *isp_sub_module = (isp_sub_module_t *)data2;
  uint32_t tbl_len = sizeof(int32_t) * ISP_GAMMA_NUM_ENTRIES;
  boolean  ret;

  if (!gamma || !isp_sub_module) {
    ISP_ERR("failed: %p %p", gamma, isp_sub_module);
    return FALSE;
  }

  ret = isp_sub_module_util_write_dmi(
    (void*)gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_g, tbl_len,
    RGBLUT_RAM_CH0_BANK0 + bank_sel.ch0BankSelect,
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_g");
    return FALSE;
  }

  ret = isp_sub_module_util_write_dmi(
    (void*)gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_b, tbl_len,
    RGBLUT_RAM_CH1_BANK0 + bank_sel.ch1BankSelect,
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_b");
    return FALSE;
  }

  ret = isp_sub_module_util_write_dmi(
    (void*)gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_r, tbl_len,
    RGBLUT_RAM_CH2_BANK0 + bank_sel.ch2BankSelect,
    VFE_WRITE_DMI_32BIT, ISP_DMI_CFG_OFF, ISP_DMI_ADDR, isp_sub_module);
  if (ret == FALSE) {
    ISP_ERR("failed writing table_r");
    return FALSE;
  }

  return TRUE;
} /* gamma44_dmi_hw_update */

/** gamma44_get_downsampled_tables:
 *
 *  @tbl_in: table to be downsampled. If NULL
 *           use the rgb_in table
 *  @rgb_in: table with all R,G,B entries. Use
 *           the corresponding table for each
 *           of the components. If NULL use
 *           the tbl_in
 *  @skip_ratio: number of entries to be skipped
 *
 *  This function populates gamma R, G, B values
 *  from the chromatix tables based on skip ratio.
 **/
void gamma44_get_downsampled_tables(gamma_rgb_ds_t *rgb_tbl,
  uint16_t *tbl_in, gamma_rgb_t *rgb_in, int skip_ratio)
{
  if (!tbl_in && !rgb_in) {
    ISP_ERR("Something wrong. Both pointers are NULL");
    return;
  }

  if (tbl_in) {
    gamma44_get_downsampled_table(&rgb_tbl->gamma_r[0],
      tbl_in, skip_ratio);
    gamma44_get_downsampled_table(&rgb_tbl->gamma_g[0],
      tbl_in, skip_ratio);
    gamma44_get_downsampled_table(&rgb_tbl->gamma_b[0],
      tbl_in, skip_ratio);
  } else {
    gamma44_get_downsampled_table(&rgb_tbl->gamma_r[0],
      &rgb_in->gamma_r[0], skip_ratio);
    gamma44_get_downsampled_table(&rgb_tbl->gamma_g[0],
      &rgb_in->gamma_g[0], skip_ratio);
    gamma44_get_downsampled_table(&rgb_tbl->gamma_b[0],
      &rgb_in->gamma_b[0], skip_ratio);
  }
}

/** gamma44_update_tone_map_curves:
 *
 *  @data1: meta data
 *  @data2: gamma info
 *  @val: tone map mode
 *
 *  This function populates gamma curves information in the meta
 *  data.
 **/
void gamma44_update_tone_map_curves(void * data1, void * data2, int val)
{
  isp_per_frame_meta_t *out_frame_meta = (isp_per_frame_meta_t *)data1;
  gamma44_t *gamma = (gamma44_t *)data2;
  cam_tonemap_mode_t tonemap_mode = (cam_tonemap_mode_t)val;
  int i = 0;

  out_frame_meta->bestshot_mode = gamma->bestshot_mode;
  if (gamma->effect_applied == TRUE)
    out_frame_meta->special_effect = gamma->effects.spl_effect;
  out_frame_meta->contrast = gamma->effects.contrast;

  out_frame_meta->tonemap_mode = tonemap_mode;

  //Populate CAM_INTF_META_TONEMAP_CURVES
  out_frame_meta->tone_map.tonemap_points_cnt = ISP_GAMMA_NUM_ENTRIES;
  /* ch0 = G, ch 1 = B, ch 2 = R*/
  for (i=0; i<ISP_GAMMA_NUM_ENTRIES; i++) {
    out_frame_meta->tone_map.curves[0].tonemap_points[i][0] =
      (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);

    out_frame_meta->tone_map.curves[0].tonemap_points[i][1] =
      (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_g[i] &
              GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;

    out_frame_meta->tone_map.curves[1].tonemap_points[i][0] =
      (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);
    out_frame_meta->tone_map.curves[1].tonemap_points[i][1] =
      (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_b[i] &
              GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;

    out_frame_meta->tone_map.curves[2].tonemap_points[i][0] =
      (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);
    out_frame_meta->tone_map.curves[2].tonemap_points[i][1] =
      (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_r[i] &
              GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;

    /* profile_tone_curve */
    out_frame_meta->tone_curve.curve.tonemap_points[i][0] =
      (float)i / (ISP_GAMMA_NUM_ENTRIES - 1);
    out_frame_meta->tone_curve.curve.tonemap_points[i][1] =
      (float)(gamma->ISP_GammaCfgCmd.Gamatbl.hw_table_g[i] &
              GAMMA_HW_UNPACK_MASK) / MAX_GAMMA_VALUE;
  }
}

/** gamma44_set_solarize_table:
 *
 *    @gamma: gamma module instance
 *    @chromatix_ptr: pointer to chromatix
 *
 *  This function sets solarize table
 *
 *  Return:   None
 **/
void gamma44_set_solarize_table(gamma44_t *gamma,
  chromatix_parms_type *chromatix_ptr)
{
  /* FIXME: There is no solorize reflection point in chromatix 301 */
  int                   i = 0;
  uint16_t              solarize_reflection_point = 32;
  uint16_t              gamma_max_value = 0x3FFF;
  gamma_rgb_ds_t           Gamma_Lut;
  chromatix_gamma_type *pchromatix_gamma = NULL;
  uint16_t reflect_gam_value_R = 1, reflect_gam_value_G = 1, reflect_gam_value_B = 1;

  if (!gamma || !chromatix_ptr) {
    ISP_ERR("failed: gamma %p ctrl_event %p", gamma, chromatix_ptr);
    return;
  }
  pchromatix_gamma =
    &(chromatix_ptr->chromatix_VFE.chromatix_gamma);

  /* Chromatix reflection point is based on 256 (8 bit) gamma entries,
   * whereas VFE gamma has 1024 (10 bit) entries. So we multiple the
   * reflection point by 4 here */
  gamma44_copy_gamma(&Gamma_Lut, &gamma->default_gamma_tbl,
    ISP_GAMMA_NUM_ENTRIES);

  reflect_gam_value_G = (gamma_max_value - Gamma_Lut.gamma_g[solarize_reflection_point - 1]);
  if(!reflect_gam_value_G) reflect_gam_value_G = 1;

  reflect_gam_value_R = (gamma_max_value - Gamma_Lut.gamma_r[solarize_reflection_point - 1]);
  reflect_gam_value_B = (gamma_max_value - Gamma_Lut.gamma_b[solarize_reflection_point - 1]);
  if(!reflect_gam_value_R) reflect_gam_value_R = 1;
  if(!reflect_gam_value_B) reflect_gam_value_B = 1;

  for (i = solarize_reflection_point - 1; i < ISP_GAMMA_NUM_ENTRIES; ++i) {
    gamma->solarize_gamma_table.gamma_g[i] =
      ((gamma_max_value - Gamma_Lut.gamma_g[i]) *
        Gamma_Lut.gamma_g[solarize_reflection_point - 1] /
        reflect_gam_value_G);

    gamma->solarize_gamma_table.gamma_r[i] =
      ((gamma_max_value - Gamma_Lut.gamma_r[i]) *
        Gamma_Lut.gamma_r[solarize_reflection_point - 1] /
        reflect_gam_value_R);

    gamma->solarize_gamma_table.gamma_b[i] =
      ((gamma_max_value - Gamma_Lut.gamma_b[i]) *
        Gamma_Lut.gamma_b[solarize_reflection_point - 1] /
        reflect_gam_value_B);
  }
} /* gamma44_set_solarize_table */


void gamma44_send_meta_data(
  isp_meta_entry_t *gamma_dmi_info,
  gamma44_t *gamma)
{
  memcpy(&gamma_dmi_info->isp_meta_dump[0],
    &gamma->applied_table.hw_table_r[0],
    sizeof(uint32_t) * ISP_GAMMA_NUM_ENTRIES);
  memcpy(&gamma_dmi_info->isp_meta_dump[
    ISP_GAMMA_NUM_ENTRIES * sizeof(uint32_t)],
    &gamma->applied_table.hw_table_g[0],
    sizeof(uint32_t) * ISP_GAMMA_NUM_ENTRIES);
  memcpy(&gamma_dmi_info->isp_meta_dump[
    ISP_GAMMA_NUM_ENTRIES * 2 * sizeof(uint32_t)],
    &gamma->applied_table.hw_table_b[0],
    sizeof(uint32_t) * ISP_GAMMA_NUM_ENTRIES);
}
