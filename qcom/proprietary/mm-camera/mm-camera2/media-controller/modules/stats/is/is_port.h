/* is_port.h
 *
 * Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef __IS_PORT_H__
#define __IS_PORT_H__
#include "mct_port.h"
#include "is_thread.h"
#ifdef _ANDROID_
#include <cutils/properties.h>

#define IS_PROPERTY_MAX_LEN 96

#define IS_PROP_GET_MARGIN(margin) {\
  char prop[IS_PROPERTY_MAX_LEN];\
  property_get("persist.camera.is.margin", prop, "0.10");\
  margin = atof(prop);\
}

#define IS_PROP_GET_BUFFER_DELAY(buffer_delay) {\
  char prop[IS_PROPERTY_MAX_LEN];\
  property_get("persist.camera.is.buffer_delay", prop, "15");\
  buffer_delay = atoi(prop);\
}

#define IS_PROP_GET_NUM_MESH_X(num_mesh_x) {\
  char prop[IS_PROPERTY_MAX_LEN];\
  property_get("persist.camera.is.num_mesh_x", prop, "1");\
  num_mesh_x = atoi(prop);\
}

#define IS_PROP_GET_NUM_MESH_Y(num_mesh_y) {\
  char prop[IS_PROPERTY_MAX_LEN];\
  property_get("persist.camera.is.num_mesh_y", prop, "10");\
  num_mesh_y = atoi(prop);\
}
#else
#define IS_PROP_GET_MARGIN(margin) {margin = 0.10;}
#define IS_PROP_GET_BUFFER_DELAY(buffer_delay) {buffer_delay = 15;}
#define IS_PROP_GET_NUM_MESH_X(num_mesh_x) {num_mesh_x = 1;}
#define IS_PROP_GET_NUM_MESH_Y(num_mesh_y) {num_mesh_y = 10;}
#endif


typedef enum {
  IS_PORT_STATE_CREATED = 1,
  IS_PORT_STATE_RESERVED,
  IS_PORT_STATE_LINKED,
  IS_PORT_STATE_UNLINKED,
  IS_PORT_STATE_UNRESERVED
} is_port_state_t;


/** _is_port_private_t:
 *    @reserved_id[]: session id + stream id
 *    @state: IS port state
 *    @RSCS_stats_ready: Indicates whether row and column sum stats has arrived
 *      for the current frame (composite stats assumed)
 *    @is_output: DIS/EIS output/results
 *    @stream_type: Indicates camera or camcorder mode
 *    @is_info: IS-related information
 *    @is_process_output: Output from IS processing
 *    @set_parameters: pointer to function that sets IS parameters
 *    @process: pointer to function that leads to running IS algorithm
 *    @is_stats_cb:
 *    @thread_data: IS thread data
 *    @dual_cam_sensor_info:
 *    @intra_peer_id:
 *
 * This structure represents the IS port's internal variables.
 **/
typedef struct _is_port_private {
  unsigned int reserved_id[IS_MAX_STREAMS];
  is_port_state_t state;
  int RSCS_stats_ready;
  is_output_type is_output[IS_MAX_STREAMS];
  cam_stream_type_t stream_type;
  is_info_t is_info;
  is_process_output_t is_process_output;
  is_set_parameters_func set_parameters;
  is_process_func process;
  is_callback_func callback;
  is_stats_done_callback_func is_stats_cb;
  is_thread_data_t *thread_data;
  cam_sync_type_t dual_cam_sensor_info;
  uint32_t intra_peer_id;
} is_port_private_t;


void is_port_deinit(mct_port_t *port);
boolean is_port_find_identity(mct_port_t *port, unsigned int identity);
boolean is_port_init(mct_port_t *port, unsigned int session_id);
#endif /* __IS_PORT_H__ */
