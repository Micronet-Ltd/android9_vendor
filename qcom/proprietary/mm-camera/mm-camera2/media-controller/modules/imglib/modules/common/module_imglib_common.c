/**********************************************************************
*  Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "module_imglib_common.h"
#include <media/msmb_generic_buf_mgr.h>
#include <cutils/properties.h>
#include "img_buffer.h"

/** MOD_IMGLIB_DUMP_FRAME
 *
 * Macro to enable dump frame functionality
 **/
#define MOD_IMGLIB_DUMP_FRAME

/** module_imglib_get_next_from_list
 *    @data1: not used
 *    @data2: not used
 *
 *  Gets next element from the list
 *
 *  Return TRUE always
 **/
boolean module_imglib_get_next_from_list(void *data1, void *data2)
{
  IMG_UNUSED(data1);
  IMG_UNUSED(data2);

  return TRUE;
}

/** module_imglib_free_func
 *    @data: node data
 *    @user_data: not used. Null pointer
 *
 * Memory release traverse function for flushing queue
 *
 * Returns TRUE
 **/
boolean module_imglib_free_func(void *data, void *user_data)
{
  IMG_UNUSED(user_data);

  free(data);

  return TRUE;
}

/**
 * Function: module_imglib_msg_thread_can_wait
 *
 * Description: Queue function to check if abort is issued
 *
 * Input parameters:
 *   p_userdata - The pointer to message thread
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
int module_imglib_msg_thread_can_wait(void *p_userdata)
{
  mod_imglib_msg_th_t *p_msg_th = (mod_imglib_msg_th_t *)p_userdata;
  return (FALSE == p_msg_th->abort_flag);
}

/**
 * Function: module_imglib_post_bus_msg
 *
 * Description: post a particular message to media bus
 *
 * Arguments:
 *   @p_mct_mod - media controller module
 *   @sessionid - session id
 *   @msg_id - bus message id
 *   @msg_data - bus message data
 *   @msg_size - bus message size
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_imglib_post_bus_msg(mct_module_t *p_mct_mod,
  unsigned int sessionid, mct_bus_msg_type_t msg_id,
  void *msg_data, uint32_t msg_size)
{
  mct_bus_msg_t bus_msg;
  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.type = msg_id;
  bus_msg.size = msg_size;
  bus_msg.msg = msg_data;
  bus_msg.sessionid = sessionid;
  IDBG_MED("%s:%d] session id %d mct_mod %p", __func__, __LINE__,
    bus_msg.sessionid, p_mct_mod);
  mct_module_post_bus_msg(p_mct_mod, &bus_msg);
}

/**
 * Function: module_imglib_post_dynamic_scene_data
 *
 * Description: post dynamic scene detect data via bus message
 *
 * Arguments:
 *   @p_dyn_data - ptr to dynamic data
 *   @p_port - mct port
 *   @event_idx - event identity
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_imglib_post_dynamic_scene_data(cam_dyn_img_data_t *p_dyn_data,
  mct_port_t *p_port, uint32_t event_idx)
{
  mct_module_t *p_mct_mod;
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(p_port))->data);
  module_imglib_post_bus_msg(p_mct_mod,
    IMGLIB_SESSIONID(event_idx),
    MCT_BUS_MSG_SET_IMG_DYN_FEAT,
    p_dyn_data, sizeof(cam_dyn_img_data_t));
}

/**
 * Function: module_imglib_msg_thread
 *
 * Description: Main message thread loop
 *
 * Input parameters:
 *   data - The pointer to message thread
 *
 * Return values:
 *     NULL
 *
 * Notes: none
 **/
void *module_imglib_msg_thread(void *data)
{
  mod_imglib_msg_th_t *p_msg_th = (mod_imglib_msg_th_t *)data;
  img_queue_t *p_q = &p_msg_th->msg_q;
  mod_img_msg_t *p_msg;
  mct_event_t mct_event;
  isp_buf_divert_ack_t buff_divert_ack;

  /* signal the main thread */
  pthread_mutex_lock(&p_q->mutex);
  p_msg_th->is_ready = TRUE;
  pthread_cond_signal(&p_q->cond);
  pthread_mutex_unlock(&p_q->mutex);

  IDBG_MED("%s:%d] abort %d", __func__, __LINE__,
    p_msg_th->abort_flag);

  while ((p_msg = img_q_wait(&p_msg_th->msg_q,
    module_imglib_msg_thread_can_wait, p_msg_th)) != NULL) {

    switch (p_msg->type) {
    case MOD_IMG_MSG_BUF_ACK: {
      IDBG_MED("%s:%d] Send buffer divert Ack buf_id %d identity %x",
        __func__, __LINE__,
        p_msg->data.buf_ack.frame_id,
        p_msg->data.buf_ack.identity);
      memset(&mct_event,  0,  sizeof(mct_event));
      mct_event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT_ACK;
      mct_event.u.module_event.module_event_data = (void *)&buff_divert_ack;
      mct_event.type = MCT_EVENT_MODULE_EVENT;
      mct_event.identity = p_msg->data.buf_ack.identity;
      mct_event.direction = MCT_EVENT_UPSTREAM;
      memset(&buff_divert_ack,  0,  sizeof(buff_divert_ack));
      buff_divert_ack.buf_idx = (uint32_t)p_msg->data.buf_ack.frame_id;
      buff_divert_ack.is_buf_dirty = 0;
      mct_port_send_event_to_peer(p_msg->port, &mct_event);
      break;
    }
    case MOD_IMG_MSG_DIVERT_BUF: {
      IDBG_MED("%s:%d] Buffer divert event buf_id %d identity %x p_exec %p",
        __func__, __LINE__,
        p_msg->data.buf_divert.buf_divert.buffer.index,
        p_msg->data.buf_divert.identity,
        p_msg->data.buf_divert.p_exec);
      if (p_msg->data.buf_divert.p_exec) {
        p_msg->data.buf_divert.p_exec(p_msg->data.buf_divert.userdata,
          &p_msg->data.buf_divert);
      }
      break;
    }
    case MOD_IMG_MSG_EXEC_INFO: {
      if (p_msg->data.exec_info.p_exec) {
        p_msg->data.exec_info.p_exec(p_msg->data.exec_info.p_userdata,
          p_msg->data.exec_info.data);
      }
      break;
    }
    default:;
    }
    free(p_msg);
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return NULL;
}

/**
 * Function: module_imglib_destroy_msg_thread
 *
 * Description: This method is used to destroy the message
 *             thread
 *
 * Input parameters:
 *   p_msg_th - The pointer to message thread
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int module_imglib_destroy_msg_thread(mod_imglib_msg_th_t *p_msg_th)
{
  img_queue_t *p_q = &p_msg_th->msg_q;

  /* signal the main thread */
  pthread_mutex_lock(&p_q->mutex);
  p_msg_th->abort_flag = TRUE;
  pthread_cond_signal(&p_q->cond);
  pthread_mutex_unlock(&p_q->mutex);

  IDBG_MED("%s:%d] threadid %d", __func__, __LINE__,
    (int)p_msg_th->threadid);
  if (!pthread_equal(p_msg_th->threadid, pthread_self())) {
    pthread_join(p_msg_th->threadid, NULL);
  }
  IDBG_MED("%s:%d] after msg thread exit", __func__, __LINE__);

  /* delete the message queue */
  img_q_deinit(p_q);

  return IMG_SUCCESS;
}

/**
 * Function: module_imglib_send_msg
 *
 * Description: This method is used to send message to the
 *             message thread
 *
 * Input parameters:
 *   p_msg_th - The pointer to message thread
 *   p_msg - pointer to the message
 *
 * Return values:
 *     IMG_SUCCESS
 *
 * Notes: none
 **/
int module_imglib_send_msg(mod_imglib_msg_th_t *p_msg_th,
  mod_img_msg_t *p_msg)
{
  int status = IMG_SUCCESS;

  mod_img_msg_t *p_msg_new;

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  /* create message */
  p_msg_new = (mod_img_msg_t *)malloc(sizeof(mod_img_msg_t));
  if (NULL == p_msg_new) {
    IDBG_ERROR("%s:%d] cannot create message", __func__, __LINE__);
    return IMG_ERR_NO_MEMORY;
  }
  memcpy(p_msg_new, p_msg, sizeof(mod_img_msg_t));

  status = img_q_enqueue(&p_msg_th->msg_q, p_msg_new);
  if (IMG_ERROR(status)) {
    IDBG_ERROR("%s:%d] cannot enqueue message", __func__, __LINE__);
    free(p_msg_new);
    return status;
  }
  img_q_signal(&p_msg_th->msg_q);
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return status;
}

/**
 * Function: module_imglib_create_msg_thread
 *
 * Description: This method is used to create the
 *             message thread
 *
 * Input parameters:
 *   p_msg_th - The pointer to message thread
 *
 * Return values:
 *     IMG_SUCCESS
 *     IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int module_imglib_create_msg_thread(mod_imglib_msg_th_t *p_msg_th)
{
  img_queue_t *p_q;
  int status = IMG_SUCCESS;

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  memset(p_msg_th, 0x0, sizeof(mod_imglib_msg_th_t));

  /* initialize the queue */
  p_q = &p_msg_th->msg_q;
  p_msg_th->threadid = 0;
  p_msg_th->abort_flag = FALSE;
  img_q_init(p_q, "message_q");

  /* start the message thread */
  pthread_mutex_lock(&p_q->mutex);
  p_msg_th->is_ready = FALSE;
  status = pthread_create(&p_msg_th->threadid, NULL,
    module_imglib_msg_thread,
    (void *)p_msg_th);
  pthread_setname_np(p_msg_th->threadid, "CAM_img_msg");
  if (status < 0) {
    IDBG_ERROR("%s:%d] pthread creation failed %d",
      __func__, __LINE__, status);
    pthread_mutex_unlock(&p_q->mutex);
    return IMG_ERR_GENERAL;
  }
  pthread_mutex_unlock(&p_q->mutex);
  IDBG_MED("%s:%d] X", __func__, __LINE__);

  return status;
}

/**
 * Function: module_imglib_check_create_msg_thread
 *
 * Description: This method is used to create the
 *             message thread if not started yet
 *
 * Input parameters:
 *   p_msg_th - The pointer to message thread
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_imglib_check_create_msg_thread(mod_imglib_msg_th_t *p_msg_th)
{
  if (FALSE == p_msg_th->is_ready) {
    IDBG_HIGH("%s:%d] Creating msg thread", __func__, __LINE__);
    module_imglib_create_msg_thread(p_msg_th);
  }
}

/**
 * Function: mod_imglib_map_fd_buffer
 *
 * Description: This method is used for updating the imglib
 * buffer structure from MCT structure with face detection buffers
 *
 * Input parameters:
 *   @data - MCT stream buffer mapping
 *   @user_data - img buffer structure
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean mod_imglib_map_fd_buffer(void *data, void *user_data)
{
  mct_stream_map_buf_t *p_buf = (mct_stream_map_buf_t *)data;
  mod_img_buffer_info_t *p_buf_info = (mod_img_buffer_info_t *)user_data;
  uint32_t idx = 0;

  IDBG_MED("%s:%d] p_buf %p p_buf_info %p", __func__, __LINE__, p_buf,
    p_buf_info);
  if (!p_buf || !p_buf_info) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }

  /* For face detection is used stream buff type */
  if (p_buf->buf_type != CAM_MAPPING_BUF_TYPE_STREAM_BUF)
    return TRUE;

  /* Check the buffer count */
  if (p_buf_info->buf_count >= p_buf_info->total_count)
    return TRUE;

  idx = p_buf_info->buf_count;
  p_buf_info->p_buffer[idx].map_buf = *p_buf;
  IDBG_MED("%s:%d] buffer cnt %d idx %d addr %p", __func__, __LINE__,
    p_buf_info->buf_count, p_buf->buf_index,
    p_buf->buf_planes[0].buf);
  p_buf_info->buf_count++;
  return TRUE;
}

/**
 * Function: mod_imglib_map_fr_buffer
 *
 * Description: This method is used for updating the imglib
 * buffer structure from MCT structure with buffer for face regsitration
 *
 * Input parameters:
 *   @data - MCT stream buffer mapping
 *   @user_data - img buffer structure
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean mod_imglib_map_fr_buffer(void *data, void *user_data)
{
  mct_stream_map_buf_t *p_buf = (mct_stream_map_buf_t *)data;
  mod_img_buffer_info_t *p_buf_info = (mod_img_buffer_info_t *)user_data;
  uint32_t idx = 0;

  IDBG_MED("%s:%d] p_buf %p p_buf_info %p", __func__, __LINE__, p_buf,
    p_buf_info);
  if (!p_buf || !p_buf_info) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }

  /* For face registration is used offline input buffer sent from hal */
  if (p_buf->buf_type != CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF)
    return TRUE;

  idx = p_buf_info->buf_count;
  p_buf_info->p_buffer[idx].map_buf = *p_buf;
  IDBG_MED("%s:%d] buffer cnt %d idx %d addr %p", __func__, __LINE__,
    p_buf_info->buf_count, p_buf->buf_index,
    p_buf->buf_planes[0].buf);
  p_buf_info->buf_count++;
  return TRUE;
}
/**
 * Function: mod_imglib_convert_buffer
 *
 * Description: This method is used to convert the ISP buffer
 *        to frame buffer
 *
 * Input parameters:
 *   @data - MCT stream buffer mapping
 *   @user_data - img buffer structure
 *
 * Return values:
 *     true/false
 *
 * Notes: incomplete
 **/
int mod_imglib_convert_buffer(isp_buf_divert_t *p_isp_buf,
  img_frame_t *p_frame)
{
  int status = IMG_SUCCESS;
  IDBG_MED("%s:%d] %d", __func__, __LINE__, p_isp_buf->native_buf);
  if (!p_isp_buf->native_buf) {
    /* Todo: add support for native buffers */
    IMG_UNUSED(p_frame);
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return status;
}

/** mod_imglib_dump_stream_info
 *    @info: stream info configuration
 *
 * Prints stream info configuration
 *
 * Returns TRUE in case of success
 **/
void mod_imglib_dump_stream_info(mct_stream_info_t* info)
{
  uint32_t i;

  if (info) {
    IDBG_MED("info->stream_type %d", info->stream_type);
    IDBG_MED("info->fmt %d", info->fmt);
    IDBG_MED("info->dim.width %d", info->dim.width);
    IDBG_MED("info->dim.height %d", info->dim.height);
    IDBG_MED("info->buf_planes.plane_info.frame_len %d",
      info->buf_planes.plane_info.frame_len);
    IDBG_MED("info->buf_planes.plane_info.num_planes %d",
      info->buf_planes.plane_info.num_planes);
    IDBG_MED("info->buf_planes.plane_info.sp.len %d",
      info->buf_planes.plane_info.sp.len);
    IDBG_MED("info->buf_planes.plane_info.sp.y_offset %d",
      info->buf_planes.plane_info.sp.y_offset);
    IDBG_MED("info->buf_planes.plane_info.sp.cbcr_offset %d",
      info->buf_planes.plane_info.sp.cbcr_offset);
    for (i=0; i<(uint32_t)info->buf_planes.plane_info.num_planes; i++) {
      IDBG_MED("info->buf_planes.plane_info.mp[%d].len %d", i,
        info->buf_planes.plane_info.mp[i].len);
      IDBG_MED("info->buf_planes.plane_info.mp[%d].offset %d", i,
        info->buf_planes.plane_info.mp[i].offset);
      IDBG_MED("info->buf_planes.plane_info.mp[%d].offset_x %d", i,
        info->buf_planes.plane_info.mp[i].offset_x);
      IDBG_MED("info->buf_planes.plane_info.mp[%d].offset_y %d", i,
        info->buf_planes.plane_info.mp[i].offset_y);
      IDBG_MED("info->buf_planes.plane_info.mp[%d].stride %d", i,
        info->buf_planes.plane_info.mp[i].stride);
      IDBG_MED("info->buf_planes.plane_info.mp[%d].scanline %d", i,
        info->buf_planes.plane_info.mp[i].scanline);
    }
    IDBG_MED("info->streaming_mode %d", info->streaming_mode);
    IDBG_MED("info->num_burst %d", info->num_burst);
    IDBG_MED("info->img_buffer_list %p", info->img_buffer_list);
    IDBG_MED("info->parm_buf.type %d", info->parm_buf.type);
    IDBG_MED("info->parm_buf.reprocess.buf_index %d",
      info->parm_buf.reprocess.buf_index);
    IDBG_MED("info->parm_buf.reprocess.ret_val %d",
      info->parm_buf.reprocess.ret_val);
    IDBG_MED("info->reprocess_config.pp_type %d",
      info->reprocess_config.pp_type);
    if (CAM_ONLINE_REPROCESS_TYPE == info->reprocess_config.pp_type) {
      IDBG_MED("info->reprocess_config.online.input_stream_id %d",
        info->reprocess_config.online.input_stream_id);
    } else {
      IDBG_MED("info->reprocess_config.offline.input_fmt %d",
        info->reprocess_config.offline.input_fmt);
      IDBG_MED("info->reprocess_config.offline.input_dim.width %d",
        info->reprocess_config.offline.input_dim.width);
      IDBG_MED("info->reprocess_config.offline.input_dim.height %d",
        info->reprocess_config.offline.input_dim.height);
      IDBG_MED("info->reprocess_config.offline.num_of_bufs %d",
        info->reprocess_config.offline.num_of_bufs);
      IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
        \b\b\b\b\b\b\b\bplane_info.frame_len %d",
        info->reprocess_config.offline.input_buf_planes.plane_info.frame_len);
      IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
        \b\b\b\b\b\b\b\bplane_info.num_planes %d",
        info->reprocess_config.offline.input_buf_planes.plane_info.num_planes);
      IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
        \b\b\b\b\b\b\b\bplane_info.sp.len %d",
        info->reprocess_config.offline.input_buf_planes.plane_info.sp.len);
      IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
        \b\b\b\b\b\b\b\bplane_info.sp.y_offset %d",
        info->reprocess_config.offline.input_buf_planes.\
        plane_info.sp.y_offset);
      IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
        \b\b\b\b\b\b\b\bplane_info.sp.cbcr_offset %d",
        info->reprocess_config.offline.input_buf_planes.\
        plane_info.sp.cbcr_offset);

      for (i=0;\
        i<(uint32_t)info->reprocess_config.offline.input_buf_planes.\
        plane_info.num_planes;\
        i++) {

        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].len %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].len);
        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].offset %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].offset);
        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].offset_x %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].offset_x);
        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].offset_y %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].offset_y);
        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].stride %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].stride);
        IDBG_MED("info->reprocess_config.offline.input_buf_planes.\
          \b\b\b\b\b\b\b\b\b\bplane_info.mp[%d].scanline %d", i,
          info->reprocess_config.offline.input_buf_planes.\
          plane_info.mp[i].scanline);
      }
    }
    IDBG_MED("info->reprocess_config.pp_feature_config.feature_mask %lld",
      info->reprocess_config.pp_feature_config.feature_mask);
    IDBG_MED("info->reprocess_config.pp_feature_config.\
      \b\b\b\b\b\bdenoise2d.denoise_enable %d",
      info->reprocess_config.pp_feature_config.denoise2d.denoise_enable);
    IDBG_MED("info->reprocess_config.pp_feature_config.\
      \b\b\b\b\b\bdenoise2d.process_plates %d",
      info->reprocess_config.pp_feature_config.denoise2d.process_plates);
    IDBG_MED("info->reprocess_config.pp_feature_config.input_crop.left %d",
      info->reprocess_config.pp_feature_config.input_crop.left);
    IDBG_MED("info->reprocess_config.pp_feature_config.input_crop.top %d",
      info->reprocess_config.pp_feature_config.input_crop.top);
    IDBG_MED("info->reprocess_config.pp_feature_config.input_crop.width %d",
      info->reprocess_config.pp_feature_config.input_crop.width);
    IDBG_MED("info->reprocess_config.pp_feature_config.input_crop.height %d",
      info->reprocess_config.pp_feature_config.input_crop.height);
    IDBG_MED("info->reprocess_config.pp_feature_config.rotation %d",
      info->reprocess_config.pp_feature_config.rotation);
    IDBG_MED("info->reprocess_config.pp_feature_config.flip %d",
      info->reprocess_config.pp_feature_config.flip);
    IDBG_MED("info->reprocess_config.pp_feature_config.sharpness %d",
      info->reprocess_config.pp_feature_config.sharpness);
  }
}

/** mod_imglib_check_stream
 *    @d1: mct_stream_t* pointer to the streanm being checked
 *    @d2: uint32_t* pointer to identity
 *
 *  Check if the stream matches stream index or stream type.
 *
 *  Return: TRUE if stream matches.
 **/
static boolean mod_imglib_check_stream(void *d1, void *d2)
{
  boolean ret_val = FALSE;
  mct_stream_t *stream = (mct_stream_t *)d1;
  uint32_t *id = (uint32_t *)d2;

  if (stream && id && stream->streaminfo.identity == *id)
    ret_val = TRUE;

  return ret_val;
}

boolean mod_imglib_read_file(char* file_name, int file_count,
  img_mem_handle_t *p_buffer)
{
  FILE *fp;
  char name[FILENAME_MAX];
  int fileLen = 0;
  int i = 0, rc = 0;

  for (i = 0; i < file_count; i++) {
    snprintf(name, sizeof(name),
      "%s%s_%d%s", "/data/misc/camera/", file_name, i+1, ".yuv");
    fp = fopen(name, "rb+");
    if (!fp) {
      IDBG_ERROR("%s Cannot open file %s", __func__, name);
      goto error;
    }
    fseek(fp, 0, SEEK_END);
    fileLen = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    rc = img_buffer_get(IMG_BUFFER_ION_IOMMU, -1, 1, fileLen, &p_buffer[i]);
    if (rc != IMG_SUCCESS) {
      IDBG_ERROR("%s:%d] Img buffer get failed", __func__, __LINE__);
      goto error;
    }
    fread(p_buffer[i].vaddr, fileLen, 1, fp);
    p_buffer[i].length = fileLen;
    IDBG_INFO("%s:%d] Img buffer %d len %d", __func__, __LINE__, i, fileLen);
    fclose(fp);
  }
  return IMG_SUCCESS;

error:
  if (fp)
    fclose(fp);
  for (i = 0; i < file_count; i++) {
    if (p_buffer[i].handle) {
      img_buffer_release(&p_buffer[i]);
    }
  }
  return IMG_ERR_GENERAL;
}

/** mod_imglib_find_module_parent
 *    @identity: required identity
 *    @module: module, whichs parents will be serached
 *
 * Finds module parent (stream) with specified identity
 *
 * Returns Pointer to stream handler in case of cucess
 *   or NULL in case of failure
 **/
mct_stream_t* mod_imglib_find_module_parent(uint32_t identity,
  mct_module_t* module)
{
  mct_stream_t* ret_val = NULL;
  mct_list_t *find_list;

  if (module && MCT_MODULE_PARENT(module)) {
    find_list = mct_list_find_custom(MCT_MODULE_PARENT(module),
      &identity, mod_imglib_check_stream);

    if (find_list)
      ret_val = find_list->data;
  }

  return ret_val;
}

/** module_imglib_common_get_bfr_mngr_subdev:
 *  @buf_mgr_fd: buffer manager file descriptor
 *
 * Function to get buffer manager file descriptor
 *
 * Returns TRUE in case of success
 **/
int module_imglib_common_get_bfr_mngr_subdev(int *buf_mgr_fd)
{
  struct media_device_info mdev_info;
  int32_t num_media_devices = 0;
  char dev_name[32];
  char subdev_name[32];
  int32_t dev_fd = 0, ioctl_ret;
  boolean ret = FALSE;

  while (buf_mgr_fd) {
    uint32_t num_entities = 1;
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    if (dev_fd < 0) {
      IDBG_ERROR("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      break;
    }
    num_media_devices++;
    ioctl_ret = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (ioctl_ret < 0) {
      IDBG_ERROR("%s:%d Done enumerating media devices\n", __func__, __LINE__);
      close(dev_fd);
      break;
    }
    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0) {
      close(dev_fd);
      continue;
    }
    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities++;
      IDBG_MED("%s:%d entity id %d", __func__, __LINE__, entity.id);
      ioctl_ret = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (ioctl_ret < 0) {
        IDBG_ERROR("%s:%d Done enumerating media entities\n",
          __func__, __LINE__);
        ret = FALSE;
        break;
      }
      IDBG_MED("%s:%d entity name %s type %d group id %d\n", __func__, __LINE__,
        entity.name, entity.type, entity.group_id);

      IDBG_MED("%s:group_id=%d", __func__, entity.group_id);

      if (entity.type == MEDIA_ENT_T_V4L2_SUBDEV &&
          entity.group_id == MSM_CAMERA_SUBDEV_BUF_MNGR) {
        snprintf(subdev_name, sizeof(dev_name), "/dev/%s", entity.name);

        IDBG_MED("%s:subdev_name=%s", __func__, subdev_name);

        *buf_mgr_fd = open(subdev_name, O_RDWR);
        IDBG_MED("%s: *buf_mgr_fd=%d\n", __func__, *buf_mgr_fd);
        if (*buf_mgr_fd < 0) {
          IDBG_ERROR("%s: Open subdev failed\n", __func__);
          continue;
        }
        ret = TRUE;
        IDBG_MED("%s:%d:ret=%d\n", __func__, __LINE__, ret);
        close(dev_fd);
        return ret;
      }
    }
    close(dev_fd);
  }
  IDBG_MED("%s:%d] ret=%d\n", __func__, __LINE__, ret);
  return (TRUE == ret) ? IMG_SUCCESS : IMG_ERR_GENERAL;
}

/** module_imglib_common_get_buffer:
 *  @subdev_fd: buffer mgr fd
 *  @identity: stream/session id
 *
 * Function to get buffer for denoise port
 *
 * Returns buffer index
 **/
int module_imglib_common_get_buffer(int subdev_fd, uint32_t identity)
{
  struct msm_buf_mngr_info buff;
  int32_t ret;

  IDBG_MED("%s +", __func__);

  buff.session_id = IMGLIB_SESSIONID(identity);
  buff.stream_id = IMGLIB_STREAMID(identity);
  buff.type = MSM_CAMERA_BUF_MNGR_BUF_PLANAR;

  ret = ioctl(subdev_fd, VIDIOC_MSM_BUF_MNGR_GET_BUF, &buff);
  if (ret < 0) {
    IDBG_ERROR("Failed to get buffer from buffer manager %d", ret);
    return -1;
  }
  IDBG_LOW("VIDIOC_MSM_BUF_MNGR_GET_BUF buf index %d, ", (int32_t)buff.index);
  return (int32_t)buff.index;
}

/** module_imglib_common_release_buffer:
 *  @subdev_fd: buffer mgr fd
 *  @identity: stream/session id
 *  @idx: buffer index
 *  @frame_id: frame id
 *  @buff_done: buff done/put buff
 *
 * Function to release buffer back to kernel
 *
 * Returns imaging error values
 **/
int module_imglib_common_release_buffer(int subdev_fd, uint32_t identity,
  uint32_t idx, uint32_t frame_id, boolean buff_done)
{
  struct msm_buf_mngr_info buff_info;
  int32_t cmd;
  int ret;

  IDBG_MED("%s:%d]", __func__, __LINE__);

  buff_info.index = idx;
  buff_info.session_id = IMGLIB_SESSIONID(identity);
  buff_info.stream_id = IMGLIB_STREAMID(identity);
  buff_info.frame_id = frame_id;
  IDBG_MED("%s:%d] Release frameId %d", __func__, __LINE__, frame_id);

  if (buff_done)
    cmd = (int32_t)VIDIOC_MSM_BUF_MNGR_BUF_DONE;
  else
    cmd = (int32_t)VIDIOC_MSM_BUF_MNGR_PUT_BUF;
  ret = ioctl(subdev_fd, cmd, &buff_info);

  if (ret < 0) {
    IDBG_MED("%s:%d] Failed to do buf_done id 0x%x %d %d",
      __func__, __LINE__, identity, idx, frame_id);
    return IMG_ERR_GENERAL;
  }

  IDBG_MED("%s:%d]", __func__, __LINE__);

  return IMG_SUCCESS;
}

/**
 * Function: module_imglib_common_get_misc_buff
 *
 * Description: Function used as callback to find
 *   miscellaneous buffer with corresponding index
 *
 * Input parameters:
 *   @data - MCT stream buffer mapping
 *   @user_data - Pinter of searched buffer index
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_imglib_common_get_misc_buff(void *data, void *user_data)
{
  mct_stream_map_buf_t *p_buf = (mct_stream_map_buf_t *)data;
  uint8_t *p_buf_index = (uint8_t *)user_data;

  if (!p_buf || !p_buf_index) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] buf type %d buff index %d search index %d",
    __func__, __LINE__, p_buf->buf_type, p_buf->buf_index, *p_buf_index);

  /* For face detection is used stream buff type */
  if (p_buf->buf_type != CAM_MAPPING_BUF_TYPE_MISC_BUF)
    return FALSE;

  return ((uint8_t)p_buf->buf_index == *p_buf_index);
}

/** module_imglib_common_get_miscdata:
 *  @info: Stream info
 *  @meta_index: Miscellaneous buffer index
 *
 * Function to get miscellaneous buffer pointer
 *
 * Returns Pointer to miscellaneous buffer / NULL on fail
 **/
void *module_imglib_common_get_miscdata(mct_stream_info_t *info,
  uint32_t misc_index)
{
  void *miscdata_buff = NULL;
  mct_list_t *temp_list;

  if (!info) {
    IDBG_ERROR("%s:%d Invalid input %p", __func__, __LINE__, info);
    return NULL;
  }

  if (CAM_STREAM_TYPE_OFFLINE_PROC != info->stream_type) {
    IDBG_LOW("%s:%d Misc data is null for stream type %d", __func__, __LINE__,
      info->stream_type);
    return NULL;
  }


  temp_list = mct_list_find_custom(info->img_buffer_list,
      &misc_index, module_imglib_common_get_misc_buff);
  if (temp_list && temp_list->data) {
    mct_stream_map_buf_t *buff_holder = temp_list->data;
    miscdata_buff = buff_holder->buf_planes[0].buf;
  } else {
    IDBG_HIGH("%s:%d] miscellaneous buffer idx %d is not available",
      __func__, __LINE__, misc_index);
  }

  return miscdata_buff;
}

/**
 * Function: module_imglib_common_get_meta_buff
 *
 * Description: Function used as callback to find
 *   metadata buffer wht corresponding index
 *
 * Input parameters:
 *   @data - MCT stream buffer mapping
 *   @user_data - Pinter of searched buffer index
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_imglib_common_get_meta_buff(void *data, void *user_data)
{
  mct_stream_map_buf_t *p_buf = (mct_stream_map_buf_t *)data;
  uint8_t *p_buf_index = (uint8_t *)user_data;

  if (!p_buf || !p_buf_index) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] buf type %d buff index %d search index %d",
    __func__, __LINE__, p_buf->buf_type, p_buf->buf_index, *p_buf_index);

  /* For face detection is used stream buff type */
  if (p_buf->buf_type != CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF)
    return FALSE;

  return ((uint8_t)p_buf->buf_index == *p_buf_index);
}

/** module_imglib_common_get_metadata:
 *  @info: Stream info
 *  @meta_index: Metadata buffer index
 *
 * Function to get metadata buffer pointer
 *
 * Returns Pointer to metadata buffer / NULL on fail
 **/
metadata_buffer_t *module_imglib_common_get_metadata(mct_stream_info_t *info,
  uint32_t meta_index)
{
  metadata_buffer_t *metadata_buff = NULL;
  mct_list_t *temp_list;

  if (!info) {
    IDBG_ERROR("%s:%d Invalid input %p", __func__, __LINE__, info);
    return NULL;
  }

  temp_list = mct_list_find_custom(info->img_buffer_list,
      &meta_index, module_imglib_common_get_meta_buff);
  if (temp_list && temp_list->data) {
    mct_stream_map_buf_t *buff_holder = temp_list->data;
    metadata_buff = buff_holder->buf_planes[0].buf;
  } else {
    IDBG_ERROR("%s:%d] Metadata buffer idx %d is not available",
        __func__, __LINE__, meta_index);
  }

  return metadata_buff;
}


/**
 * Function: module_imglib_common_get_zoom_level
 *
 * Description: This function is returning current zoom ratio
 *
 * Arguments:
 *   p_mct_cap - capababilities
 *   @zoom_ratio: zoom ratio
 *
 * Return values:
 *     zoom level, -1 if error
 *
 * Notes: none
 **/
int module_imglib_common_get_zoom_level(mct_pipeline_cap_t *p_mct_cap,
  float zoom_ratio)
{
  mct_pipeline_isp_cap_t* p_isp_cap;
  int i;
  int ret_val = -1;
  int zoom_ratio_int = (int)(zoom_ratio * MOD_IMGLIB_ZOOM_DENUMINATOR);

  p_isp_cap = &p_mct_cap->isp_cap;

  for (i=0; i<p_isp_cap->zoom_ratio_tbl_cnt; i++) {
    if (zoom_ratio_int <= p_isp_cap->zoom_ratio_tbl[i]) {
      ret_val = i;
      break;
    }
  }

  return ret_val;
}

/**
 * Function: module_imglib_common_find_meta_by_frameid
 *
 * Description: Function used in list find custom to
 *  find matching metadata frame id with current frame id
 *
 * Arguments:
 *   @data1: metadata
 *   @data2: current frame id
 *
 * Return values:
 *   TRUE/FALSE
 *
 **/
static boolean module_imglib_common_find_meta_by_frameid(void *data1,
  void *data2)
{
  img_meta_t *p_meta = (img_meta_t *)data1;
  uint32_t *current_frame_id = (unsigned int *)data2;

  if (!data1 || !data2) {
    return FALSE;
  }

  if (p_meta->frame_id == *current_frame_id) {
    return TRUE;
  }

  return FALSE;
}

/**
 * Function: module_imglib_common_meta_find_by_frameid
 *
 * Description: This function is finds metadata pointer for current frame id
 *
 * Arguments:
 *   @current_frame_id: current frame id
 *   @p_meta_list - list of frame metadata
 *   @p_meta: output metadata pointer for current frame id
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_find_by_frameid(uint32_t current_frame_id,
  mct_list_t *p_meta_list, img_meta_t** p_meta)
{
  mct_list_t* p_list;

  p_list = mct_list_find_custom(p_meta_list, &current_frame_id,
    module_imglib_common_find_meta_by_frameid);

  if (p_list && p_list->data) {
    *p_meta = p_list->data;
    return IMG_SUCCESS;
  }

  *p_meta = NULL;

  return IMG_ERR_NOT_FOUND;
}

/**
 * Function: module_imglib_common_meta_get_by_frameid
 *
 * Description: This function gets metadata pointer for current frame id
 *
 * Arguments:
 *   @current_frame_id: current frame id
 *   @p_meta_list - list of frame metadata
 *   @p_meta: output metadata pointer for current frame id
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_get_by_frameid(uint32_t current_frame_id,
  mct_list_t **p_meta_list, img_meta_t** p_meta)
{
  int32_t rc;

  rc = module_imglib_common_meta_find_by_frameid(current_frame_id, *p_meta_list,
    p_meta);

  if (IMG_SUCCESS != rc) {
    *p_meta = calloc(1, sizeof(img_meta_t));
    if (NULL == *p_meta) {
      IDBG_ERROR("%s:%d Out of memory", __func__, __LINE__);
      return IMG_ERR_NO_MEMORY;
    }

    (*p_meta)->frame_id = current_frame_id;
    *p_meta_list = mct_list_append(*p_meta_list, *p_meta, NULL, NULL);
  }

  return IMG_SUCCESS;
}

/**
 * Function: module_imglib_common_meta_set_aec
 *
 * Description: This function sets aec in frame metadata
 *
 * Arguments:
 *   @p_meta_list - list of frame metadata
 *   @stats_update: stats update event payload
 *   @current_frame_id: current frame id
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_aec(mct_list_t **p_meta_list,
  stats_update_t *stats_update, uint32_t current_frame_id)
{
  img_meta_t* p_meta;
  aec_update_t* aec_update;
  int32_t rc;
  img_aec_info_t aec_info;

  if (!stats_update) {
    IDBG_ERROR("%s:%d Null pointer detected", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  rc = module_imglib_common_meta_get_by_frameid(current_frame_id, p_meta_list,
    &p_meta);

  if (IMG_SUCCESS == rc) {
    aec_update = &stats_update->aec_update;
    aec_info.real_gain = aec_update->real_gain;
    aec_info.linecount = aec_update->linecount;
    aec_info.exp_time = aec_update->exp_time;
    aec_info.lux_idx = aec_update->lux_idx;
    aec_info.iso = aec_update->exif_iso;

    rc = img_set_meta(p_meta, IMG_META_AEC_INFO, &aec_info);
  } else {
    IDBG_ERROR("%s:%d Setting aec failed", __func__, __LINE__);
  }

  return rc;
}

/**
 * Function: module_imglib_common_meta_set_awb
 *
 * Description: This function sets awb in frame metadata
 *
 * Arguments:
 *   @p_meta_list - list of frame metadata
 *   @stats_update: stats update event payload
 *   @current_frame_id: current frame id
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_awb(mct_list_t **p_meta_list,
  stats_update_t *stats_update, uint32_t current_frame_id)
{
  img_meta_t* p_meta;
  awb_update_t* awb_update;
  img_awb_info_t awb_info;
  int32_t rc;

  if (!stats_update) {
    IDBG_ERROR("%s:%d Null pointer detected", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  rc = module_imglib_common_meta_get_by_frameid(current_frame_id, p_meta_list,
    &p_meta);

  if (IMG_SUCCESS == rc) {
    awb_update = &stats_update->awb_update;
    awb_info.r_gain = awb_update->gain.r_gain;
    awb_info.g_gain = awb_update->gain.g_gain;
    awb_info.b_gain = awb_update->gain.b_gain;
    awb_info.color_temp = awb_update->color_temp;

    rc = img_set_meta(p_meta, IMG_META_AWB_INFO, &awb_info);
  } else {
    IDBG_ERROR("%s:%d Setting awb failed", __func__, __LINE__);
  }

  return rc;
}

/**
 * Function: module_imglib_common_meta_set_gamma
 *
 * Description: This function sets gamma in frame metadata
 *
 * Arguments:
 *   @p_meta_list - list of frame metadata
 *   @p_gamma: stats update event payload
 *   @current_frame_id: current frame id
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_gamma(mct_list_t **p_meta_list,
  void *p_gamma, uint32_t current_frame_id)
{
  img_meta_t* p_meta;
  int32_t rc;

  if (!p_gamma) {
    IDBG_ERROR("%s:%d Null pointer detected", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  rc = module_imglib_common_meta_get_by_frameid(current_frame_id, p_meta_list,
    &p_meta);

  if (IMG_SUCCESS == rc) {
    rc = img_set_meta(p_meta, IMG_META_R_GAMMA, p_gamma);
    rc += img_set_meta(p_meta, IMG_META_G_GAMMA, p_gamma);
    rc += img_set_meta(p_meta, IMG_META_B_GAMMA, p_gamma);
  } else {
    IDBG_ERROR("%s:%d Setting gamma failed", __func__, __LINE__);
  }

  return rc;
}

/**
 * Function: module_imglib_common_meta_set_fd_info
 *
 * Description: This function sets fd info in frame metadata
 *
 * Arguments:
 *   @comb_faces_data: combined fd info
 *   @p_meta: metadata
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_fd_info(
  cam_faces_data_t *comb_faces_data, img_meta_t *p_meta)
{
  img_fd_info_t fd_info;
  cam_face_detection_data_t *faces_data;
  cam_face_gaze_data_t *gazes_data;
  int32_t rc;
  int32_t i;

  if (!comb_faces_data || !p_meta) {
    IDBG_ERROR("%s:%d Null pointer detected comb_faces_data %p p_meta %p",
      __func__, __LINE__, comb_faces_data, p_meta);
    return IMG_ERR_INVALID_INPUT;
  }

  faces_data = &(comb_faces_data->detection_data);
  gazes_data = &(comb_faces_data->gaze_data);

  if (faces_data->num_faces_detected > MAX_ROI ||
    faces_data->num_faces_detected > IMG_MAX_FACES) {
    IDBG_HIGH("%s:%d Invalid num faces %d", __func__, __LINE__,
      faces_data->num_faces_detected);
    return IMG_ERR_INVALID_INPUT;
  }

  fd_info.valid_faces_detected = 0;

  for (i = 0; i < faces_data->num_faces_detected; i++) {
    if (fd_info.valid_faces_detected < IMG_MAX_FACES) {
      fd_info.faceRollDir[fd_info.valid_faces_detected] =
        gazes_data->gaze[fd_info.valid_faces_detected].roll_dir;
      fd_info.faceROIx[fd_info.valid_faces_detected] =
        faces_data->faces[i].face_boundary.left;
      fd_info.faceROIy[fd_info.valid_faces_detected] =
        faces_data->faces[i].face_boundary.top;
      fd_info.faceROIWidth[fd_info.valid_faces_detected] =
        faces_data->faces[i].face_boundary.width;
      fd_info.faceROIHeight[fd_info.valid_faces_detected] =
        faces_data->faces[i].face_boundary.height;

      IDBG_LOW("%s:%d] Face "
        "left %d top %d size %d x %d",
        __func__, __LINE__,
        fd_info.faceROIx[fd_info.valid_faces_detected],
        fd_info.faceROIy[fd_info.valid_faces_detected],
        fd_info.faceROIWidth[fd_info.valid_faces_detected],
        fd_info.faceROIHeight[fd_info.valid_faces_detected]);

      fd_info.valid_faces_detected++;
    }
  }
  fd_info.fd_frame_width  = faces_data->fd_frame_dim.width;
  fd_info.fd_frame_height = faces_data->fd_frame_dim.height;

  rc = img_set_meta(p_meta, IMG_META_NO_ROT_FD_INFO, &fd_info);

  return rc;
}

/**
 * Function: module_imglib_common_meta_set_rotation
 *
 * Description: This function sets rotation in frame metadata
 *
 * Arguments:
 *   @rotation: rotation
 *   @p_meta: metadata
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_rotation(cam_rotation_info_t *rotation,
  img_meta_t *p_meta)
{
  int32_t rc;
  img_rotation_t new_rotation;

  if (!p_meta) {
    IDBG_ERROR("%s:%d Null pointer detected, p_meta %p",
      __func__, __LINE__, p_meta);
    return IMG_ERR_INVALID_INPUT;
  }

  if (rotation->rotation == ROTATE_0) {
    new_rotation.frame_rotation = 0;
  } else if (rotation->rotation == ROTATE_90) {
    new_rotation.frame_rotation = 90;
  } else if (rotation->rotation == ROTATE_180) {
    new_rotation.frame_rotation = 180;
  } else if (rotation->rotation == ROTATE_270) {
    new_rotation.frame_rotation = 270;
  } else {
    return IMG_ERR_INVALID_INPUT;
  }

  if (rotation->device_rotation == ROTATE_0) {
    new_rotation.device_rotation = 0;
  } else if (rotation->device_rotation == ROTATE_90) {
    new_rotation.device_rotation = 90;
  } else if (rotation->device_rotation == ROTATE_180) {
    new_rotation.device_rotation = 180;
  } else if (rotation->device_rotation == ROTATE_270) {
    new_rotation.device_rotation = 270;
  } else {
    return IMG_ERR_INVALID_INPUT;
  }

  rc = img_set_meta(p_meta, IMG_META_ROTATION, &new_rotation);

  IDBG_MED("%s:%d] frame rotation %d device rotation %d",
    __func__, __LINE__, new_rotation.frame_rotation,
    new_rotation.device_rotation);

  return rc;
}

/**
 * Function: module_imglib_common_meta_set_flip
 *
 * Description: This function sets flip in frame metadata
 *
 * Arguments:
 *   @flip_mask: flip mask
 *   @p_meta: metadata
 *
 * Return values:
 *   standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_set_flip(int32_t flip_mask,
  img_meta_t *p_meta)
{
  int32_t rc;

  if (!p_meta) {
    IDBG_ERROR("%s:%d Null pointer detected. p_meta %p",
      __func__, __LINE__, p_meta);
    return IMG_ERR_INVALID_INPUT;
  }

  rc = img_set_meta(p_meta, IMG_META_FLIP, &flip_mask);
  return rc;
}

/**
 * Function: module_imglib_common_meta_fd_info_apply_rotation
 *
 * Description: This function applies rotation on fd info in frame metadata
 *
 * Arguments:
 *   @face_detect_tilt_cut_off - maximum angle for face tilt filter
 *   @p_meta: metadata
 *
 * Return values:
 *    standard image lib return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_meta_fd_info_apply_rotation(
  int32_t face_detect_tilt_cut_off, img_meta_t *p_meta)
{
  img_rotation_t *p_rotation;
  img_fd_info_t *no_rot_fd_info;
  int32_t device_rotation;
  int32_t frame_rotation;
  int32_t orientation;
  img_fd_info_t fd_info;
  int32_t rc;
  uint32_t i;
  int32_t faceDirection;
  int32_t *p_flip;
  int32_t flip;

  if (!p_meta) {
    IDBG_MED("%s:%d Error, null ptr p_meta %p", __func__, __LINE__, p_meta);
    return IMG_ERR_INVALID_INPUT;
  }

  no_rot_fd_info = (img_fd_info_t *)img_get_meta(p_meta,
    IMG_META_NO_ROT_FD_INFO);
  if (!no_rot_fd_info) {
    IDBG_MED("%s:%d Error, null ptr no_rot_fd_info %p",
      __func__, __LINE__, no_rot_fd_info);
    return IMG_ERR_INVALID_INPUT;
  }

  p_rotation = (img_rotation_t *)img_get_meta(p_meta, IMG_META_ROTATION);
  p_flip = (int32_t *)img_get_meta(p_meta, IMG_META_FLIP);

  fd_info.valid_faces_detected = 0;
  device_rotation = (p_rotation) ? p_rotation->device_rotation : 0;
  frame_rotation = (p_rotation) ? p_rotation->frame_rotation : 0;
  flip = (p_flip) ? *p_flip : 0;

  IDBG_MED("%s:%d] device rotation %d, frame rotation %d, flip %d",
    __func__, __LINE__, device_rotation, frame_rotation, flip);

  if ((device_rotation == 0) || (device_rotation == 180)) {
    fd_info.fd_frame_width = no_rot_fd_info->fd_frame_width;
    fd_info.fd_frame_height = no_rot_fd_info->fd_frame_height;
  } else {
    fd_info.fd_frame_width = no_rot_fd_info->fd_frame_height;
    fd_info.fd_frame_height = no_rot_fd_info->fd_frame_width;
  }

  for (i = 0; i < no_rot_fd_info->valid_faces_detected; i++) {
    /* check orientation of the face; if it is in gravity direction,
      and orientation is betwene -45 and 45, then only it should
      be passed to trueportrait; if not filter that face alone */

    faceDirection = no_rot_fd_info->faceRollDir[i];
    orientation = faceDirection + device_rotation;

    if ((device_rotation == 180) && (faceDirection > 90) &&
      (faceDirection < 180)) {
      orientation = faceDirection - device_rotation;
    }

    IDBG_MED("%s:%d] roll_dir before sensor mount angle comp off %d, %d",
      __func__, __LINE__, faceDirection, orientation);

    if (ABS(orientation) < face_detect_tilt_cut_off) {

      fd_info.faceRollDir[i] = orientation;

      switch (frame_rotation) {
      case 0:
        if (!flip) {
          fd_info.faceROIx[fd_info.valid_faces_detected] =
            no_rot_fd_info->faceROIx[i];
        } else {
          fd_info.faceROIx[fd_info.valid_faces_detected] =
            no_rot_fd_info->fd_frame_width - no_rot_fd_info->faceROIWidth[i] -
            no_rot_fd_info->faceROIx[i];
        }
        fd_info.faceROIy[fd_info.valid_faces_detected] =
          no_rot_fd_info->faceROIy[i];
        fd_info.faceROIWidth[fd_info.valid_faces_detected] =
          no_rot_fd_info->faceROIWidth[i];
        fd_info.faceROIHeight[fd_info.valid_faces_detected] =
          no_rot_fd_info->faceROIHeight[i];
        break;
      case 90:
        if (!flip) {
          fd_info.faceROIx[fd_info.valid_faces_detected] =
            no_rot_fd_info->fd_frame_height - no_rot_fd_info->faceROIHeight[i] -
            no_rot_fd_info->faceROIy[i];
        } else {
          fd_info.faceROIx[fd_info.valid_faces_detected] =
            no_rot_fd_info->faceROIy[i];
        }
        fd_info.faceROIy[fd_info.valid_faces_detected] =
          no_rot_fd_info->faceROIx[i];
        fd_info.faceROIWidth[fd_info.valid_faces_detected] =
          no_rot_fd_info->faceROIHeight[i];
        fd_info.faceROIHeight[fd_info.valid_faces_detected] =
          no_rot_fd_info->faceROIWidth[i];
        break;
      case 180:
        if (!flip) {
          fd_info.faceROIx[fd_info.valid_faces_detected] =
            no_rot_fd_info->fd_frame_width - no_rot_fd_info->faceROIWidth[i] -
            no_rot_fd_info->faceROIx[i];
        } else {
          fd_info.faceROIx[fd_info.valid_faces_detected] =
            no_rot_fd_info->faceROIx[i];
        }
        fd_info.faceROIy[fd_info.valid_faces_detected] =
          no_rot_fd_info->fd_frame_height - no_rot_fd_info->faceROIHeight[i] -
          no_rot_fd_info->faceROIy[i];
        fd_info.faceROIWidth[fd_info.valid_faces_detected] =
          no_rot_fd_info->faceROIWidth[i];
        fd_info.faceROIHeight[fd_info.valid_faces_detected] =
          no_rot_fd_info->faceROIHeight[i];
        break;
      case 270:
      case -90:
        if (!flip) {
          fd_info.faceROIx[fd_info.valid_faces_detected] =
            no_rot_fd_info->faceROIy[i];
        } else {
          fd_info.faceROIx[fd_info.valid_faces_detected] =
            no_rot_fd_info->fd_frame_height - no_rot_fd_info->faceROIHeight[i] -
            no_rot_fd_info->faceROIy[i];
        }
        fd_info.faceROIy[fd_info.valid_faces_detected] =
          no_rot_fd_info->fd_frame_width - no_rot_fd_info->faceROIWidth[i] -
          no_rot_fd_info->faceROIx[i];
        fd_info.faceROIWidth[fd_info.valid_faces_detected] =
          no_rot_fd_info->faceROIHeight[i];
        fd_info.faceROIHeight[fd_info.valid_faces_detected] =
          no_rot_fd_info->faceROIWidth[i];
        break;
      default:
        IDBG_ERROR("%s:%d Not supported frame rotation %d", __func__, __LINE__,
          p_rotation->frame_rotation);
        return IMG_ERR_INVALID_INPUT;
      }

      IDBG_MED("%s:%d] Rotated face rol dir %d left %d top %d size %d x %d",
        __func__, __LINE__, fd_info.faceRollDir[i],
        fd_info.faceROIx[fd_info.valid_faces_detected],
        fd_info.faceROIy[fd_info.valid_faces_detected],
        fd_info.faceROIWidth[fd_info.valid_faces_detected],
        fd_info.faceROIHeight[fd_info.valid_faces_detected]);

      fd_info.valid_faces_detected++;
    }
  }

  rc = img_set_meta(p_meta, IMG_META_FD_INFO, &fd_info);

  IDBG_MED("%s:%d] Final number of faces filtered is %d outof %d, %d x %d",
    __func__, __LINE__, fd_info.valid_faces_detected,
    no_rot_fd_info->valid_faces_detected, fd_info.fd_frame_width,
    fd_info.fd_frame_height);

  return rc;
}

/**
 * Function: module_imglib_common_get_hal_meta
 *
 * Description: This function getsimage lib debug data for hal meta data
 *
 * Arguments:
 *   @data: imglib metadata
 *   @user_data: hal metadata
 *
 * Return values:
 *    true/false
 *
 * Notes: none
 **/
static boolean module_imglib_common_get_hal_meta(void *data, void *user_data)
{
  img_meta_t *p_meta = (img_meta_t *)data;
  cam_intf_meta_imglib_t* hal_debug_meta =
    (cam_intf_meta_imglib_t*)user_data;
  img_aec_info_t *p_aec_info;
  uint32_t frame_count;

  if (!data || !user_data) {
    IDBG_ERROR("%s:%d Null pointer detected data %p user_data %p", __func__,
      __LINE__, data, user_data);
    return FALSE;
  }

  p_aec_info = (img_aec_info_t *)img_get_meta(p_meta, IMG_META_AEC_INFO);
  frame_count = hal_debug_meta->meta_imglib_input_aec.frame_count;

  if (p_aec_info && (frame_count < CAM_INTF_AEC_DATA_MAX)) {
    hal_debug_meta->meta_imglib_input_aec.aec_data[frame_count].real_gain =
      p_aec_info->real_gain;
    hal_debug_meta->meta_imglib_input_aec.aec_data[frame_count].lux_idx =
      p_aec_info->lux_idx;
    hal_debug_meta->meta_imglib_input_aec.aec_data[frame_count].exp_time =
      p_aec_info->exp_time;
    hal_debug_meta->meta_imglib_input_aec.frame_count++;
  }

  return TRUE;
}

/**
 * Function: module_imglib_common_fill_hal_meta
 *
 * Description: This function fills hal meta data with image lib debug data
 *
 * Arguments:
 *   @data: hal metadata ptr
 *   @user_data: hal metadata
 *
 * Return values:
 *    true/false
 *
 * Notes: none
 **/
static boolean module_imglib_common_fill_hal_meta(void *data, void *user_data)
{
  cam_intf_meta_imglib_t* hal_debug_meta =
    (cam_intf_meta_imglib_t*)user_data;

  if (!data || !user_data) {
    IDBG_ERROR("%s:%d Null pointer detected data %p user_data %p", __func__,
      __LINE__, data, user_data);
    return FALSE;
  }

  add_metadata_entry(CAM_INTF_META_IMGLIB, sizeof(cam_intf_meta_imglib_t),
    hal_debug_meta, data);

  return TRUE;
}

/**
 * Function: module_imglib_common_add_imglib_debug_meta
 *
 * Description: This function fills adds imglib debug metato in hal meta
 *
 * Arguments:
 *   @p_hal_meta_list: list of hal metadata
 *   @p_meta_list: list of frame metadata
 *
 * Return values:
 *    standard image libe return codes
 *
 * Notes: none
 **/
int32_t module_imglib_common_add_imglib_debug_meta(mct_list_t *p_hal_meta_list,
  mct_list_t *p_meta_list)
{
  boolean rc;
  cam_intf_meta_imglib_t hal_debug_meta;
  char value[PROPERTY_VALUE_MAX];

  property_get("persist.camera.mobicat", value, "0");
  if (0 == atoi(value)) {
    return IMG_SUCCESS;
  }

  if (!p_hal_meta_list || !p_meta_list) {
    IDBG_ERROR("%s:%d Null pointer detected, p_hal_meta %p p_meta %p",
      __func__, __LINE__, p_hal_meta_list, p_meta_list);
    return IMG_ERR_INVALID_INPUT;
  }

  hal_debug_meta.meta_imglib_input_aec.frame_count = 0;
  rc = mct_list_traverse(p_meta_list, module_imglib_common_get_hal_meta,
    &hal_debug_meta);

  if (rc) {
    rc = mct_list_traverse(p_hal_meta_list, module_imglib_common_fill_hal_meta,
      &hal_debug_meta);
  }

  return rc ? IMG_SUCCESS : IMG_ERR_GENERAL;
}

/**
 * Function: module_imglib_common_get_prop
 *
 * Description: This function returns property value in 32-bit
 * integer
 *
 * Arguments:
 *   @prop_name: name of the property
 *   @def_val: default value of the property
 *
 * Return values:
 *    value of the property in 32-bit integer
 *
 * Notes: API will return 0 in case of error. The API wont
 *    check for validation of the inputs. The caller must ensure
 *    that the property name and default value is correct.
 **/
int32_t module_imglib_common_get_prop(const char* prop_name,
  const char* def_val)
{
  int32_t prop_val;
#ifdef _ANDROID_
  char prop[PROPERTY_VALUE_MAX];

  property_get(prop_name, prop, def_val);
  /* If all angles search is used disable device usage of device orientation */
  prop_val = atoi(prop);
#else
  prop_val = atoi(def_val);
#endif
  return prop_val;
}
