/**********************************************************************
*  Copyright (c) 2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "faceproc_comp.h"
#include "faceproc_sw_wrapper.h"

/**
 * Function: faceproc_fd_output
 *
 * Description: Gets the frameproc output
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_fd_data - Faceproc result data
 *   p_num_faces - Pointer to number of faces
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_fd_output(faceproc_comp_t *p_comp,
  faceproc_result_t *p_fd_data,
  int32_t *p_num_faces)
{
  int rc;

  rc = faceproc_sw_wrapper_get_fd_info(p_comp->p_wrapper,
    p_fd_data->roi, MAX_FACE_ROI, p_num_faces);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Failed in getting fd info");
    return rc;
  }

  /* Parse and store the faces */
  p_fd_data->num_faces_detected = (uint32_t)*p_num_faces;
  p_fd_data->frame_id = p_comp->frame_id;

  if (p_fd_data->num_faces_detected > MAX_FACE_ROI)
    p_fd_data->num_faces_detected = MAX_FACE_ROI;

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_frame_dump
 *
 * Description: Dump frames based on dump configuration
 *
 * Arguments:
 *   @p_comp: Pointer to faceproc component struct
 *   @p_frame: Pointer to frame which need to be dumped.
 *   @num_faces: Number of detected faces.
 *
 * Return values:
 *   None
 **/
static void faceproc_frame_dump(faceproc_comp_t *p_comp, img_frame_t *p_frame,
  unsigned int num_faces)
{
  unsigned int i;
  unsigned int tracked;

  if (!p_comp || !p_frame) {
    IDBG_ERROR("Invalid input p_comp=%p, p_frame=%p", p_comp, p_frame);
    return;
  }

  if (p_comp->dump_mode != FACE_FRAME_DUMP_OFF) {
    tracked = 0;
    for (i = 0; i < num_faces; i++) {
      tracked += faceproc_sw_wrapper_is_face_tracked(p_comp->p_wrapper, i);
    }

    switch (p_comp->dump_mode) {
    case FACE_FRAME_DUMP_NON_TRACKED:
      if (num_faces && !tracked) {
        img_dump_frame(p_frame, FACE_DEBUG_PATH, num_faces, NULL);
      }
      break;
    case FACE_FRAME_DUMP_TRACKED:
      if (tracked) {
        img_dump_frame(p_frame, FACE_DEBUG_PATH, tracked, NULL);
      }
      break;
    case FACE_FRAME_DUMP_NOT_DETECTED:
      if (num_faces == 0) {
        img_dump_frame(p_frame, FACE_DEBUG_PATH, 0, NULL);
      }
      break;
    case FACE_FRAME_DUMP_ALL:
      img_dump_frame(p_frame, FACE_DEBUG_PATH, num_faces, NULL);
      break;
    default:
      return;
    }
  }
}

/**
 * Function: faceproc_fd_execute
 *
 * Description: Executes the face detecttion algorithm
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_frame - pointer to input frame
 *   p_num_faces - number of faces
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
static int faceproc_fd_execute(faceproc_comp_t *p_comp, img_frame_t *p_frame,
  int32_t *p_num_faces)
{
  int rc;
  IDBG_HIGH("E %dx%d", FD_WIDTH(p_frame), FD_HEIGHT(p_frame));

  p_comp->frame_id = p_frame->frame_id;

  ATRACE_BEGIN_SNPRINTF(32, "FD_%d", p_frame->frame_id);

  rc = faceproc_sw_wrapper_execute_fd(p_comp->p_wrapper, p_frame, p_num_faces);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("sw_wrapper execute fd failed %d", rc);
    return rc;
  }

  ATRACE_END();

  faceproc_frame_dump(p_comp, p_frame, *p_num_faces);

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_get_angle
 *
 * Description: Get the faceproc angle
 *
 * Input parameters:
 *   angle: face detection angle macro
 *
 * Return values:
 *     bitmask for engine angle
 *
 * Notes: none
 **/
uint32_t faceproc_comp_eng_get_angle(fd_chromatix_angle_t angle)
{
  switch (angle) {
    case FD_ANGLE_ALL:
      return ROLL_ANGLE_ALL;
    case FD_ANGLE_15_ALL:
      return (ROLL_ANGLE_0 | ROLL_ANGLE_3 | ROLL_ANGLE_6 | ROLL_ANGLE_9);
    case FD_ANGLE_45_ALL:
      return (ROLL_ANGLE_0 | ROLL_ANGLE_1 | ROLL_ANGLE_2 |
        ROLL_ANGLE_3 | ROLL_ANGLE_4 | ROLL_ANGLE_8 |
        ROLL_ANGLE_9 | ROLL_ANGLE_10 | ROLL_ANGLE_11);
    case FD_ANGLE_MANUAL:
      /* todo */
      return ROLL_ANGLE_NONE;
    case FD_ANGLE_NONE:
    default:
      return ROLL_ANGLE_NONE;
  }
  return ROLL_ANGLE_NONE;
}

/**
 * Function: faceproc_comp_eng_face_size
 *
 * Description: Get the face size
 *
 * Input parameters:
 *   face_adj_type: type of face dimension calculation
 *   face_size: face size for fixed adjustment
 *   ratio: facesize ratio for floating adjustment
 *   min_size: minimum face size supported by library
 *   dimension: min(height/width) of the image
 *
 * Return values:
 *     face size based on input parameters
 *
 * Notes: none
 **/
uint32_t faceproc_comp_eng_face_size(fd_face_adj_type_t face_adj_type,
  uint32_t face_size,
  float ratio,
  uint32_t min_size,
  uint32_t dimension)
{
  switch (face_adj_type) {
    case FD_FACE_ADJ_FLOATING: {
      uint32_t size = (uint32_t)((float)dimension * ratio);;
      size = (size/10) * 10;
      return(size < min_size) ? min_size : size;
    }
    case FD_FACE_ADJ_FIXED:
    default:
      return face_size;
  }
  return face_size;
}

/**
 * Function: faceproc_comp_eng_update_cfg
 *
 * Description: Update the faceproc config when
 *   there is a dynamic update
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *
 * Return values:
 *     IMG_SUCCESS on success
 *     IMG_error otherwise
 *
 * Notes: none
 **/
int faceproc_comp_eng_update_cfg(faceproc_comp_t *p_comp)
{
  int rc;
  uint32_t min_face_size = faceproc_comp_eng_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    FACEPROC_MIN_FACE_SIZE, /* restricting min facesize */
    MIN(p_comp->width, p_comp->height));

  IDBG_MED("new ###min_face_size %d", min_face_size);

  // Get the default/current config first
  fpsww_config_t sw_config;
  rc = faceproc_sw_wrapper_get_config(p_comp->p_wrapper, &sw_config);
  if (IMG_ERROR(rc)) {
    IMG_ERROR("sw_wrapper get config failed");
    return rc;
  }

  // Update the config params as required/tuned

  // Update min, max sizes
  sw_config.dt_params.nMinSize = (int32_t)min_face_size;
  sw_config.dt_params.nMaxSize = (int32_t)p_comp->fd_chromatix.max_face_size;

  // Update facial parts features
  boolean enable_face_landmarks;
  boolean enable_contour, enable_smile, enable_blink, enable_gaze;

  faceproc_get_feature_flags(p_comp, &enable_face_landmarks, &enable_contour,
    &enable_smile, &enable_blink, &enable_gaze);

  // features
  sw_config.enable_facial_parts = enable_face_landmarks;
  sw_config.enable_contour = enable_contour;
  sw_config.enable_smile = enable_smile;
  sw_config.enable_gaze  = enable_gaze;
  sw_config.enable_blink = enable_blink;

  rc = faceproc_sw_wrapper_set_config(p_comp->p_wrapper, &sw_config);
  if (IMG_ERROR(rc)) {
    IMG_ERROR("sw_wrapper set config failed");
    return rc;
  }

  return rc;
}

/**
 * Function: faceproc_comp_eng_reconfig_core
 *
 * Description: Re-Configure the faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   reset_results - Whether to reset the current detection results
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_reconfig_core(faceproc_comp_t *p_comp,
  bool reset_results)
{
  IDBG_LOW("Enter: %p", (void *)p_comp);

  uint32_t an_still_angle[POSE_TYPE_COUNT];
  int rc = IMG_SUCCESS;
  faceproc_config_t *p_cfg = &p_comp->config;
  uint32_t min_face_size;

  // Get the default/current config first
  fpsww_config_t sw_config;
  rc = faceproc_sw_wrapper_get_config(p_comp->p_wrapper, &sw_config);
  if (IMG_ERROR(rc)) {
    IMG_ERROR("sw_wrapper get config failed");
    return rc;
  }

  /* Get the tuning params from chromatix */

  // minimum face size
  min_face_size = faceproc_comp_eng_face_size(
    p_comp->fd_chromatix.min_face_adj_type,
    p_comp->fd_chromatix.min_face_size,
    p_comp->fd_chromatix.min_face_size_ratio,
    FACEPROC_MIN_FACE_SIZE, /* restricting min facesize */
    MIN(p_cfg->frame_cfg.max_width, p_cfg->frame_cfg.max_height));

  IDBG_MED("###min_face_size %d", min_face_size);

  /* Minimum face size to be detected should be at most half the
    height of the input frame */
  if (min_face_size > (p_cfg->frame_cfg.max_height / 2)) {
    IDBG_WARN("Error, min face size to detect is greater than "
      "half the height of the input frame");
  }

  // POSE angles
  if (FD_ANGLE_ENABLE(p_comp)) {
    an_still_angle[FPSWW_ANGLE_FRONT] =
      faceproc_comp_eng_get_angle(p_comp->fd_chromatix.angle_front);
    an_still_angle[FPSWW_ANGLE_HALFPROFILE] =
      faceproc_comp_eng_get_angle(p_comp->fd_chromatix.angle_half_profile);
    an_still_angle[FPSWW_ANGLE_FULLPROFILE] =
      faceproc_comp_eng_get_angle(p_comp->fd_chromatix.angle_full_profile);
  } else {
    IDBG_MED("###Disable Angle");
    an_still_angle[FPSWW_ANGLE_FRONT] = ROLL_ANGLE_NONE;
    an_still_angle[FPSWW_ANGLE_HALFPROFILE] = ROLL_ANGLE_NONE;
    an_still_angle[FPSWW_ANGLE_FULLPROFILE] = ROLL_ANGLE_NONE;
  }

  int32_t  search_density;
  // Map the existing density range to new density enums
  // existing range (used in chromatix) : 20 to 40
  // New density params :
  //   DENSITY_HIGHEST, DENSITY_HIGH, DENSITY_NORMAL,
  //   DENSITY_LOW, DENSITY_LOWEST
  // density property is reverse between v5 vs v6
  if (p_comp->fd_chromatix.search_density_nontracking <= 25) {
    search_density = DENSITY_HIGHEST;
  } else if (p_comp->fd_chromatix.search_density_nontracking <= 30) {
    search_density = DENSITY_HIGH;
  } else if (p_comp->fd_chromatix.search_density_nontracking <= 34) {
    search_density = DENSITY_NORMAL;
  } else if (p_comp->fd_chromatix.search_density_nontracking <= 37) {
    search_density = DENSITY_LOW;
  } else {
    search_density = DENSITY_LOWEST;
  }

  // Update the config params as required/tuned

  boolean enable_face_landmarks;
  boolean enable_contour, enable_smile, enable_blink, enable_gaze;
  faceproc_get_feature_flags(p_comp, &enable_face_landmarks, &enable_contour,
    &enable_smile, &enable_blink, &enable_gaze);

  // features
  sw_config.enable_facial_parts = enable_face_landmarks;
  sw_config.enable_contour = enable_contour;
  sw_config.enable_smile = enable_smile;
  sw_config.enable_gaze  = enable_gaze;
  sw_config.enable_blink = enable_blink;

  /************* DT Params *************/
  sw_config.dt_params.nMinSize = min_face_size;
  sw_config.dt_params.nMaxSize = (int32_t)p_comp->fd_chromatix.max_face_size;

  sw_config.dt_params.nAngle[FPSWW_ANGLE_FRONT] =
    an_still_angle[FPSWW_ANGLE_FRONT];
  sw_config.dt_params.nAngle[FPSWW_ANGLE_HALFPROFILE] =
    an_still_angle[FPSWW_ANGLE_HALFPROFILE];
  sw_config.dt_params.nAngle[FPSWW_ANGLE_FULLPROFILE] =
    an_still_angle[FPSWW_ANGLE_FULLPROFILE];

  sw_config.dt_params.nThreshold = (int32_t)p_comp->fd_chromatix.threshold;

  sw_config.dt_params.nSearchDensity = search_density;
  sw_config.dt_params.bMask = (int32_t)p_comp->fd_chromatix.direction;

  // These are what we think as better configuration
  sw_config.dt_params.nDelayCount = 0;
  sw_config.dt_params.nMaxHoldCount = 1;
  sw_config.dt_params.nAccuracy = TRACKING_ACCURACY_HIGH;
  sw_config.dt_params.nInitialFaceSearchCycle = 15;
  sw_config.dt_params.nNewFaceSearchCycle = 15;
  sw_config.dt_params.nNewFaceSearchInterval = 0;
  sw_config.dt_params.nPosSteadinessParam = 0;
  sw_config.dt_params.nSizeSteadinessParam = 0;

  // Modify config as per the chromatix request
  if (p_comp->fd_chromatix.delay_count) {
    sw_config.dt_params.nDelayCount = p_comp->fd_chromatix.delay_count;
  }
  if (p_comp->fd_chromatix.no_face_search_cycle) {
    sw_config.dt_params.nInitialFaceSearchCycle =
      p_comp->fd_chromatix.no_face_search_cycle;
  }
  if (p_comp->fd_chromatix.new_face_search_cycle) {
    sw_config.dt_params.nNewFaceSearchCycle =
      p_comp->fd_chromatix.new_face_search_cycle;
  }
  sw_config.dt_params.nNewFaceSearchInterval =
    p_comp->fd_chromatix.refresh_count;

  /************* PT Params *************/
  sw_config.pt_params.nMode = PT_MODE_DEFAULT;
  sw_config.pt_params.nConfMode = PT_CONF_USE;

  /************* CT Params *************/
  sw_config.ct_params.nMode = p_comp->fd_chromatix.ct_detection_mode;

  /************* Wrapper filter params *************/
  sw_config.filter_params.lock_faces = p_comp->fd_chromatix.lock_faces;
  sw_config.filter_params.discard_facialparts_threshold =
    p_comp->fd_chromatix.facial_parts_threshold;

  rc = faceproc_sw_wrapper_set_config(p_comp->p_wrapper, &sw_config);
  if (IMG_ERROR(rc)) {
    IMG_ERROR("sw_wrapper set config failed");
    return rc;
  }

  if (reset_results == TRUE) {
    rc = faceproc_sw_wrapper_reset_result(p_comp->p_wrapper);
    if (IMG_ERROR(rc)) {
      IMG_ERROR("sw_wrapper reset results failed");
      return rc;
    }
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_config
 *
 * Description: Configure the faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_config(faceproc_comp_t *p_comp)
{
  IDBG_LOW("Enter: %p", (void *)p_comp);

  if (!p_comp) {
    IDBG_ERROR("NULL component");
    return IMG_ERR_GENERAL;
  }

  int rc = IMG_SUCCESS;
  uint32_t max_num_face_to_detect =
    faceproc_get_max_number_to_detect(&p_comp->fd_chromatix);

  // Create sw_wrapper handle
  fpsww_create_params_t create_params;
  create_params.engine = FACEPROC_ENGINE_SW;
  create_params.create_face_parts = TRUE;
  create_params.create_face_recog = FALSE;
  create_params.max_face_count = max_num_face_to_detect;
  create_params.detection_mode =
    (p_comp->fd_chromatix.detection_mode == FD_CHROMATIX_MODE_MOTION_FULL) ?
    DETECTION_MODE_STILL : DETECTION_MODE_MOVIE;
  create_params.no_of_fp_handles = MAX_FACE_ROI;

  p_comp->p_wrapper = faceproc_sw_wrapper_create(&create_params);
  if (!p_comp->p_wrapper) {
    IMG_ERROR("sw_wrapper creation failed");
    return IMG_ERR_GENERAL;
  }

  faceproc_sw_wrapper_set_debug(p_comp->p_wrapper, &p_comp->debug_settings);

  rc = faceproc_comp_eng_reconfig_core(p_comp, FALSE);
  if (rc != IMG_SUCCESS) {
    IMG_ERROR("failed in configuring sw_wrapper");
  }

  return IMG_SUCCESS;
}

/**
 * Function: faceproc_comp_eng_exec
 *
 * Description: main algorithm execution function for face processing
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_frame - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_exec(faceproc_comp_t *p_comp,
  img_frame_t *p_frame)
{
  int32_t num_faces;
  int rc = IMG_SUCCESS;

  switch (p_comp->mode) {
    case FACE_DETECT_BSGC:
    case FACE_DETECT:
    case FACE_DETECT_LITE:
      if (p_comp->is_chromatix_changed == TRUE) {
        p_comp->is_chromatix_changed = FALSE;
        rc = faceproc_comp_eng_reconfig_core(p_comp, TRUE);
        if (IMG_ERROR(rc)) {
          IDBG_ERROR("Failed in faceproc_comp_eng_reconfig_core, rc = %d", rc);
        }
      }

      if (IMG_SUCCEEDED(rc)) {
        rc = faceproc_fd_execute(p_comp, p_frame, &num_faces);
        if (IMG_ERROR(rc))
          IDBG_ERROR("Failed in faceproc_fd_execute, rc = %d", rc);
      }
      break;
    case FACE_RECOGNIZE:
    case FACE_REGISTER:
    default :
      IDBG_ERROR("MODE not selected/recognized");
      rc = IMG_ERR_NOT_SUPPORTED;
  }

  return rc;
}

/**
 * Function: faceproc_comp_eng_get_output
 *
 * Description: Get the output from the frameproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *   p_fd_data - Input frame
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_get_output(faceproc_comp_t *p_comp,
  faceproc_result_t *p_fd_data)
{
  int32_t num_faces;
  int status;

  IDBG_LOW("p_comp:%p, p_res:%p", p_comp, p_fd_data);

  switch (p_comp->mode) {
    case FACE_DETECT_BSGC:
    case FACE_DETECT:
    case FACE_DETECT_LITE:
      status = faceproc_fd_output(p_comp, p_fd_data, &num_faces);
      if (IMG_ERROR(status)) {
        IDBG_ERROR("Failed in faceproc_fd_output, rc = %d", status);
      }
      break;

    case FACE_RECOGNIZE:
    case FACE_REGISTER:
    default:
      IDBG_ERROR("Unsupported mode selected");
      status = IMG_ERR_INVALID_INPUT;
  }

  IDBG_LOW("After rc: %d, p_comp:%p, p_res:%p",
    status, p_comp, p_fd_data);

  return status;
}

/**
 * Function: faceproc_comp_eng_destroy
 *
 * Description: Destroy the faceproc engine
 *
 * Input parameters:
 *   p_comp - The pointer to the faceproc engine object
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int faceproc_comp_eng_destroy(faceproc_comp_t *p_comp)
{
  int rc;

  if (!p_comp)
    return IMG_ERR_GENERAL;

  IDBG_LOW("faceproc engine clean p_comp=%p, p_comp->p_wrapper=%p",
    p_comp, p_comp->p_wrapper);

  if (p_comp->p_wrapper) {
    rc = faceproc_sw_wrapper_destroy(p_comp->p_wrapper);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("sw_wrapper destroy failed %d", rc);
    }
    p_comp->p_wrapper = NULL;
  }

  return IMG_SUCCESS;
}

