/* is_set.c
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include "is.h"
#include "q3a_is_chromatix_wrapper.h"

static boolean is_set_init_chromatix(is_info_t *is_info,
  is_set_parameter_init_t *init_param)
{
  if (is_info == NULL || init_param == NULL
      || init_param->chromatix == NULL) {
    return FALSE;
  }

  void *chromatix3APtr = init_param->chromatix;

  parse_EIS_chromatix(&(is_info->is_chromatix_info),
    chromatix3APtr);

  return TRUE;
}

/** is_set_parameters:
 *    @param: information about parameter to be set
 *    @is_info: IS information
 *
 * Returns TRUE ons success
 **/
boolean is_set_parameters(is_set_parameter_t *param, is_info_t *is_info)
{
  boolean rc = TRUE;

  switch (param->type) {
  case IS_SET_PARAM_STREAM_CONFIG:
     IS_LOW("IS_SET_PARAM_STREAM_CONFIG, ma = %u, p = %d",
      param->u.is_sensor_info.sensor_mount_angle,
      param->u.is_sensor_info.camera_position);
    is_info->sensor_mount_angle = param->u.is_sensor_info.sensor_mount_angle;
    is_info->camera_position = param->u.is_sensor_info.camera_position;
    break;

  case IS_SET_PARAM_DIS_CONFIG:
    if (param->u.is_config_info.stream_type == CAM_STREAM_TYPE_VIDEO) {
      is_info->width[IS_VIDEO] = param->u.is_config_info.width;
      is_info->height[IS_VIDEO] = param->u.is_config_info.height;
      IS_HIGH("IS_SET_PARAM_DIS_CONFIG, vwidth = %ld, vheight = %ld",
        is_info->width[IS_VIDEO], is_info->height[IS_VIDEO]);
    } else {
      /* Must be CAM_STREAM_TYPE_PREVIEW because other types are filtered out
         by poster of this event */
      is_info->width[IS_PREVIEW] = param->u.is_config_info.width;
      is_info->height[IS_PREVIEW] = param->u.is_config_info.height;
      IS_HIGH("IS_SET_PARAM_DIS_CONFIG, pwidth = %ld, pheight = %ld",
        is_info->width[IS_PREVIEW], is_info->height[IS_PREVIEW]);
    }
    break;

  case IS_SET_PARAM_OUTPUT_DIM:
    break;

  case IS_SET_PARAM_CHROMATIX: {
    is_set_init_chromatix(is_info, &param->u.is_init_param);

     IS_LOW("IS_SET_PARAM_CHROMATIX\nvirtual margin = %f\n"
      "gyro noise floor = %f\ngyro pixel scale = %d",
      is_info->is_chromatix_info.virtual_margin,
      is_info->is_chromatix_info.gyro_noise_floor,
      is_info->is_chromatix_info.gyro_pixel_scale);
    }
    break;

  case IS_SET_PARAM_IS_ENABLE:
    is_info->is_enabled[IS_PREVIEW] = param->u.is_enable;
    is_info->is_enabled[IS_VIDEO] = param->u.is_enable;
    IS_LOW("IS_SET_PARAM_IS_ENABLE, IS enable = %u", param->u.is_enable);
    break;

  default:
    rc = FALSE;
    break;
  }

  return rc;
} /* is_set_parameters */
