/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "faceproc_hw_comp.h"
#ifdef FACEPROC_USE_NEW_WRAPPER
#include "faceproc_sw_wrapper.h"
#else
#include "facial_parts_wrapper.h"
#endif
#include "faceproc_hw_tracker_wrapper.h"
#include <media/msm_fd.h>
#include <math.h>
#include <poll.h>

static fphwtw_lib_t g_faceproc_track_lib = { .p_lib_handle = 0 };
static int g_track_lib_counter = 0;

/*
 * Set the confidence to this value if the face is accepted and
 * while sending the face info to upper layers.
 * Some application may draw ROI only if confidence is above some
 * value.
 */
#define FD_MIN_CONFIDENCE 540

/**
 * Function: fphwtw_trackerr_to_imgerror
 *
 * Description: Converts faceproc tracker lib error to Imglib error
 *
 * Input parameters:
 *   lib_ret - faceproc tracker lib error code
 *
 * Return values:
 *     Corresponding Imglib error code.
 *     IMG_SUCCESS for RET_NORMAL
 *     other corresponding errors
 *
 * Notes: none
 **/
static int fphwtw_trackerr_to_imgerror(int lib_track_ret)
{
  int img_error;

  switch (lib_track_ret) {
    case RET_NORMAL :
      img_error = IMG_SUCCESS;
      break;
    case RET_ERR_INITIALIZE :
      img_error = IMG_ERR_INVALID_OPERATION;
      break;
    case RET_ERR_INVALIDPARAM :
      img_error = IMG_ERR_INVALID_INPUT;
      break;
    case RET_ERR_ALLOCMEMORY :
      img_error = IMG_ERR_NO_MEMORY;
      break;
    case FACEPROC_ERR_NOHANDLE :
      img_error = IMG_ERR_NOT_FOUND;
      break;

    case RET_ERR_VARIOUS :
    default :
      img_error = IMG_ERR_GENERAL;
      break;
  }

  return img_error;
}

/**
 * Function: fphwtw_convert_hwtdtresult_to_faceinfo
 *
 * Description: Converts hwt dt result information into
 *   faceproc faceinfo data structure
 *
 * Input parameters:
 *   p_hwt_dt_result - hw tracker dt result
 *   p_face_info - faceproc face info
 *   frame_width - frame width
 *   frame_height - frame height
 *
 * Return values:
 *     Corresponding Imglib error code.
 *     IMG_SUCCESS on success
 *     other corresponding errors
 *
 * Notes: none
 **/
static int fphwtw_convert_hwtdtresult_to_faceinfo(
  const HWTDtResult *p_hwt_dt_result, faceproc_info_t *p_face_info,
  uint32_t frame_width, uint32_t frame_height)
{
  if (!p_hwt_dt_result || !p_face_info) {
    IDBG_ERROR("Invalid input p_hwt_dt_result=%p p_face_info=%p",
      p_hwt_dt_result, p_face_info);
    return IMG_ERR_INVALID_INPUT;
  }

  INT32 half_size = (p_hwt_dt_result->nSize >> 1);

  // Clip center of face coordinates to be within the frame boundary -
  // (half_size, half_size) to (frame_width-half_size, frame_height-half_size)
  POINT corrected_center = p_hwt_dt_result->ptCenter;

  CLIP(corrected_center.x, half_size, ((int32_t)frame_width - half_size));
  CLIP(corrected_center.y, half_size, ((int32_t)frame_height - half_size));

  p_face_info->face_boundary.dx = (uint32_t)p_hwt_dt_result->nSize;
  p_face_info->face_boundary.dy = (uint32_t)p_hwt_dt_result->nSize;
  p_face_info->face_boundary.x = (uint32_t)corrected_center.x - half_size;
  p_face_info->face_boundary.y = (uint32_t)corrected_center.y - half_size;

  p_face_info->face_angle_roll = p_hwt_dt_result->nAngle;
  p_face_info->gaze_angle = p_hwt_dt_result->nPose;
  p_face_info->fd_confidence = p_hwt_dt_result->nConf;
  p_face_info->unique_id = p_hwt_dt_result->nID_HW;

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_convert_hwtresult_to_faceinfo
 *
 * Description: Converts hw tracker result information into
 *   faceproc faceinfo data structure
 *
 * Input parameters:
 *   p_hwt_result - hw tracker result
 *   p_face_info - faceproc face info
 *   frame_width - frame width
 *   frame_height - frame height
 *
 * Return values:
 *     Corresponding Imglib error code.
 *     IMG_SUCCESS on success
 *     other corresponding errors
 *
 * Notes: none
 **/
static int fphwtw_convert_hwtresult_to_faceinfo(
  const HWTResult *p_hwt_result, faceproc_info_t *p_face_info,
  uint32_t frame_width, uint32_t frame_height)
{
  if (!p_hwt_result || !p_face_info) {
    IDBG_ERROR("Invalid input p_hwt_result=%p p_face_info=%p",
      p_hwt_result, p_face_info);
    return IMG_ERR_INVALID_INPUT;
  }

  INT32 half_size = (p_hwt_result->nSize >> 1);

  // Clip center of face coordinates to be within the frame boundary -
  // (half_size, half_size) to (frame_width-half_size, frame_height-half_size)
  POINT corrected_center = p_hwt_result->ptCenter;

  CLIP(corrected_center.x, half_size, ((int32_t)frame_width - half_size));
  CLIP(corrected_center.y, half_size, ((int32_t)frame_height - half_size));

  p_face_info->face_boundary.dx = (uint32_t)p_hwt_result->nSize;
  p_face_info->face_boundary.dy = (uint32_t)p_hwt_result->nSize;
  p_face_info->face_boundary.x = (uint32_t)corrected_center.x - half_size;
  p_face_info->face_boundary.y = (uint32_t)corrected_center.y - half_size;

  p_face_info->face_angle_roll = (int32_t)p_hwt_result->nAngle;
  p_face_info->gaze_angle = (int32_t)p_hwt_result->nPose;
  p_face_info->fd_confidence = (int32_t)p_hwt_result->nConf_Tr;
  p_face_info->unique_id = (int32_t)p_hwt_result->nID_Tr;

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_convert_faceinfo_to_hwtdtresult
 *
 * Description: Converts faceproc face information into
 *   hw tracker dt data structure
 *
 * Input parameters:
 *   p_face_info - faceproc face info
 *   p_hwt_dt_result - hw tracker dt result
 *
 * Return values:
 *     Corresponding Imglib error code.
 *     IMG_SUCCESS on success
 *     other corresponding errors
 *
 * Notes: none
 **/
static int fphwtw_convert_faceinfo_to_hwtdtresult(
  const faceproc_info_t *p_face_info, HWTDtResult* p_hwt_dt_result)
{
  if (!p_face_info || !p_hwt_dt_result) {
    IDBG_ERROR("Invalid input p_face_info=%p p_hwt_dt_result=%p",
      p_face_info, p_hwt_dt_result);
    return IMG_ERR_INVALID_INPUT;
  }

  p_hwt_dt_result->nID_HW = p_face_info->unique_id;

  p_hwt_dt_result->nAngle = p_face_info->face_angle_roll;
  p_hwt_dt_result->nPose = p_face_info->gaze_angle;
  p_hwt_dt_result->nConf = p_face_info->fd_confidence;
  p_hwt_dt_result->nSize = p_face_info->face_boundary.dx;
  p_hwt_dt_result->ptCenter.x =
    p_face_info->face_boundary.x + (p_face_info->face_boundary.dx >> 1);
  p_hwt_dt_result->ptCenter.y =
    p_face_info->face_boundary.y + (p_face_info->face_boundary.dx >> 1);

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_convert_hwfacedata_to_hwtdtresult
 *
 * Description: Converts hw face data information into
 *   hw tracker dt data structure
 *
 * Input parameters:
 *   p_face_data - hw face data
 *   p_hwt_dt_result - hw tracker dt result
 *
 * Return values:
 *     Corresponding Imglib error code.
 *     IMG_SUCCESS on success
 *     other corresponding errors
 *
 * Notes: none
 **/
static int fphwtw_convert_hwfacedata_to_hwtdtresult(
  const struct msm_fd_face_data *p_face_data, HWTDtResult* p_hwt_dt_result)
{
  if (!p_face_data || !p_hwt_dt_result) {
    IDBG_ERROR("Invalid input p_face_data=%p p_hwt_dt_result=%p",
      p_face_data, p_hwt_dt_result);
    return IMG_ERR_INVALID_INPUT;
  }

  p_hwt_dt_result->nConf = p_face_data->confidence;
  p_hwt_dt_result->nAngle = p_face_data->angle;
  p_hwt_dt_result->nPose =
    faceproc_hw_comp_get_pose_angle(p_face_data->pose);

  p_hwt_dt_result->nSize = p_face_data->face.width;
  p_hwt_dt_result->ptCenter.x =
    p_face_data->face.left + (p_face_data->face.width >> 1);
  p_hwt_dt_result->ptCenter.y =
    p_face_data->face.top + (p_face_data->face.width >> 1);

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_IsSimilar_face
 *
 * Description: checks whether the face detected by SW filter
 *   is similar to what HW detected
 *
 * Input parameters:
 *   FInfo1 - FaceInfo detected by HW
 *   FInfo2 - FaceInfo detected by SW
 *
 * Return values:
 *     TRUE if same face
 *     FALSE if not
 *
 * Notes: none
 **/
static BOOL fphwtw_IsSimilar_face(
  const HWTDtResult *p_FInfo1,
  const HWTDtResult *p_FInfo2)
{
  // dummy function for now.
  return TRUE;
}

/**
 * Function: fphwtw_execute_sw_filter
 *
 * Description: Execute SW False positive filter on the given face
 *
 * Input parameters:
 *   FInfoHW - FaceInfo detected by HW which needs to verified
 *   pFInfoSW - Pointer to FaceInfo to fill face information detected  by SW
 *   p_user_data - Private user data
 *   filter_mode - false positive filtering mode. normal or hwsalvation
 *
 * Return values:
 *     TRUE if the face is detected by SW
 *     FALSE if the face is not detected by SW, means false positive
 *
 * Notes: none
 **/
BOOL fphwtw_execute_sw_filter(const HWTDtResult FInfoHW,
  HWTDtResult* pFInfoSW, void* p_user_data, const FPFilterMode filter_mode)
{
  faceproc_hw_comp_t *p_comp = (faceproc_hw_comp_t *)p_user_data;
  faceproc_hw_tracker_wrap_t *p_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;
  int rc;
  faceproc_info_t face_info;

  memset(pFInfoSW, 0x00, sizeof(HWTDtResult));

  fphwtw_convert_hwtdtresult_to_faceinfo(&FInfoHW, &face_info,
    IMG_FD_WIDTH(p_hnd->p_frame), IMG_HEIGHT(p_hnd->p_frame));

  IDBG_HIGH("Frame[%d] FaceInfo input to SWFP Filter from Tracker :"
    "nID_HW=%d : "
    "x=%d, y=%d, dx=%d, dy=%d, hw_conf=%d face_angle_roll=%d, gaze_angle=%d",
    p_hnd->p_frame->frame_id,
    face_info.unique_id, face_info.face_boundary.x, face_info.face_boundary.y,
    face_info.face_boundary.dx, face_info.face_boundary.dy,
    face_info.fd_confidence, face_info.face_angle_roll, face_info.gaze_angle);

#ifdef FACEPROC_USE_NEW_WRAPPER
  // we can call get_config and set_config to set any per face
  // based configuration. For exa, for this face use box_border = 50%

  fpsww_config_t sw_wrapper_config;

  // Get the default/current config first
  rc = faceproc_sw_wrapper_get_config(p_comp->p_sw_wrapper,
    &sw_wrapper_config);
  if (IMG_SUCCEEDED(rc)) {
    // Update any per face configuration
    if (filter_mode == FPF_MODE_HWSalvation) {
      sw_wrapper_config.filter_params.sw_face_box_border_per =
        p_comp->fd_chromatix.assist_sw_detect_hw_salvation_box_border_perc;
      sw_wrapper_config.filter_params.sw_face_spread_tol =
        p_comp->fd_chromatix.assist_sw_detect_hw_salvation_face_spread_tol;
    } else {
      sw_wrapper_config.filter_params.sw_face_box_border_per =
        p_comp->fd_chromatix.assist_sw_detect_box_border_perc;
      sw_wrapper_config.filter_params.sw_face_spread_tol =
        p_comp->fd_chromatix.assist_sw_detect_face_spread_tol;
    }

    // Set the config to wrapper so that this is applied on next face
    rc = faceproc_sw_wrapper_set_config(p_comp->p_sw_wrapper,
      &sw_wrapper_config);
    if (IMG_ERROR(rc)) {
      IMG_ERROR("sw_wrapper set config failed");
    }
  }

  rc = faceproc_sw_wrapper_process_result(p_comp->p_sw_wrapper,
    p_hnd->p_frame, &face_info, 1, 0, NULL, 0, false);
#else
  rc = facial_parts_wrap_process_result(p_comp->facial_parts_hndl,
    p_hnd->p_frame, &face_info, 1, 0, NULL, false);
#endif
  if (rc == IMG_ERR_NOT_FOUND) {
    goto false_positive;
  }

  fphwtw_convert_faceinfo_to_hwtdtresult(&face_info, pFInfoSW);

  if (fphwtw_IsSimilar_face(&FInfoHW, pFInfoSW)) {
    IDBG_HIGH("FPFILTER FACE DETECTED : nID_HW=%d : "
      "x=%d, y=%d, dx=%d, dy=%d, "
      "sw_conf=%d face_angle_roll=%d, gaze_angle=%d",
      face_info.unique_id,
      face_info.face_boundary.x, face_info.face_boundary.y,
      face_info.face_boundary.dx, face_info.face_boundary.dy,
      face_info.fd_confidence, face_info.face_angle_roll,
      face_info.gaze_angle);
  } else {
    goto false_positive;
  }

  return TRUE;

false_positive:
  IDBG_HIGH("FPFILTER FACE REJECTED");
  // If the HW result is filtered, the SW result struct will be invalidated.
  memset(pFInfoSW, 0x0, sizeof(HWTDtResult));
  return FALSE;
}

/**
 * Function: fphwtw_track_status
 *
 * Description: Select track status to output
 *
 * Input parameters:
 *   status - Tracker status
 *
 * Return values:
 *     TRUE if the face need to be considered detected
 *     FALSE if the face needs to be ignored
 *
 * Notes: none
 **/
static BOOL  fphwtw_track_status(HWT_TRACKSTATUS status)
{
  if((status == HWT_CONSISTENT) ||
    (status == HWT_LOST_HW) || (status == HWT_LOST_SW)) {
    return TRUE;
  }else{
    return FALSE;
  }
}

/**
 * Function: fphwtw_tracker_load_fptrs
 *
 * Description: Load facedetection tracker library function pointers
 *
 * Input parameters:
 *   p_lib_ptr - library handle
 *   p_tracker_funcs - pointer to save function pointers
 *
 * Return values:
 *     IMG_ERR_NOT_FOUND if failed to load functions
 *     IMG_SUCCESS if success
 *
 * Notes: none
 **/
int fphwtw_tracker_load_fptrs(void *p_lib_ptr,
  fphwtw_tracker_fptr_t *p_tracker_funcs)
{
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_CreateHandle,
    "HWTracker_CreateHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_DeleteHandle,
    "HWTracker_DeleteHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_CreateResultHandle,
    "HWTracker_CreateResultHandle");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_DeleteResultHandle,
    "HWTracker_DeleteResultHandle");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetDetectCount,
    "HWTracker_SetDetectCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetDetectResult,
    "HWTracker_SetDetectResult");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_Execute,
    "HWTracker_Execute");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetConsistentCount,
    "HWTracker_GetConsistentCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetConsistentResult,
    "HWTracker_GetConsistentResult");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetHoldCount,
    "HWTracker_SetHoldCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetHoldCount,
    "HWTracker_GetHoldCount");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetDelayCount,
    "HWTracker_SetDelayCount");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetDelayCount,
    "HWTracker_GetDelayCount");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetDeviceOrientation,
    "HWTracker_SetDeviceOrientation");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetDeviceOrientation,
    "HWTracker_GetDeviceOrientation");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs,
    HWTracker_SetAngleDiffForStrictThresholds,
    "HWTracker_SetAngleDiffForStrictThresholds");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs,
    HWTracker_GetAngleDiffForStrictThresholds,
    "HWTracker_GetAngleDiffForStrictThresholds");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetSwThresholds,
    "HWTracker_SetSwThresholds");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetSwThresholds,
    "HWTracker_GetSwThresholds");

  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_SetStrictSwThresholds,
    "HWTracker_SetStrictSwThresholds");
  IMG_DLSYM_MUST(p_lib_ptr, p_tracker_funcs, HWTracker_GetStrictSwThresholds,
    "HWTracker_GetStrictSwThresholds");

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_set_config
 *
 * Description: Set tracker config
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *   p_new_config - New config to set
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_set_config(faceproc_hw_tracker_wrap_t *p_hnd,
  fphwtw_config_t *p_new_config)
{
  if (!p_hnd->hHWTR) {
    IDBG_MED("Tracker is not created");
    return IMG_SUCCESS;
  }

  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;
  INT32 lib_track_ret;
  fphwtw_config_t *p_current_config = &p_hnd->config;

  if (p_current_config->nHoldCount != p_new_config->nHoldCount) {
    lib_track_ret = p_tracker_funcs->HWTracker_SetHoldCount(p_hnd->hHWTR,
      p_new_config->nHoldCount);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetHoldCount, lib_track_ret=%d",
        lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }
    p_current_config->nHoldCount = p_new_config->nHoldCount;
  }

  if (p_current_config->nDelayCount != p_new_config->nDelayCount) {
    lib_track_ret = p_tracker_funcs->HWTracker_SetDelayCount(p_hnd->hHWTR,
      p_new_config->nDelayCount);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetDelayCount, lib_track_ret=%d",
        lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }
    p_current_config->nDelayCount = p_new_config->nDelayCount;
  }

  if (p_current_config->nAngleDiffForStrictThreshold !=
    p_new_config->nAngleDiffForStrictThreshold) {

    lib_track_ret = p_tracker_funcs->HWTracker_SetAngleDiffForStrictThresholds(
      p_hnd->hHWTR,
      p_new_config->nAngleDiffForStrictThreshold);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetAngleDiffForStrictThresholds, "
        "lib_track_ret=%d",
        lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }

    p_current_config->nAngleDiffForStrictThreshold =
      p_new_config->nAngleDiffForStrictThreshold;
  }

  if ((p_current_config->nSwGoodFaceThreshold !=
    p_new_config->nSwGoodFaceThreshold) ||
    (p_current_config->nSwThreshold !=
    p_new_config->nSwThreshold) ||
    (p_current_config->nSwTrackingThreshold !=
    p_new_config->nSwTrackingThreshold)) {

    lib_track_ret = p_tracker_funcs->HWTracker_SetSwThresholds(
      p_hnd->hHWTR,
      p_new_config->nSwGoodFaceThreshold,
      p_new_config->nSwThreshold,
      p_new_config->nSwTrackingThreshold);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetSwThresholds, lib_track_ret=%d",
        lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }

    p_current_config->nSwGoodFaceThreshold =
      p_new_config->nSwGoodFaceThreshold;
    p_current_config->nSwThreshold =
      p_new_config->nSwThreshold;
    p_current_config->nSwTrackingThreshold =
      p_new_config->nSwTrackingThreshold;
  }

  if ((p_current_config->nStrictSwGoodFaceThreshold !=
    p_new_config->nStrictSwGoodFaceThreshold) ||
    (p_current_config->nStrictSwThreshold !=
    p_new_config->nStrictSwThreshold) ||
    (p_current_config->nStrictSwTrackingThreshold !=
    p_new_config->nStrictSwTrackingThreshold)) {

    lib_track_ret = p_tracker_funcs->HWTracker_SetStrictSwThresholds(
      p_hnd->hHWTR,
      p_new_config->nStrictSwGoodFaceThreshold,
      p_new_config->nStrictSwThreshold,
      p_new_config->nStrictSwTrackingThreshold);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetStrictSwThresholds, lib_track_ret=%d",
        lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }

    p_current_config->nStrictSwGoodFaceThreshold =
      p_new_config->nStrictSwGoodFaceThreshold;
    p_current_config->nStrictSwThreshold =
      p_new_config->nStrictSwThreshold;
    p_current_config->nStrictSwTrackingThreshold =
      p_new_config->nStrictSwTrackingThreshold;
  }

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_set_device_orientation
 *
 * Description: Set device orientation information to tracker
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *   curr_device_orientation - current device orientation
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_set_device_orientation(faceproc_hw_tracker_wrap_t *p_hnd,
  int32_t curr_device_orientation)
{
  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;
  INT32 lib_track_ret;

  lib_track_ret = p_tracker_funcs->HWTracker_SetDeviceOrientation(p_hnd->hHWTR,
    curr_device_orientation);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_SetDeviceOrientation, lib_track_ret=%d",
      lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  return IMG_SUCCESS;
}


/**
 * Function: fphwtw_get_config
 *
 * Description: Get the current tracker config
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *   p_tracker_params - pointer to save the current config
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_get_config(faceproc_hw_tracker_wrap_t *p_hnd,
  fphwtw_config_t *p_tracker_params)
{
  if (!p_hnd->hHWTR) {
    IDBG_ERROR("Invalid handle");
    return IMG_ERR_INVALID_INPUT;
  }

  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;
  INT32 lib_track_ret;

  lib_track_ret = p_tracker_funcs->HWTracker_GetHoldCount(p_hnd->hHWTR,
    &p_tracker_params->nHoldCount);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in Tracker_GetHoldCount, lib_ret=%d", lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  lib_track_ret = p_tracker_funcs->HWTracker_GetDelayCount(p_hnd->hHWTR,
    &p_tracker_params->nDelayCount);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in Tracker_GetDelayCount, lib_ret=%d", lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  lib_track_ret = p_tracker_funcs->HWTracker_GetAngleDiffForStrictThresholds(
    p_hnd->hHWTR,
    &p_tracker_params->nAngleDiffForStrictThreshold);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_GetAngleDiffForStrictThresholds, "
      "lib_ret=%d", lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  lib_track_ret = p_tracker_funcs->HWTracker_GetSwThresholds(
    p_hnd->hHWTR,
    &p_tracker_params->nSwGoodFaceThreshold,
    &p_tracker_params->nSwThreshold,
    &p_tracker_params->nSwTrackingThreshold);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_GetSwThresholds, lib_ret=%d",
      lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  lib_track_ret = p_tracker_funcs->HWTracker_GetStrictSwThresholds(
    p_hnd->hHWTR,
    &p_tracker_params->nStrictSwGoodFaceThreshold,
    &p_tracker_params->nStrictSwThreshold,
    &p_tracker_params->nStrictSwTrackingThreshold);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_GetSwThresholds, lib_ret=%d",
      lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_print_current_config
 *
 * Description: Print current tracker config
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_print_current_config(faceproc_hw_tracker_wrap_t *p_hnd)
{
  IDBG_INFO("===================== Tracker V2 Config =====================");
  IDBG_INFO("nHoldCount                   : %d",
    p_hnd->config.nHoldCount);
  IDBG_INFO("nDelayCount                  : %d",
    p_hnd->config.nDelayCount);
  IDBG_INFO("nSwGoodFaceThreshold         : %d",
    p_hnd->config.nSwGoodFaceThreshold);
  IDBG_INFO("nSwThreshold                 : %d",
    p_hnd->config.nSwThreshold);
  IDBG_INFO("nSwTrackingThreshold         : %d",
    p_hnd->config.nSwTrackingThreshold);
  IDBG_INFO("nStrictSwGoodFaceThreshold   : %d",
    p_hnd->config.nStrictSwGoodFaceThreshold);
  IDBG_INFO("nStrictSwThreshold           : %d",
    p_hnd->config.nStrictSwThreshold);
  IDBG_INFO("nStrictSwTrackingThreshold   : %d",
    p_hnd->config.nStrictSwTrackingThreshold);
  IDBG_INFO("nAngleDiffForStrictThreshold : %d",
    p_hnd->config.nAngleDiffForStrictThreshold);

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_create_handles
 *
 * Description: Create HW tracker handles
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *   max_track_count - max number of faces to track
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_create_handles(faceproc_hw_tracker_wrap_t *p_hnd,
  int32_t max_track_count)
{
  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;

  p_hnd->hHWTR = p_tracker_funcs->HWTracker_CreateHandle(max_track_count);
  if (!p_hnd->hHWTR) {
    IDBG_ERROR("Failed in creating Tracker handle : max_track_count=%d",
      max_track_count);
    return IMG_ERR_GENERAL;
  }

  p_hnd->hHWTresult = p_tracker_funcs->HWTracker_CreateResultHandle();
  if (!p_hnd->hHWTresult) {
    IDBG_ERROR("Failed in creating Tracker Results handle");
    return IMG_ERR_GENERAL;
  }

  return IMG_SUCCESS;
}

/**
 * Function: fphwtw_delete_handles
 *
 * Description: Delete HW tracker handles
 *
 * Input parameters:
 *   p_hnd - Tracker handle
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: Caller needs to make sure incoming args are non NULL
 **/
int fphwtw_delete_handles(faceproc_hw_tracker_wrap_t *p_hnd)
{
  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;
  INT32 lib_track_ret = RET_NORMAL;

  if (p_hnd->hHWTR) {
    lib_track_ret = p_tracker_funcs->HWTracker_DeleteHandle(p_hnd->hHWTR);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in deleting Tracker handle %p, lib_ret=%d",
        p_hnd->hHWTR, lib_track_ret);
    }
    p_hnd->hHWTR = NULL;
  }

  if (p_hnd->hHWTresult) {
    lib_track_ret =
      p_tracker_funcs->HWTracker_DeleteResultHandle(p_hnd->hHWTresult);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in deleting Tracker Results handle %p, lib_ret=%d",
        p_hnd->hHWTresult, lib_track_ret);
    }
    p_hnd->hHWTresult = NULL;
  }

  return fphwtw_trackerr_to_imgerror(lib_track_ret);
}

/**
 * Function: faceproc_hw_tracker_wrapper_fill_result
 *
 * Description: Execute FD tracker
 *
 * Input parameters:
 *   p_comp - HW component handle
 *   face_cnt - number faces that HW detected
 *   p_face_holder - Pointer to face info that HW detected
 *   p_frame - Current frame
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_fill_result(faceproc_hw_comp_t *p_comp,
  uint32_t face_cnt, faceproc_hw_face_holder_t *p_face_holder,
  img_frame_t *p_frame)
{
  if (!p_comp || !p_face_holder || !p_frame ||
    !p_comp->p_hw_tracker_wrapper) {
    IDBG_ERROR("Invalid input p_comp=%p p_face_holder=%p p_frame=%p",
      p_comp, p_face_holder, p_frame);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_result_t final_result;
  struct msm_fd_face_data *p_face_data;
  int i;
  uint8_t tracked_faces = 0;
  uint8_t face_tracked;
  int rc;
  HWTDtResult hwt_dt_result[HWTRACKER_FACE_MAX_NUM];
  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;
  fphwtw_tracker_fptr_t *p_tracker_funcs = &p_lib->tracker_funcs;
  INT32 lib_track_ret = RET_NORMAL;
  faceproc_hw_tracker_wrap_t *p_tracker_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;
  uint32_t max_face_cnt =
    (p_comp->config.face_cfg.max_num_face_to_detect > MAX_FACE_ROI) ?
    MAX_FACE_ROI : p_comp->config.face_cfg.max_num_face_to_detect;
  INT32 track_face_count = 0;
  HWTResult hwt_final_result;
  INT32 final_face_count = 0;
  int32_t threshold = 0;
    // (int32_t)p_comp->fd_chromatix.assist_sw_detect_threshold;

  /* Return failure in case of overflow */
  if (p_comp->batch_result_count >= FRAME_BATCH_SIZE_MAX) {
    IDBG_ERROR("Batch Result idx overflow %d ", p_comp->batch_result_count);
    return IMG_ERR_OUT_OF_BOUNDS;
  }

  p_comp->fd_frame_counter++;

  IDBG_HIGH("Frame [frame_id %d][counter %d] start : HW detected %d faces",
    p_frame->frame_id, p_comp->fd_frame_counter, face_cnt);

  // Max faces that Tracker can track
  if (face_cnt > HWTRACKER_FACE_MAX_NUM) {
    face_cnt = HWTRACKER_FACE_MAX_NUM;
  }

  if (!p_tracker_hnd->hHWTR || !p_tracker_hnd->hHWTresult) {
    IDBG_ERROR("Invalid handles hTrack=%p, hHWTresult=%p",
      p_tracker_hnd->hHWTR, p_tracker_hnd->hHWTresult);
    return IMG_ERR_INVALID_INPUT;
  }

  // Set the HW detect count to Tracker
  lib_track_ret =
    p_tracker_funcs->HWTracker_SetDetectCount(p_tracker_hnd->hHWTR, face_cnt);
  if(lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_SetDetectCount handle %p, lib_ret=%d",
      p_tracker_hnd->hHWTR, lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  // Set the HW detect face results to Tracker
  for (i = 0; i < (int)face_cnt; i++) {
    p_face_data = (struct msm_fd_face_data *)p_face_holder[i].p_face_data;

    fphwtw_convert_hwfacedata_to_hwtdtresult(p_face_data, &hwt_dt_result[i]);
    hwt_dt_result[i].nID_HW = i;

    IDBG_HIGH("FaceInfo HW Detected : Face[%d] : x=%d, y=%d, dx=%d, dy=%d, "
      "hw_conf=%d face_angle_roll=%d, gaze_angle=%d",
      i, p_face_data->face.left, p_face_data->face.top,
      p_face_data->face.width, p_face_data->face.height,
      p_face_data->confidence, p_face_data->angle, p_face_data->pose);

    lib_track_ret = p_tracker_funcs->HWTracker_SetDetectResult(
      p_tracker_hnd->hHWTR, i, &hwt_dt_result[i]);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_SetDetectResult hHWTR %p, lib_ret=%d",
        p_tracker_hnd->hHWTR, lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }
  }


  // Setup the Callback information.

  // Save the current frame in tracker_hndle, this will be used in FP callback
  p_tracker_hnd->p_frame = p_frame;

  // Callback function pointer, userdata
  FPFILTER_HNDL filter_handle;
  filter_handle.pUserData = p_comp;
  filter_handle.FP_filter = fphwtw_execute_sw_filter;

  // Execute Track
  lib_track_ret = p_tracker_funcs->HWTracker_Execute(p_tracker_hnd->hHWTR,
    p_tracker_hnd->hHWTresult, p_comp->fd_frame_counter, 0,
    IMG_FD_WIDTH(p_frame), IMG_HEIGHT(p_frame), &filter_handle);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in HWTracker_Execute handle %p, lib_ret=%d",
      p_tracker_hnd->hHWTR, lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  // Get the results from Tracker
  lib_track_ret = p_tracker_funcs->HWTracker_GetConsistentCount(
    p_tracker_hnd->hHWTresult, &track_face_count);
  if (lib_track_ret != RET_NORMAL) {
    IDBG_ERROR("Failed in GetConsistentCount hHWTresult %p, lib_ret=%d",
      p_tracker_hnd->hHWTresult, lib_track_ret);
    return fphwtw_trackerr_to_imgerror(lib_track_ret);
  }

  IDBG_HIGH("Tracker Consistent face Count %d", track_face_count);

  memset(&final_result, 0x0, sizeof(faceproc_result_t));

  for (i = 0;
    ((i < track_face_count) && (final_face_count < (int32_t)max_face_cnt));
    i++) {

    lib_track_ret = p_tracker_funcs->HWTracker_GetConsistentResult(
      p_tracker_hnd->hHWTresult, i, &hwt_final_result);
    if (lib_track_ret != RET_NORMAL) {
      IDBG_ERROR("Failed in HWTracker_GetConsistentResult i=%d, lib_ret=%d",
        i, lib_track_ret);
      return fphwtw_trackerr_to_imgerror(lib_track_ret);
    }

    IDBG_HIGH("FaceInfo from Tracker [%d] nID_HW=%d, nID_Tr=%d "
      "nConf_HW=%d, nConf_SW=%d, nConf_Tr=%d, STATUS=%d, nHold=%d, "
      "nDelay=%d, nHistory=%d, gaze_angle=%d, face_angle_roll=%d, "
      "nSize=%d, Center=(%d, %d)",
      i,
      hwt_final_result.nID_HW, hwt_final_result.nID_Tr,
      hwt_final_result.nConf_HW, hwt_final_result.nConf_SW,
      hwt_final_result.nConf_Tr, hwt_final_result.STATUS,
      hwt_final_result.nHold, hwt_final_result.nDelay,
      hwt_final_result.nHistory, hwt_final_result.nPose,
      hwt_final_result.nAngle, hwt_final_result.nSize,
      hwt_final_result.ptCenter.x, hwt_final_result.ptCenter.y);

    if ((fphwtw_track_status(hwt_final_result.STATUS)) &&
      (hwt_final_result.nConf_Tr >= threshold)) {

      fphwtw_convert_hwtresult_to_faceinfo(&hwt_final_result,
        &final_result.roi[final_face_count],
        IMG_FD_WIDTH(p_frame), IMG_HEIGHT(p_frame));

      // Accept the face if the state is good, even if final tracking
      // confidence is low. But hardcode the confidence to a higher
      // value so that the upper layers wont reject this face info.
      // Even if we are getting tracking confidence less than
      // SET_TO_MIN_CONFIDENCE, it doesn't
      // mean SW confidence is low. It can happen in cases where
      // HW_LOST/SW_LOST. Since tracking confidence is combination of
      // HW, SW confidence, this can happen.
      if (final_result.roi[final_face_count].fd_confidence <=
        FD_MIN_CONFIDENCE) {
        final_result.roi[final_face_count].fd_confidence = FD_MIN_CONFIDENCE;
      }

      if (hwt_final_result.nHistory <=
        (p_tracker_hnd->config.nDelayCount + 1)) {
        face_tracked = 0;
        IDBG_HIGH("New Face Detected");
      } else {
        face_tracked = 1;
        tracked_faces++;
        IDBG_HIGH("Old Face.. tracking..");
      }

      // Run facial parts
#ifdef FACEPROC_USE_NEW_WRAPPER
      rc = faceproc_sw_wrapper_process_result(p_comp->p_sw_wrapper,
        p_frame, &final_result.roi[final_face_count], 0, face_tracked,
        NULL, final_face_count, true);
#else
      rc = facial_parts_wrap_process_result(p_comp->facial_parts_hndl,
        p_frame, &final_result.roi[final_face_count], 0, 0, NULL, true);
#endif
      if (IMG_ERROR(rc)) {
        IDBG_ERROR("Error in Facial parts");
      }

      IDBG_HIGH("FinalFaceInfo output : Face[%d] : unique_id(tracker id)=%d, "
        "x=%d, y=%d, dx=%d, dy=%d, confidence(tracker conf)=%d "
        "face_angle_roll=%d, gaze_angle=%d",
        final_face_count,
        final_result.roi[final_face_count].unique_id,
        final_result.roi[final_face_count].face_boundary.x,
        final_result.roi[final_face_count].face_boundary.y,
        final_result.roi[final_face_count].face_boundary.dx,
        final_result.roi[final_face_count].face_boundary.dy,
        final_result.roi[final_face_count].fd_confidence,
        final_result.roi[final_face_count].face_angle_roll,
        final_result.roi[final_face_count].gaze_angle);

      final_face_count++;
    }
  }

  final_result.frame_id = p_frame->frame_id;
  final_result.num_faces_detected = final_face_count;

  pthread_mutex_lock(&p_comp->result_mutex);
  if (p_comp->frame_batch_info.batch_mode) {
    p_comp->intermediate_result[p_comp->tracking_info.cur_tbl_idx] =
      final_result;
    p_comp->batch_result_count++;
  } else {
    p_comp->last_result = final_result;
  }
  pthread_mutex_unlock(&p_comp->result_mutex);

  IDBG_HIGH("Frame [frame_id %d][counter %d] Done : Final faces = %d",
    p_frame->frame_id, p_comp->fd_frame_counter, final_face_count);

  faceproc_hw_comp_frame_dump(p_frame, p_comp->dump_mode,
    tracked_faces, final_face_count);

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_hw_tracker_wrapper_get_config
 *
 * Description: Get tracker config
 *
 * Input parameters:
 *   p_comp - HW component handle
 *   p_config - config to get
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_get_config(faceproc_hw_comp_t *p_comp,
  fphwtw_config_t *p_config)
{
  int ret = IMG_SUCCESS;

  if (!p_comp || !p_config) {
    IDBG_HIGH("Invalid input param p_comp=%p, p_config=%p", p_comp, p_config);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_hw_tracker_wrap_t *p_tracker_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;

  *p_config = p_tracker_hnd->config;

  return ret;
}

/**
 * Function: faceproc_hw_tracker_wrapper_set_config
 *
 * Description: Set tracker config
 *
 * Input parameters:
 *   p_comp - HW component handle
 *   p_config - config to set
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_set_config(faceproc_hw_comp_t *p_comp,
  fphwtw_config_t *p_config)
{
  int ret = IMG_SUCCESS;

  if (!p_comp || !p_config) {
    IDBG_HIGH("Invalid input param p_comp=%p, p_config=%p", p_comp, p_config);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_hw_tracker_wrap_t *p_tracker_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;

  IDBG_HIGH("Set Config. p_tracker_hnd=%p, hHWTR=%p, hHWTresult=%p",
    p_tracker_hnd, p_tracker_hnd->hHWTR, p_tracker_hnd->hHWTresult);

  ret = fphwtw_set_config(p_tracker_hnd, p_config);
  if (IMG_ERROR(ret)) {
    IDBG_ERROR("Failed in setting new tracker config, ret=%d", ret);
    return ret;
  }

  if (IMG_LOG_LEVEL_HIGH) {
    IDBG_INFO("Set Tracker Config");
    fphwtw_print_current_config(p_tracker_hnd);
  }

  return ret;
}

/**
 * Function: faceproc_hw_tracker_wrapper_create
 *
 * Description: Create tracker handles
 *
 * Input parameters:
 *   p_comp - HW component handle
 *   max_track_count - maximum number of faces to track
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_create(faceproc_hw_comp_t *p_comp,
  int32_t max_track_count)
{
  int ret = IMG_SUCCESS;
  faceproc_hw_tracker_wrap_t *p_hnd;

  IDBG_HIGH("Create Wrapper. p_comp=%p, max_track_count=%d, "
    "p_lib_handle=%p, p_hw_tracker_wrapper=%p",
    p_comp, max_track_count, g_faceproc_track_lib.p_lib_handle,
    p_comp->p_hw_tracker_wrapper);

  // Check if the library is loaded or not.
  // Caller must call faceproc_hw_tracker_wrapper_load before calling this.
  if ((!g_faceproc_track_lib.p_lib_handle) || (g_track_lib_counter <= 0)) {
    IDBG_ERROR("HW tracker lib not loaded yet : lib handle %p, counter %d",
      g_faceproc_track_lib.p_lib_handle, g_track_lib_counter);
    return IMG_ERR_INVALID_INPUT;
  }

  if (p_comp->p_hw_tracker_wrapper) {
    IDBG_ERROR("HW tracker is already created, something wrong");
    return IMG_ERR_INVALID_INPUT;
  }

  p_hnd = calloc(1, sizeof(*p_hnd));
  if (p_hnd == NULL) {
    IDBG_ERROR("Fail to allocate handler memory");
    return IMG_ERR_NO_MEMORY;
  }

  ret = fphwtw_create_handles(p_hnd, max_track_count);
  if (IMG_ERROR(ret)) {
    IDBG_ERROR("Failed in creating tracker handles, ret=%d", ret);
    goto error_create_handle;
  }

  ret = fphwtw_get_config(p_hnd, &p_hnd->config);
  if (ret != IMG_SUCCESS) {
    IDBG_ERROR("Failed in getting default config");
    goto error_config;
  }

  if (IMG_LOG_LEVEL_HIGH) {
    IDBG_INFO("Default tracker config");
    fphwtw_print_current_config(p_hnd);
  }

  p_hnd->curr_device_orientation = -1;

  p_comp->p_hw_tracker_wrapper = (void*)p_hnd;

  IDBG_HIGH("Created Wrapper. p_hw_tracker_wrapper=%p, "
    "hHWTR=%p, hHWTresult=%p",
    p_comp->p_hw_tracker_wrapper, p_hnd->hHWTR, p_hnd->hHWTresult);

  return ret;

error_config:
  fphwtw_delete_handles(p_hnd);

error_create_handle:
  free(p_hnd);
  return ret;
}

/**
 * Function: faceproc_hw_tracker_wrapper_destroy
 *
 * Description: Destroy tracker handles
 *
 * Input parameters:
 *   p_comp - HW component handle
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_destroy(faceproc_hw_comp_t *p_comp)
{
  int ret = IMG_SUCCESS;

  if (!p_comp || !p_comp->p_hw_tracker_wrapper) {
    IDBG_HIGH("HW tracker not created p_comp=%p", p_comp);
    return IMG_SUCCESS;
  }

  faceproc_hw_tracker_wrap_t *p_tracker_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;

  IDBG_HIGH("Destroying Wrapper. p_tracker_hnd=%p, "
    "hHWTR=%p, hHWTresult=%p",
    p_tracker_hnd, p_tracker_hnd->hHWTR, p_tracker_hnd->hHWTresult);

  ret = fphwtw_delete_handles(p_tracker_hnd);
  if (IMG_ERROR(ret)) {
    IDBG_ERROR("Failed in Deleting tracker handles, ret=%d", ret);
    // fall through
  }

  free(p_comp->p_hw_tracker_wrapper);

  p_comp->p_hw_tracker_wrapper = NULL;

  return ret;
}

/**
 * Function: faceproc_hw_tracker_wrapper_load_library
 *
 * Description: Load tracker library
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_load_library()
{
  int rc = IMG_SUCCESS;

  IDBG_HIGH("Loading library. p_lib_handle=%p",
    g_faceproc_track_lib.p_lib_handle);

  if (g_faceproc_track_lib.p_lib_handle) {
    IDBG_HIGH("Tracker library already loaded");
    g_track_lib_counter++;
    return IMG_SUCCESS;
  }

  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;

  p_lib->p_lib_handle = dlopen("libmmcamera_facedetection_lib.so", RTLD_NOW);
  if (!p_lib->p_lib_handle) {
    IDBG_HIGH("Error opening libmmcamera_facedetection_lib.so lib");
    return IMG_ERR_NOT_FOUND;
  }

  rc = fphwtw_tracker_load_fptrs(p_lib->p_lib_handle, &p_lib->tracker_funcs);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in loading Tracker function pointers");
    goto error;
  }

  g_track_lib_counter++;

  IDBG_HIGH("Loaded library. p_lib_handle=%p",
    g_faceproc_track_lib.p_lib_handle);

  return rc;

error:
  dlclose(p_lib->p_lib_handle);
  return rc;
}

/**
 * Function: faceproc_hw_tracker_wrapper_unload_library
 *
 * Description: Unload tracker library
 *
 * Input parameters:
 *   None
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_unload_library()
{
  fphwtw_lib_t *p_lib = &g_faceproc_track_lib;

  IDBG_HIGH("UnLoading library. p_lib_handle=%p",
    g_faceproc_track_lib.p_lib_handle);

  g_track_lib_counter--;

  if (g_track_lib_counter == 0) {
    dlclose(p_lib->p_lib_handle);
    memset(&g_faceproc_track_lib, 0x0, sizeof(fphwtw_lib_t));

    IDBG_HIGH("UnLoaded library. p_lib_handle=%p",
      g_faceproc_track_lib.p_lib_handle);
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_hw_tracker_wrapper_set_orientation
 *
 * Description: Set device orientation
 *
 * Input parameters:
 *   p_comp - HW component handle
 *   curr_device_orientation - device orientation
 *
 * Return values:
 *     IMG_SUCCESS if success
 *     IMG_xxx on failure
 *
 * Notes: None
 **/
int faceproc_hw_tracker_wrapper_set_orientation(faceproc_hw_comp_t *p_comp,
  int32_t curr_device_orientation)
{
  int ret = IMG_SUCCESS;

  if (!p_comp) {
    IDBG_HIGH("Invalid input param p_comp=%p", p_comp);
    return IMG_ERR_INVALID_INPUT;
  }

  faceproc_hw_tracker_wrap_t *p_tracker_hnd =
    (faceproc_hw_tracker_wrap_t *)p_comp->p_hw_tracker_wrapper;

  p_tracker_hnd->curr_device_orientation = curr_device_orientation;

  fphwtw_set_device_orientation(p_tracker_hnd, curr_device_orientation);

  return ret;
}


