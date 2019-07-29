/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef __PDAF48_H__
#define __PDAF48_H__

/* std headers */
#include <stdio.h>
#include <unistd.h>
#include <math.h>

/* mctl headers */
#include "chromatix.h"
#include "chromatix_common.h"

/* isp headers */
#include "isp_defs.h"
#include "isp_common.h"
#include "isp_log.h"
#include "isp_sub_module_log.h"
#include "isp_sub_module_common.h"
#include "isp_sub_module.h"
#include "isp_sub_module_port.h"
#include "isp_sub_module_util.h"
#include "isp_pipeline_reg.h"

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/* sensor headers */
#include "pdaf_lib.h"

/* pdaf headers */
#include "pdaf_reg.h"

#define PDAF48_VERSION "48"
#define PDAF48_MODULE_NAME(n) \
  "pdaf"n

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_PDAF, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_PDAF, fmt, ##args)

#define DEFAULT_CONFIG_INDEX            0
#define PDAF_MIN_FMAX                   0
#define PDAF_MAX_FMAX                   255 // ((1<<8)-1)
#define PDAF_MIN_FMIN                   0
#define PDAF_MAX_FMIN                   255 // ((1<<8)-1)
#define PDAF_MIN_OFFSET                 0
#define PDAF_MAX_OFFSET                 32767 // ((1<<15)-1)
#define PDAF_MIN_AWB_GAIN_RATIO         0
#define PDAF_MAX_AWB_GAIN_RATIO         16384 // ((1<<14))
#define PDAF_MIN_HDR_EXP_RATIO          0
#define PDAF_MAX_HDR_EXP_RATIO          16384 // ((1<<14))
#define PDAF_MIN_HDR_EXP_RATIO_RECIP    16
#define PDAF_MAX_HDR_EXP_RATIO_RECIP    256 // ((1<<8))
#define PDAF_MIN_LOC_OFFSET             0
#define PDAF_MAX_LOC_OFFSET             16383 // ((1<<14)-1)

/* This needs to be defined in isp_pipeline_48 header */
#define PDAF_LUT_RAM_BANK               0xB

#define AWB_GAIN_EQUAL(g1, g2) ( \
  F_EQUAL(g1->r_gain, g2->r_gain) && \
  F_EQUAL(g1->g_gain, g2->g_gain) && \
  F_EQUAL(g1->b_gain, g2->b_gain))

#define AWB_GAIN_EQ_ZERO(g1) ( \
  F_EQUAL(g1->r_gain, 0.0) || \
  F_EQUAL(g1->g_gain, 0.0) || \
  F_EQUAL(g1->b_gain, 0.0))

#define CLAMP(a,b,c) (((a) <= (b))? (b) : (((a) >= (c))? (c) : (a)))

typedef struct {
  int            lut[PDAF_LUT_COUNT];
}__attribute__((packed, aligned(4)))pdaf_packed_lut_t;

/** pdaf48_t:
 *
 *  @aec_update: aec update info
 *  @curr_real_gain: current real gain
 *  @curr_awb_gain: gain from awb update
 *  @sensor_out_info: sensor information
 **/
typedef struct {
  ISP_PDAF_Reg_t                   pdaf_reg;
  float                            curr_real_gain;
  awb_gain_t                       curr_awb_gain;
  sensor_out_info_t                sensor_out_info;
  uint8_t                          trigger_index;
  float                            aec_ratio;
  float                            hdr_exp_ratio;
  pdaf_block_pattern_t             pdaf_pattern;
  pdaf_packed_lut_t                packed_lut;
  uint8_t                          one_time_config_done;
} pdaf48_t;

boolean pdaf48_init(isp_sub_module_t *isp_sub_module);

void pdaf48_destroy(isp_sub_module_t *isp_sub_module);

boolean pdaf48_streamon(isp_sub_module_t *isp_sub_module, void *data);

boolean pdaf48_streamoff(isp_sub_module_t *isp_sub_module, void *data);

boolean pdaf48_set_chromatix_ptr(isp_sub_module_t *isp_sub_module,
  void *data);

boolean pdaf48_stats_aec_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean pdaf48_stats_awb_update(isp_sub_module_t *isp_sub_module,
  void *data);

boolean pdaf48_set_pdaf_pixel(isp_sub_module_t *isp_sub_module,
  void *data);

boolean pdaf48_trigger_update(isp_sub_module_t *isp_sub_module,
  void *data);

#endif /* __PDAF48_H__ */
