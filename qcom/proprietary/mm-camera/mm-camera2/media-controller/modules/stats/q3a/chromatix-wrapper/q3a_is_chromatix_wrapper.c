/* q3a_is_chromatix_wrapper.c
 *
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "chromatix_3a.h"
#include "q3a_is_chromatix_wrapper.h"

void parse_EIS_chromatix(is_chromatix_info_t *is_chromatix_info, void *chromatix3APtr)
{
  Chromatix_EIS_algo_type *chromatix_EIS =
    &(((chromatix_3a_parms_type *)chromatix3APtr)->chromatix_EIS_data);

  is_chromatix_info->focal_length = chromatix_EIS->focal_length;
  is_chromatix_info->gyro_pixel_scale = chromatix_EIS->gyro_pixel_scale;
  is_chromatix_info->virtual_margin = chromatix_EIS->virtual_margin;
  is_chromatix_info->gyro_noise_floor = chromatix_EIS->gyro_noise_floor;
  is_chromatix_info->gyro_frequency = chromatix_EIS->gyro_frequency;
  is_chromatix_info->rs_offset_1 = chromatix_EIS->rs_offset_1;
  is_chromatix_info->rs_offset_2 = chromatix_EIS->rs_offset_2;
  is_chromatix_info->rs_offset_3 = chromatix_EIS->rs_offset_3;
  is_chromatix_info->rs_offset_4 = chromatix_EIS->rs_offset_4;
  is_chromatix_info->s3d_offset_1 = chromatix_EIS->s3d_offset_1;
  is_chromatix_info->s3d_offset_2 = chromatix_EIS->s3d_offset_2;
  is_chromatix_info->s3d_offset_3 = chromatix_EIS->s3d_offset_3;
  is_chromatix_info->s3d_offset_4 = chromatix_EIS->s3d_offset_4;
  is_chromatix_info->rs_threshold_1 = chromatix_EIS->rs_threshold_1;
  is_chromatix_info->rs_threshold_2 = chromatix_EIS->rs_threshold_2;
  is_chromatix_info->rs_threshold_3 = chromatix_EIS->rs_threshold_3;
  is_chromatix_info->s3d_threshold_1 = chromatix_EIS->s3d_threshold_1;
  is_chromatix_info->s3d_threshold_2 = chromatix_EIS->s3d_threshold_2;
  is_chromatix_info->s3d_threshold_3 = chromatix_EIS->s3d_threshold_3;
  is_chromatix_info->rs_time_interval_1 = chromatix_EIS->rs_time_interval_1;
  is_chromatix_info->rs_time_interval_2 = chromatix_EIS->rs_time_interval_2;
  is_chromatix_info->rs_time_interval_3 = chromatix_EIS->rs_time_interval_3;
  is_chromatix_info->rs_time_interval_4 = chromatix_EIS->rs_time_interval_4;
  is_chromatix_info->reserve_1 = chromatix_EIS->reserve_1;
  is_chromatix_info->reserve_2 = chromatix_EIS->reserve_2;
  is_chromatix_info->reserve_3 = chromatix_EIS->reserve_3;
  is_chromatix_info->reserve_4 = chromatix_EIS->reserve_4;

}

