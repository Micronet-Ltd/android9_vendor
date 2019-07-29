/***************************************************************************
* Copyright (c) 2013-2015 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#include "module_imgbase.h"

/** UBIFOCUS_BURST_CNT:
 *
 *  Burst count
 **/
#define UBIFOCUS_BURST_CNT 5

/**
 *  Static functions
 **/
static boolean module_ubifocus_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* /*p_mod*/);
static boolean module_ubifocus_init_params(img_init_params_t *p_params);

/** g_focus_steps:
 *
 *  Focus steps
 **/
static uint8_t g_focus_steps[MAX_AF_BRACKETING_VALUES] =
  {1, 1, 1, 1, 1};

/** g_caps:
 *
 *  Set the capabilities for ubifocus module
 **/
static img_caps_t g_caps = {
  .num_input = UBIFOCUS_BURST_CNT,
  .num_output = 1,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 1,
};

/** g_params: asd
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_query_mod = module_ubifocus_query_mod,
  .imgbase_client_init_params = module_ubifocus_init_params,
};

/**
 * Function: module_ubifocus_init_params
 *
 * Description: This function is used to init parameters
 *
 * Arguments:
 *   p_params - ubifocus init params
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_ubifocus_init_params(img_init_params_t *p_params)
{
  boolean ret = FALSE;
  if (p_params) {
    p_params->refocus_encode = 0;
    ret = TRUE;
  }
  return ret;
}

/**
 * Function: module_ubifocus_deinit
 *
 * Description: This function is used to deinit ubifocus
 *               module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_ubifocus_deinit(mct_module_t *p_mct_mod)
{
  module_imgbase_deinit(p_mct_mod);
}

/**
 * Function: module_ubifocus_query_mod
 *
 * Description: This function is used to query ubifocus
 *               caps
 *
 * Arguments:
 *   @p_mct_cap - capababilities
 *   @p_mod - pointer to the module
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_ubifocus_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* p_mod)
{
  mct_pipeline_imaging_cap_t *buf;

  IMG_UNUSED(p_mod);
  if (!p_mct_cap) {
    IDBG_ERROR("%s:%d] Error", __func__, __LINE__);
    return FALSE;
  }

  buf = &p_mct_cap->imaging_cap;
  buf->ubifocus_af_bracketing_need.burst_count = UBIFOCUS_BURST_CNT;
  buf->ubifocus_af_bracketing_need.enable = TRUE;
  buf->ubifocus_af_bracketing_need.output_count = 1;
  buf->ubifocus_af_bracketing_need.meta_max_size = 0;
  memcpy(&buf->ubifocus_af_bracketing_need.focus_steps, &g_focus_steps,
    sizeof(g_focus_steps));
  return TRUE;
}

/** module_ubifocus_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the ubifocus
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_ubifocus_init(const char *name)
{
  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    NULL,
    &g_caps,
    "libmmcamera_ubifocus_lib.so",
    CAM_QCOM_FEATURE_UBIFOCUS,
    &g_params);
}
