//======================================================================
// Copyright (c) 2016 Qualcomm Technologies, Inc.
// All Rights Reserved.
// Confidential and Proprietary - Qualcomm Technologies, Inc.
//======================================================================

#ifndef CHROMATIX_IOT_H
#define CHROMATIX_IOT_H

#include "chromatix.h"

#define SHDR_EXP_LUT_ENTRIES    20

typedef struct
{
    unsigned short chromatix_version;
    unsigned short IOT_header_version;
} IOT_version_type;

//=============================================================================
// Feature name: Exposure LUT  Parameters
//
// Variable name: exp_ratio
// exposure ratio
// default: 16.0f
//
// Variable name: gtm_gamma
// GPU-GTM gamma
// default: 0.65f
//
// Variable name: exp_change_up_th
// up threshold for changing exposure ratio
// default: 1.5f
//
// Variable name: exp_change_low_th
// low threshold for changing exposure ratio
// default: 1.0f
//
// Variable name: non_linear_low_th
// low threshold for non_linear of bhist
// default: 4
//
// Variable name: non_linear_hi_th
// high threshold for non_linear of bhist
// default: 220
//
// Variable name: reserved[2]
// reserved for potential use
// default: 0.0f
//=============================================================================
typedef struct
{
    float   exp_ratio;
    float   gtm_gamma;
    float   exp_change_up_th;
    float   exp_change_low_th;
    int     non_linear_low_th;
    int     non_linear_hi_th;
    float   reserved[10];
} exp_lut_type;

//=============================================================================
// Feature name: Staggered HDR Parameters
//
// Variable name: num_lut_entries
// default: 8
//
// Variable name: temporal_filter1
// default: 0.02f
//
// Variable name: temporal_filter2
// default: 1.0f
//
// Variable name: reserved[6]
// reserved for potential use
// default: 0
//=============================================================================
typedef struct
{
    int          shdr_enable;
    exp_lut_type exp_lut[SHDR_EXP_LUT_ENTRIES];
    int          num_lut_entries;
    float        temporal_filter1;
    float        temporal_filter2;
    float        reserved[200];
} chromatix_shdr_type;

//=============================================================================
// Feature name : ir_led_adjust_type
// IR LED params for IR mode
//
// Variable name: ir_led_enable
// Enable IR LED params
//
// Variable name: ir_led_intensity_unit
// IR LED intensity unit
//
// Variable name: ir_led_intensity_idx_max
// Maximum IR LED Index
//
// Variable name: ir_led_intensity_idx_min
// Minimum IR LED Index
//=============================================================================
typedef struct
{
    int     ir_led_enable;
    int     ir_led_intensity_unit;
    int     ir_led_intensity_idx_max;
    int     ir_led_intensity_idx_min;
} ir_led_adjust_type;

//=============================================================================
// Feature name: ir_als_adjust_type
// Ambient light sensor params for IR mode
//
// Variable name: ir_lightsensor_enable
// Enable Ambient Light Sensor
//
// Variable name: day2ir_switch_pt
// Day to IR mode switch point
//
// Variable name: ir2day_switch_pt
// IR to Day mode switch point
//=============================================================================
typedef struct
{
    int     ir_lightsensor_enable;
    float   day2ir_switch_pt;
    float   ir2day_switch_pt;
} ir_als_adjust_type;

//=============================================================================
// Feature name: ir_daylight_distribution_adjust_type
//  Normal light distribution params for IR mode
//
// Variable name: normal_light_cnt_thres
// Count threshold of detected normal light
//
// Variable name: normal_light_target_thres
// Target_G threshold of detected normal light
//
// Variable name: normal_light_GB_ratio_max
// Maximum GB ratio to detect normal light
//
// Variable name: normal_light_GB_ratio_min
// Minimum GB ratio to detect normal light
//
// Variable name: normal_light_GR_ratio_max
// Maximum GR ratio to detect normal light
//
// Variable name: normal_light_GR_ratio_min
// Minimum GR ratio to detect normal light
//
// Variable name: normal_light_RB_ratio_max
// Maximum RB ratio to detect normal light
//
// Variable name: normal_light_RB_ratio_min
// Minimum RB ratio to detect normal light
//
// Variable name: ir_distribution_weight[16][16]
// Weight for calculating normal mode distribution (16x16)
//=============================================================================
typedef struct
{
    float   normal_light_cnt_thres;
    float   normal_light_target_thres;
    int     normal_light_GB_ratio_max;
    int     normal_light_GB_ratio_min;
    int     normal_light_GR_ratio_max;
    int     normal_light_GR_ratio_min;
    int     normal_light_RB_ratio_max;
    int     normal_light_RB_ratio_min;
    float   ir_distribution_weight[16][16];
} ir_daylight_distribution_adjust_type;

//=============================================================================
// Feature name: chromatix_ir_mode_type
// IR Mode enable
//
// Variable name: ir_mode_enable
// Enable or Disable IR Mode feature
//
// Variable name: ir_center_weight[16][16]
// Weight for calculating luma of IR mode  (16x16)
//
// Variable name: ir_highlight_suppress_weight[64]
// Weight for avoid object over-exposure (64)
//
// Variable name: ir_mode_conv_speed
// AEC converge speed for IR mode
//
// Variable name: day2ir_exp_index
//  Day to IR exp_index to trigger IR LED
//
// Variable name: ir2day_exp_index
//  IR to Day exp_index to turn off IR LED
//
// Variable name: ir_mode_day2ir_check_cnt
//  Stable count of Day to IR mode switching
//
// Variable name: ir_mode_ir2day_check_cnt
//  Stable count of IR to Day mode switching
//
// Variable name: ir_mode_skip_check_cnt
//  Next check period after IR/Day switching
//
// Variable name: ir_mode_aec_skip_cnt
//  Skip frames of AEC after IR cut switched
//=============================================================================
typedef struct
{
    int                 ir_mode_enable;
    ir_led_adjust_type  ir_led_adj;
    ir_als_adjust_type  ir_als_adj;
    float               ir_center_weight[16][16];
    float               ir_highlight_suppress_weight[64];
    float               ir_mode_conv_speed;
    int                 day2ir_exp_index;
    int                 ir2day_exp_index;
    int                 ir_mode_day2ir_check_cnt;
    int                 ir_mode_ir2day_check_cnt;
    int                 ir_mode_skip_check_cnt;
    int                 ir_mode_aec_skip_cnt;
    ir_daylight_distribution_adjust_type ir_daylight_dist_adj;
    float               reserved[200];
} chromatix_ir_mode_type;

typedef struct
{
    //=========================================================================
    // Chromatix header version info (MUST BE THE FIRST PARAMETER)
    //=========================================================================
    ChromatixVersionInfoType        chromatix_version_info;

    IOT_version_type                iot_version;

    //=========================================================================
    // SHDR
    //=========================================================================
    chromatix_shdr_type             shdr_algo_data;

    //=========================================================================
    // AEC
    //=========================================================================
    chromatix_ir_mode_type          irmode_data;
} chromatix_iot_parms_type;

#endif
