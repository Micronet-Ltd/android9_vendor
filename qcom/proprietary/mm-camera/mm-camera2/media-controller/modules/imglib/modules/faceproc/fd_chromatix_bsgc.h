/**********************************************************************
*  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
.chromatix_id = FD_CHROMATIX_BSGC,
.enable = 1,
.min_face_adj_type = FD_FACE_ADJ_FLOATING,
.min_face_size = 90,
.min_face_size_ratio = 0.09f,
.max_face_size = 1000,
.max_num_face_to_detect = 5,
.angle_front = FD_ANGLE_ALL,
.angle_half_profile = FD_ANGLE_15_ALL,
.angle_full_profile = FD_ANGLE_15_ALL,
.detection_mode = FD_CHROMATIX_MODE_MOTION_PROGRESS,
.fd_max_fps = 15,
.enable_smile_detection = 1,
.enable_blink_detection = 1,
.enable_gaze_detection = 1,
.search_density_nontracking = 33,
.search_density_tracking = 33,
.direction = 0,
.threshold = 630,
.facial_parts_threshold = 0,
.closed_eye_ratio_threshold = 600,
.face_retry_count = 3,
.head_retry_count = 3,
.lock_faces = 1,
.move_rate_threshold = 8,
.initial_frame_no_skip_cnt = 15,
.ct_detection_mode = FD_CONTOUR_MODE_DEFAULT,
.delay_count = 0,
.hold_count = 1,
.no_face_search_cycle = 15,
.new_face_search_cycle = 15,
.refresh_count = 0,
/* Stabilization parameters */
.stab_enable = 1,
.stab_history = 3,
/* Confidence tuning params, used to filter false positives */
.stab_conf = {
  .enable = 1,
  .filter_type = FD_STAB_HYSTERESIS,
  .hyst = {
    .start_A = 520,
    .end_A = 560,
  },
},
/* Position stabilization tuning params */
.stab_pos = {
  .enable = 1,
  .mode = FD_STAB_EQUAL,
  .state_cnt = 0,
  .threshold = 15,
  .use_reference = 1,
  .filter_type = FD_STAB_NO_FILTER,
},
/* Size stabilization tuning params */
.stab_size = {
  .enable = 1,
  .mode = FD_STAB_CONTINUES_SMALLER,
  .state_cnt = 3,
  .threshold = 500,
  .use_reference = 0,
  .filter_type = FD_STAB_TEMPORAL,
  .temp = {
    .num = 8,
    .denom = 6,
  },
},
/* Mouth stabilization tuning params */
.stab_mouth = {
  .enable = 0,
  .mode = FD_STAB_CONTINUES_CLOSER_TO_REFERENCE,
  .state_cnt = 1,
  .threshold = 10,
  .use_reference = 1,
  .filter_type = FD_STAB_NO_FILTER,
},
/* Smile stabilization tuning params */
.stab_smile = {
  .enable = 1,
  .mode = FD_STAB_EQUAL,
  .state_cnt = 3,
  .threshold = 0,
  .use_reference = 0,
  .filter_type = FD_STAB_HYSTERESIS,
  .hyst = {
    .start_A = 25,
    .end_A = 35,
    .start_B = 55,
    .end_B = 70,
  },
},
/* region filter tuning params */
  .region_filter = {
  .enable = 0,
  .max_face_num = 10,
  .p_region = {0.5, 0.8, 1},
  .w_region = {100, 0.8, 0.5},
  .size_region = {80, 250, 500},
},

