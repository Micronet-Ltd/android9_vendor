/*============================================================================

  Copyright (c) 2013-2016 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.

============================================================================*/
#include "c2d_thread.h"
#include "c2d_log.h"
#include "pp_buf_mgr.h"
#include <poll.h>
#include <unistd.h>
#include "c2d_hardware.h"
#include "c2d_module.h"
#define PIPE_FD_IDX   0
#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
//#define SUBDEV_FD_IDX 1
extern c2d_hardware_stream_status_t*
  c2d_hardware_get_stream_status(c2d_hardware_t* c2dhw, uint32_t identity);
static int32_t c2d_thread_process_pipe_message(c2d_module_ctrl_t *ctrl,
  c2d_thread_msg_t msg);
static void c2d_thread_fatal_exit(c2d_module_ctrl_t *ctrl, boolean post_to_bus);
static int32_t c2d_thread_process_hardware_event(c2d_module_ctrl_t *ctrl);
/** c2d_thread_func:
 *
 * Description:
 *   Entry point for c2d_thread. Polls over pipe read fd and c2d
 *   hw subdev fd. If there is any new pipe message or hardware
 *   event, it is processed.
 **/
static void* c2d_thread_func(void* data)
{
  int32_t rc;
  mct_module_t *module = (mct_module_t *) data;
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  PTHREAD_MUTEX_LOCK(&(ctrl->c2d_mutex));
  ctrl->c2d_thread_started = TRUE;
  pthread_cond_signal(&(ctrl->th_start_cond));
  PTHREAD_MUTEX_UNLOCK(&(ctrl->c2d_mutex));
  /* poll on the pipe readfd and subdev fd */
  struct pollfd pollfds;
  uint32_t num_fds = 1;
  int ready=0;
  uint32_t i=0;
  pollfds.fd = ctrl->pfd[READ_FD];
  pollfds.events = POLLIN|POLLPRI;
  C2D_HIGH("c2d_thread entering the polling loop...");
  while(1) {
    /* poll on the fds with no timeout */
    ready = poll(&pollfds, (nfds_t)num_fds, -1);
    if (ready > 0) {
      if (pollfds.revents & (POLLIN|POLLPRI)) {
        int num_read=0;
        c2d_thread_msg_t pipe_msg;
        num_read = read(pollfds.fd, &(pipe_msg),
                     sizeof(c2d_thread_msg_t));
        if(num_read < 0) {
          C2D_ERR("read() failed, rc=%d", num_read);
          c2d_thread_fatal_exit(ctrl, TRUE);
        } else if(num_read != sizeof(c2d_thread_msg_t)) {
          C2D_ERR("failed, in read(), num_read=%d, msg_size=%zu",
            num_read, sizeof(c2d_thread_msg_t));
          c2d_thread_fatal_exit(ctrl, FALSE);
        }
        rc = c2d_thread_process_pipe_message(ctrl, pipe_msg);
        if (rc < 0) {
          C2D_ERR("failed");
          c2d_thread_fatal_exit(ctrl, FALSE);
        }
      } /* if */
    } else if(ready == 0){
      C2D_ERR("error: poll() timed out");
      c2d_thread_fatal_exit(ctrl, FALSE);
    } else if (ready < 0 && errno == EINTR) {
      C2D_ERR("error: poll() failed error = %s %d, continue...",
        strerror(errno), errno);
      continue;
    } else {
      C2D_ERR("error: poll() failed error =%s %d",
        strerror(errno), errno);
      c2d_thread_fatal_exit(ctrl, FALSE);
    }
  } /* while(1) */
  return NULL;
}

/* c2d_thread_handle_divert_buf_event:
 *
 *   send a buf divert event to downstream module, if the piggy-backed ACK
 *   is received, we can update the ACK from ack_list, otherwise, the ACK will
 *   be updated when buf_divert_ack event comes from downstream module.
 *
 **/
static int32_t c2d_thread_handle_divert_buf_event(c2d_module_ctrl_t* ctrl,
  c2d_module_event_t* c2d_event)
{
  int32_t rc;
  mct_event_t event;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.identity = c2d_event->u.divert_buf_data.div_identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  event.u.module_event.module_event_data =
    &(c2d_event->u.divert_buf_data.isp_buf_divert);

  c2d_event->u.divert_buf_data.isp_buf_divert.ack_flag = FALSE;

  C2D_DBG("sending unproc_div, identity=0x%x", event.identity);
  rc = c2d_module_send_event_downstream(ctrl->p_module, &event);
  if (rc < 0) {
    C2D_ERR("failed");
    return -EFAULT;
  }
  C2D_DBG("unprocessed divert ack = %d",
    c2d_event->u.divert_buf_data.isp_buf_divert.ack_flag);

  /* if ack is piggy backed, we can safely send ack to upstream */
  if (c2d_event->u.divert_buf_data.isp_buf_divert.ack_flag == TRUE) {
    C2D_LOW("doing ack for divert event");
    c2d_module_do_ack(ctrl, c2d_event->ack_key);
  }
  return 0;
}

/* c2d_hardware_validate_params:
 *
 * Description:
 *
 *
 **/
static boolean c2d_hardware_validate_params(c2d_hardware_params_t *hw_params)
{

  C2D_LOW("inw=%d, inh=%d, outw=%d, outh=%d",
    hw_params->input_info.width, hw_params->input_info.height,
    hw_params->output_info.width, hw_params->output_info.height);
  C2D_LOW("inst=%d, insc=%d, outst=%d, outsc=%d",
    hw_params->input_info.plane_info.mp[0].stride,
    hw_params->input_info.plane_info.mp[0].scanline,
    hw_params->output_info.plane_info.mp[0].stride,
    hw_params->output_info.plane_info.mp[0].scanline);

  if (hw_params->input_info.width <= 0 || hw_params->input_info.height <= 0) {
    C2D_ERR("invalid input dim");
    return FALSE;
  }
  /* TODO: add mode sanity checks */
  return TRUE;
}

int32_t c2d_module_get_native_output_buf(c2d_module_ctrl_t *ctrl,
  c2d_module_stream_params_t *stream_params,
  mct_stream_map_buf_t *output_buf)
{
  mct_list_t *buf_list=NULL;
  pp_frame_buffer_t *img_buf;
  int32_t recv_count;

  C2D_DBG("E: identity=%x", stream_params->streaming_identity);

  recv_count = pp_native_manager_get_bufs(&ctrl->native_buf_mgr, 1,
    C2D_GET_SESSION_ID(stream_params->streaming_identity),
    C2D_GET_STREAM_ID(stream_params->streaming_identity), &buf_list);
  if (recv_count != 1) {
    C2D_ERR("failed, recv_count=%d", recv_count);
    return -1;
  }

  if (!buf_list) {
    C2D_ERR("failed");
    return -1;
  } else if(!buf_list->data) {
    C2D_ERR("failed");
    return -1;
  }
  img_buf = (pp_frame_buffer_t*) buf_list->data;

  output_buf->buf_index = img_buf->buffer.index;
  output_buf->buf_planes[0].buf = img_buf->vaddr;
  output_buf->buf_planes[0].fd = img_buf->fd;
  output_buf->buf_size = img_buf->ion_alloc[0].len;

  mct_list_free_list(buf_list);
  return 0;
}

/* c2d_thread_check_apsect_ratio:
 *
 * @ roi_cfg - pointer to the crop data
 * @ hw_params - pointer to hardware parameters
 *
 * The functions check the aspect ratio of the input and output frames. If they
 * differ the function modify the crop data to match the apsect ratio.
 *
 * No return value.
 **/
static void c2d_thread_check_apsect_ratio(c2d_roi_cfg_t *roi_cfg,
  c2d_hardware_params_t* hw_params)
{
  uint32_t new_dimension,dst_aspect_ratio,src_aspect_ratio;

  if (!hw_params->output_info.width || !hw_params->output_info.height ||
    !roi_cfg->height) {
    C2D_ERR("Invalid parameter, output_info.width = %d,"
      "output_info.height = %d,roi_cfg->height = %d",
      hw_params->output_info.width ,hw_params->output_info.height,
      roi_cfg->height);
    return;
  }
  dst_aspect_ratio = hw_params->output_info.width * 10 /
    hw_params->output_info.height;
  src_aspect_ratio = roi_cfg->width * 10 /roi_cfg->height;

  if ((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
    dst_aspect_ratio = hw_params->output_info.height * 10 /
      hw_params->output_info.width;
  }

  if (src_aspect_ratio > dst_aspect_ratio) {
    new_dimension = roi_cfg->height * hw_params->output_info.width /
      hw_params->output_info.height;
    roi_cfg->x += (roi_cfg->width - new_dimension) / 2;
    roi_cfg->width = new_dimension;
  } else if (src_aspect_ratio < dst_aspect_ratio){
    new_dimension = roi_cfg->width * hw_params->output_info.height /
      hw_params->output_info.width;
    roi_cfg->y += (roi_cfg->height - new_dimension) / 2;
    roi_cfg->height = new_dimension;
  }
}


/* c2d_thread_handle_stream_off_event:
 *
 * Description:
 *
 *
 **/
static int32_t c2d_thread_handle_streamoff_event(c2d_module_ctrl_t* ctrl,
  c2d_module_event_t* c2d_event)
{

  int32_t rc = 0;
  /* process hardware command for stream off, this ensures
     hardware is done with this identity */
  c2d_hardware_cmd_t cmd;
  cmd.type = C2D_HW_CMD_STREAMOFF;
  cmd.u.streamoff_identity = c2d_event->u.streamoff_data.identity;
  rc = c2d_hardware_process_command(ctrl->c2dhw, cmd);
  return rc;
}


/* c2d_thread_handle_process_buf_event:
 *
 * Description:
 *
 *
 **/
static int32_t c2d_thread_handle_process_buf_event(c2d_module_ctrl_t* ctrl,
  c2d_module_event_t* c2d_event)
{
  int32_t rc = 0;
  boolean bool_ret = TRUE;
  unsigned long in_frame_fd;
  mct_event_t event;
  c2d_hardware_cmd_t cmd;
  mct_stream_map_buf_t output_buf;
  c2d_frame c2d_input_buffer, c2d_output_buffer;
  c2d_process_frame_buffer c2d_process_buffer;
  uint32 x,y;

  if(!ctrl || !c2d_event) {
    C2D_ERR("failed, ctrl=%p, c2d_event=%p", ctrl, c2d_event);
    return -EINVAL;
  }

  c2d_hardware_params_t* hw_params;
  hw_params = &(c2d_event->u.process_buf_data.hw_params);
  if (c2d_event->u.process_buf_data.isp_buf_divert.native_buf) {
    in_frame_fd = (unsigned long)c2d_event->u.process_buf_data.isp_buf_divert.fd;
    /* Get virtual address from isp divert structure */
    hw_params->input_buffer_info.vaddr =
    ( void *) ((unsigned long *)c2d_event->u.process_buf_data.isp_buf_divert.vaddr)[0];
  } else {
    in_frame_fd =
      c2d_event->u.process_buf_data.isp_buf_divert.buffer.m.planes[0].m.userptr;
    C2D_DBG("input buf index %d, fd %lu",
      c2d_event->u.process_buf_data.isp_buf_divert.buffer.index, in_frame_fd);
    /* Get virtual address for input buffer */
    bool_ret = pp_buf_mgr_get_vaddr(ctrl->buf_mgr,
      c2d_event->u.process_buf_data.isp_buf_divert.buffer.index,
      c2d_event->u.process_buf_data.input_stream_info,
      &hw_params->input_buffer_info.vaddr);
    if (bool_ret == FALSE) {
      C2D_ERR("failed: pp_buf_mgr_get_vaddr()\n");
      return c2d_module_do_ack(ctrl, c2d_event->ack_key);
    }
    C2D_HIGH("input vaddr %p\n", hw_params->input_buffer_info.vaddr);
  }
  hw_params->frame_id =
    c2d_event->u.process_buf_data.isp_buf_divert.buffer.sequence;
  hw_params->timestamp =
    c2d_event->u.process_buf_data.isp_buf_divert.buffer.timestamp;
  hw_params->identity = c2d_event->u.process_buf_data.proc_identity;
  hw_params->input_buffer_info.fd = in_frame_fd;
  hw_params->input_buffer_info.index =
    c2d_event->u.process_buf_data.isp_buf_divert.buffer.index;
  hw_params->input_buffer_info.native_buff =
    c2d_event->u.process_buf_data.isp_buf_divert.native_buf;
  hw_params->processed_divert =
    c2d_event->u.process_buf_data.proc_div_required;

  if (!c2d_event->u.process_buf_data.stream_info) {
    C2D_ERR("failed: stream_info NULL\n");
    c2d_module_do_ack(ctrl, c2d_event->ack_key);
    return -EINVAL;
  }

  if (!ctrl->c2d || !ctrl->c2d_ctrl) {
    C2D_ERR("failed: c2d / c2d ctrl NULL\n");
    c2d_module_do_ack(ctrl, c2d_event->ack_key);
    return -EINVAL;
  }

  c2d_module_stream_params_t *stream_params = NULL;
  c2d_module_stream_params_t *linked_stream_params = NULL;
  c2d_module_session_params_t *session_params = NULL;
  c2d_module_get_params_for_identity(ctrl, hw_params->identity,
    &session_params, &stream_params);
  if (!stream_params) {
    C2D_ERR("failed\n");
    c2d_module_do_ack(ctrl, c2d_event->ack_key);
    return -EFAULT;
  }
  if (stream_params->native_buff) {
    rc = c2d_module_get_native_output_buf(ctrl, stream_params, &output_buf);
    if (rc < 0) {
      C2D_ERR("failed: pp_buf_mgr_get_buf()\n");
      return c2d_module_do_ack(ctrl, c2d_event->ack_key);
    }
  } else {
    mct_stream_map_buf_t *stream_map_buf;
    /* Get output buffer from buffer manager */
    stream_map_buf = pp_buf_mgr_get_buf(ctrl->buf_mgr,
      c2d_event->u.process_buf_data.stream_info);
    if (!stream_map_buf) {
      C2D_ERR("failed: pp_buf_mgr_get_buf()\n");
      return c2d_module_do_ack(ctrl, c2d_event->ack_key);
    }
    output_buf = *stream_map_buf;
  }

  /* before giving the frame to hw, make sure the parameters are good */
  if(FALSE == c2d_hardware_validate_params(hw_params)) {
    C2D_ERR("hw_params invalid, dropping frame.");
    return c2d_module_do_ack(ctrl, c2d_event->ack_key);
  }
  C2D_DBG("output buf index %d, buf size %d\n",
    output_buf.buf_index, output_buf.buf_size);

  /* Initialize input frame */
  memset(&c2d_input_buffer, 0, sizeof(c2d_input_buffer));
  if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CBCR) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_420_NV12;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRCB) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_420_NV21;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CBCR422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_422_NV16;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRCB422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_422_NV61;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_YCBYCR422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_RAW_8BIT_YUYV;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_YCRYCB422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_RAW_8BIT_YVYU;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CBYCRY422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_RAW_8BIT_UYVY;
  } else if (hw_params->input_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRYCBY422) {
    c2d_input_buffer.format = CAM_FORMAT_YUV_RAW_8BIT_VYUY;
  }
  if ((c2d_input_buffer.format == CAM_FORMAT_YUV_422_NV61) ||
    (c2d_input_buffer.format == CAM_FORMAT_YUV_422_NV16)) {
    /* Copy input buffer to output buffer */
    memcpy(output_buf.buf_planes[0].buf,
      (const void *)hw_params->input_buffer_info.vaddr,
      output_buf.buf_size);
  } else {
  c2d_input_buffer.width = hw_params->input_info.width;
  c2d_input_buffer.height = hw_params->input_info.height;
  c2d_input_buffer.num_planes = 2;
  if (stream_params->stream_info->is_type == IS_TYPE_EIS_2_0) {
    uint32_t biger_dim;

    if (stream_params->linked_stream_params) {
      biger_dim = hw_params->output_info.width >
        stream_params->linked_stream_params->hw_params.output_info.width ?
        hw_params->output_info.width :
        stream_params->linked_stream_params->hw_params.output_info.width;
      c2d_input_buffer.x_border = c2d_input_buffer.width - biger_dim;

      biger_dim = hw_params->output_info.height >
        stream_params->linked_stream_params->hw_params.output_info.height ?
        hw_params->output_info.height :
        stream_params->linked_stream_params->hw_params.output_info.height;
      c2d_input_buffer.y_border = c2d_input_buffer.height - biger_dim;
    } else {
      c2d_input_buffer.x_border = c2d_input_buffer.width -
        hw_params->output_info.width;
      c2d_input_buffer.y_border = c2d_input_buffer.height -
        hw_params->output_info.height;
    }
  }
  /* Plane 0 */
  c2d_input_buffer.mp[0].vaddr = hw_params->input_buffer_info.vaddr;
  c2d_input_buffer.mp[0].length = hw_params->input_info.plane_info.mp[0].len;
  c2d_input_buffer.mp[0].fd = hw_params->input_buffer_info.fd;
  c2d_input_buffer.mp[0].addr_offset = 0;
  c2d_input_buffer.mp[0].data_offset =
    hw_params->input_info.plane_info.mp[0].offset;
  c2d_input_buffer.mp[0].stride = hw_params->input_info.plane_info.mp[0].stride;
  c2d_input_buffer.mp[0].scanline =
    hw_params->input_info.plane_info.mp[0].scanline;
  if ((c2d_input_buffer.format >= CAM_FORMAT_YUV_RAW_8BIT_YUYV) &&
     (c2d_input_buffer.format <= CAM_FORMAT_YUV_RAW_8BIT_VYUY)) {
    c2d_input_buffer.num_planes = 1;
    c2d_input_buffer.mp[0].length *= 2;
  } else {
    /* Plane 1 */
    c2d_input_buffer.mp[1].vaddr =
      (void *)((unsigned long)c2d_input_buffer.mp[0].vaddr +
        c2d_input_buffer.mp[0].length);
    c2d_input_buffer.mp[1].length = hw_params->input_info.plane_info.mp[1].len;
    c2d_input_buffer.mp[1].fd = hw_params->input_buffer_info.fd;
    c2d_input_buffer.mp[1].addr_offset = 0;
    /* plane_info of ISP output buffers is specified differently. mp[i].offset
     * means the offset from base of mp[0], not base of mp[i] */
    c2d_input_buffer.mp[1].data_offset =
      hw_params->input_info.plane_info.mp[1].offset;
    c2d_input_buffer.mp[1].stride =
      hw_params->input_info.plane_info.mp[1].stride;
    c2d_input_buffer.mp[1].scanline =
      hw_params->input_info.plane_info.mp[1].scanline;
  }

  /* This is taken care of by above new "len" member */
#if 0
  if ((c2d_input_buffer.format == CAM_FORMAT_YUV_422_NV61) ||
       (c2d_input_buffer.format == CAM_FORMAT_YUV_422_NV16)) {
    c2d_input_buffer.mp[1].length = (hw_params->input_info.stride *
      hw_params->input_info.scanline);
  }
#endif

  /* Configure c2d IS parameters */

  if (hw_params->crop_info.is_crop.use_3d && stream_params->dis_enable) {
    c2d_input_buffer.lens_correction_cfg.use_LC = TRUE;
    c2d_input_buffer.lens_correction_cfg.transform_mtx =
      &hw_params->crop_info.is_crop.transform_matrix[0];
    c2d_input_buffer.lens_correction_cfg.transform_type =
      hw_params->crop_info.is_crop.transform_type;
    C2D_FEATURE_INFO("EIS %s for stream %d, identity %x, frame %d",
      (((c2d_event->u.process_buf_data.input_stream_info->is_type ==
      IS_TYPE_EIS_2_0) &&
      (c2d_input_buffer.lens_correction_cfg.use_LC)) ? "ON" : "OFF"),
      c2d_event->u.process_buf_data.input_stream_info->stream_type,
      hw_params->identity, hw_params->frame_id);
  } else {
    c2d_input_buffer.lens_correction_cfg.use_LC = FALSE;
  }

  /* Configure c2d for crop paramters required by the stream */

  if (((c2d_event->u.process_buf_data.input_stream_info &&
    c2d_event->u.process_buf_data.input_stream_info->is_type ==
    IS_TYPE_EIS_2_0) || stream_params->interleaved) &&
    !stream_params->single_module){
    c2d_input_buffer.roi_cfg.x = hw_params->crop_info.is_crop.x;
    c2d_input_buffer.roi_cfg.y = hw_params->crop_info.is_crop.y;
    if (!hw_params->crop_info.is_crop.dx && ! hw_params->crop_info.is_crop.dy){
      c2d_input_buffer.roi_cfg.width  = c2d_input_buffer.width;
      c2d_input_buffer.roi_cfg.height = c2d_input_buffer.height;
    }
    else {
      c2d_input_buffer.roi_cfg.width = hw_params->crop_info.is_crop.dx;
      c2d_input_buffer.roi_cfg.height = hw_params->crop_info.is_crop.dy;
    }
  } else {
    if (!hw_params->crop_info.stream_crop.dx ||
        ! hw_params->crop_info.stream_crop.dy){
      c2d_input_buffer.roi_cfg.width  =hw_params->input_info.width;
      c2d_input_buffer.roi_cfg.height = hw_params->input_info.height;
    }
    x = (hw_params->crop_info.stream_crop.x * hw_params->crop_info.is_crop.dx) /
        hw_params->input_info.width;
    y = (hw_params->crop_info.stream_crop.y * hw_params->crop_info.is_crop.dy) /
        hw_params->input_info.height;

    /* calculate the first pixel in window */
    c2d_input_buffer.roi_cfg.x =
        x + hw_params->crop_info.is_crop.x;
    /* calculate the first line in window */
    c2d_input_buffer.roi_cfg.y =
        y + hw_params->crop_info.is_crop.y;
    /* calculate the window width */
    c2d_input_buffer.roi_cfg.width =
        (hw_params->crop_info.stream_crop.dx * hw_params->crop_info.is_crop.dx)/
        hw_params->input_info.width;
    /* calculate the window height */
    c2d_input_buffer.roi_cfg.height =
        (hw_params->crop_info.stream_crop.dy * hw_params->crop_info.is_crop.dy)/
        hw_params->input_info.height;

    C2D_HIGH("c2d_input_buffer.roi_cfg.x %d c2d_input_buffer.roi_cfg.y %d" \
      "c2d_input_buffer.roi_cfg.width %d c2d_input_buffer.roi_cfg.height %d",
      c2d_input_buffer.roi_cfg.x,c2d_input_buffer.roi_cfg.y,
      c2d_input_buffer.roi_cfg.width,c2d_input_buffer.roi_cfg.height);
  }

  c2d_thread_check_apsect_ratio(&c2d_input_buffer.roi_cfg, hw_params);

  /* Initialize output frame */
  memset(&c2d_output_buffer, 0, sizeof(c2d_output_buffer));
  if (hw_params->output_info.c2d_plane_fmt == C2D_PARAM_PLANE_CBCR) {
    c2d_output_buffer.format = CAM_FORMAT_YUV_420_NV12;
  } else if (hw_params->output_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRCB) {
    c2d_output_buffer.format = CAM_FORMAT_YUV_420_NV21;
  } else if (hw_params->output_info.c2d_plane_fmt == C2D_PARAM_PLANE_CBCR422) {
    c2d_output_buffer.format = CAM_FORMAT_YUV_422_NV16;
  } else if (hw_params->output_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRCB422) {
    c2d_output_buffer.format = CAM_FORMAT_YUV_422_NV61;
  } else if (hw_params->output_info.c2d_plane_fmt == C2D_PARAM_PLANE_CRCB420) {
    c2d_output_buffer.format = CAM_FORMAT_YUV_420_YV12;
  }
  c2d_output_buffer.width = hw_params->output_info.width;
  c2d_output_buffer.height = hw_params->output_info.height;

  if (c2d_output_buffer.format == CAM_FORMAT_YUV_420_YV12)
    c2d_output_buffer.num_planes = 3;
  else
    c2d_output_buffer.num_planes = 2;

  c2d_output_buffer.roi_cfg.x = 0;
  c2d_output_buffer.roi_cfg.y = 0;
  c2d_output_buffer.roi_cfg.width = hw_params->output_info.width;
  c2d_output_buffer.roi_cfg.height = hw_params->output_info.height;
  /* Plane 0 */
  c2d_output_buffer.mp[0].vaddr = output_buf.buf_planes[0].buf;
  c2d_output_buffer.mp[0].length = hw_params->output_info.plane_info.mp[0].len;
  c2d_output_buffer.mp[0].fd = (unsigned long)output_buf.buf_planes[0].fd;
  c2d_output_buffer.mp[0].addr_offset = 0;
  c2d_output_buffer.mp[0].data_offset =
    hw_params->output_info.plane_info.mp[0].offset;
  c2d_output_buffer.mp[0].stride =
    hw_params->output_info.plane_info.mp[0].stride;
  c2d_output_buffer.mp[0].scanline =
    hw_params->output_info.plane_info.mp[0].scanline;

  /* Plane 1 */
  c2d_output_buffer.mp[1].vaddr =
    (void *)((unsigned long)c2d_output_buffer.mp[0].vaddr +
    c2d_output_buffer.mp[0].length);
  c2d_output_buffer.mp[1].length = hw_params->output_info.plane_info.mp[1].len;
  c2d_output_buffer.mp[1].fd = (unsigned long)output_buf.buf_planes[0].fd;
  c2d_output_buffer.mp[1].addr_offset = 0;
  c2d_output_buffer.mp[1].data_offset =
    hw_params->output_info.plane_info.mp[1].offset;
  c2d_output_buffer.mp[1].stride =
    hw_params->output_info.plane_info.mp[1].stride;
  c2d_output_buffer.mp[1].scanline =
    hw_params->output_info.plane_info.mp[1].scanline;

  if (c2d_output_buffer.format == CAM_FORMAT_YUV_420_YV12) {
  /* Plane 2 */
    c2d_output_buffer.mp[2].length = hw_params->output_info.plane_info.mp[2].len;
    c2d_output_buffer.mp[2].vaddr =
      (void *)((unsigned long)c2d_output_buffer.mp[1].vaddr +
      c2d_output_buffer.mp[1].length);
    c2d_output_buffer.mp[2].fd = (unsigned long)output_buf.buf_planes[0].fd;
    c2d_output_buffer.mp[2].addr_offset = 0;
    c2d_output_buffer.mp[2].data_offset = hw_params->output_info.plane_info.mp[2].offset;
    c2d_output_buffer.mp[2].stride =
      hw_params->output_info.plane_info.mp[2].stride;
    c2d_output_buffer.mp[2].scanline =
      hw_params->output_info.plane_info.mp[2].scanline;
  }

  /* This is taken care of by above new "len" member */
#if 0
  if ((c2d_output_buffer.format == CAM_FORMAT_YUV_422_NV61) ||
       (c2d_output_buffer.format == CAM_FORMAT_YUV_422_NV16)) {
    c2d_output_buffer.mp[1].length = (hw_params->output_info.stride *
      hw_params->output_info.scanline);
  }
#endif

  c2d_process_buffer.c2d_input_buffer = &c2d_input_buffer;
  c2d_process_buffer.c2d_output_buffer = &c2d_output_buffer;
  /* Hardcode 180 rotation for testing */
  //c2d_process_buffer.rotation = (0x00000001 << 1);
  int32_t   swap_dim;

  if((hw_params->rotation == 1) || (hw_params->rotation == 3)) {
    swap_dim = hw_params->output_info.width;
    hw_params->output_info.width = hw_params->output_info.height;
    hw_params->output_info.height = swap_dim;
  }

  switch(hw_params->rotation) {
  case 0:
    c2d_process_buffer.rotation = 0;
  break;
  case 1:
   c2d_process_buffer.rotation = (0x00000001 << 0);
   break;
  case 2:
   c2d_process_buffer.rotation = (0x00000001 << 1);
   break;
  case 3:
   c2d_process_buffer.rotation = (0x00000001 << 2);
   break;
  default:
   C2D_ERR("Not a valid Rotation");
 }

  c2d_process_buffer.flip = 0;
  c2d_process_buffer.c2d_input_lib_params = stream_params->c2d_input_lib_params;
  c2d_process_buffer.c2d_output_lib_params = stream_params->c2d_output_lib_params;

  /* Call process frame on c2d */
  rc = ctrl->c2d->func_tbl->process(ctrl->c2d_ctrl, PPROC_IFACE_PROCESS_FRAME,
    &c2d_process_buffer);
  if (rc != 0) {
    C2D_ERR("failed: ctrl->c2d->func_tbl->process()\n");
  }
  }

  if ((stream_params->stream_info->is_type == IS_TYPE_EIS_2_0 ||
    stream_params->interleaved) && !stream_params->single_module) {
    bool put_buf = FALSE;
    if (stream_params->is_stream_on) {
      mct_event_t event;
      isp_buf_divert_t  isp_buf;
      struct v4l2_plane plane;
      void* vaddr = NULL;

      memset(&isp_buf, 0, sizeof(isp_buf));
      memset(&plane, 0, sizeof(plane));

      if (stream_params->native_buff) {
        pp_frame_buffer_t* pp_buf = pp_native_buf_manage_get_buf_info(
          &ctrl->native_buf_mgr,
          C2D_GET_SESSION_ID(stream_params->streaming_identity),
          C2D_GET_STREAM_ID(stream_params->streaming_identity),
          output_buf.buf_index);
        if (!pp_buf) {
          C2D_ERR("failed, idx=%d, native_identity=%x",
            output_buf.buf_index, stream_params->streaming_identity);
          return -1;
        }
        vaddr = pp_buf->vaddr;
        isp_buf.vaddr = &vaddr;
      } else
        isp_buf.vaddr = output_buf.buf_planes[0].buf;

      isp_buf.native_buf = stream_params->native_buff;
      isp_buf.buffer.sequence =
        c2d_event->u.divert_buf_data.isp_buf_divert.buffer.sequence;
      isp_buf.buffer.index = output_buf.buf_index;
      isp_buf.buffer.length =
        c2d_event->u.divert_buf_data.isp_buf_divert.buffer.length;
      isp_buf.buffer.m.planes = &plane;
      isp_buf.buffer.m.planes[0].m.userptr =
        (unsigned long)output_buf.buf_planes[0].fd;
      isp_buf.fd = output_buf.buf_planes[0].fd;
      isp_buf.buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
      isp_buf.buffer.memory = V4L2_MEMORY_USERPTR;
      isp_buf.is_uv_subsampled =
        c2d_event->u.divert_buf_data.isp_buf_divert.is_uv_subsampled;

      isp_buf.buffer.timestamp =
        c2d_event->u.divert_buf_data.isp_buf_divert.buffer.timestamp;
      isp_buf.identity = hw_params->identity;

      event.direction = MCT_EVENT_DOWNSTREAM;
      event.identity = hw_params->identity;
      event.type = MCT_EVENT_MODULE_EVENT;
      event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
      event.u.module_event.module_event_data = &isp_buf;

      cmd.type = C2D_HW_CMD_DIVERT_BUF;
      cmd.u.event_data.frame_id = hw_params->frame_id;
      cmd.u.event_data.identity = hw_params->identity;
      cmd.u.event_data.buf_idx = output_buf.buf_index;
      cmd.u.event_data.cookie = NULL;
      c2d_hardware_process_command(ctrl->c2dhw, cmd);

      C2D_DBG("send divert down for %d", hw_params->frame_id);
      rc = c2d_module_send_event_downstream(ctrl->p_module, &event);
      if (rc < 0) {
        C2D_ERR("failed, module_event_type=%d, identity=0x%x",
          event.u.module_event.type, hw_params->identity);
        put_buf = TRUE;
        rc = -EFAULT;
      } else {
      if (isp_buf.ack_flag == TRUE)
        put_buf = TRUE;
      }
    } else {
      put_buf = TRUE;
    }
    if (put_buf) {
      if (stream_params->native_buff) {
        pp_native_manager_put_buf(&ctrl->native_buf_mgr,
          C2D_GET_SESSION_ID(stream_params->streaming_identity),
          C2D_GET_STREAM_ID(stream_params->streaming_identity),
          output_buf.buf_index);
      } else {
        pp_buf_mgr_put_buf(ctrl->buf_mgr,hw_params->identity,
          output_buf.buf_index,
          c2d_event->u.divert_buf_data.isp_buf_divert.buffer.sequence,
          c2d_event->u.divert_buf_data.isp_buf_divert.buffer.timestamp);
      }
      cmd.type = C2D_HW_CMD_RELEASE_DIVERT_BUF;
      cmd.u.event_data.frame_id = hw_params->frame_id;
      cmd.u.event_data.identity = hw_params->identity;
      cmd.u.event_data.buf_idx = output_buf.buf_index;
      cmd.u.event_data.cookie = NULL;
      c2d_hardware_process_command(ctrl->c2dhw, cmd);
    }
  } else {
    /* Put buffer */
      bool_ret = pp_buf_mgr_buf_done(ctrl->buf_mgr,
      c2d_event->u.process_buf_data.stream_info->identity,
      output_buf.buf_index, hw_params->frame_id,
      hw_params->timestamp);
      /* TODO need divert update here ??? */
  }

  c2d_module_do_ack(ctrl, c2d_event->ack_key);

  return rc;
}

/* c2d_thread_get_event_from_queue
 *
 * Description:
 * - dq event from the queue based on priority. if there is any event in
 *   realtime queue, return it. Only when there is nothing in realtime queue,
 *   get event from offline queue.
 * - Get hardware related event only if the hardware is ready to process.
 **/
static c2d_module_event_t* c2d_thread_get_event_from_queue(
  c2d_module_ctrl_t *ctrl)
{
  if(!ctrl) {
    C2D_ERR("failed");
    return NULL;
  }
  c2d_module_event_t *c2d_event = NULL;
  /* TODO: see if this hardware related logic is suitable in this function
     or need to put it somewhere else */
  if (c2d_hardware_get_status(ctrl->c2dhw) == C2D_HW_STATUS_READY) {
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->realtime_queue.q) == FALSE) {
      c2d_event = (c2d_module_event_t *)
                  mct_queue_pop_head(ctrl->realtime_queue.q);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
      return c2d_event;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->offline_queue.q) == FALSE) {
      c2d_event = (c2d_module_event_t *)
                  mct_queue_pop_head(ctrl->offline_queue.q);
      PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
      return c2d_event;
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
  } else {
    PTHREAD_MUTEX_LOCK(&(ctrl->realtime_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->realtime_queue.q) == FALSE) {
      c2d_event = (c2d_module_event_t *)
                    mct_queue_look_at_head(ctrl->realtime_queue.q);
      if(!c2d_event) {
        PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
        return NULL;
      }
      if (c2d_event->hw_process_flag == FALSE) {
        c2d_event = (c2d_module_event_t *)
                      mct_queue_pop_head(ctrl->realtime_queue.q);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
        return c2d_event;
      }
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->realtime_queue.mutex));
    PTHREAD_MUTEX_LOCK(&(ctrl->offline_queue.mutex));
    if(MCT_QUEUE_IS_EMPTY(ctrl->offline_queue.q) == FALSE) {
      c2d_event = (c2d_module_event_t *)
                    mct_queue_look_at_head(ctrl->offline_queue.q);
      if(!c2d_event) {
        PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
        return NULL;
      }
      if (c2d_event->hw_process_flag == FALSE) {
        c2d_event = (c2d_module_event_t *)
                      mct_queue_pop_head(ctrl->offline_queue.q);
        PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
        return c2d_event;
      }
    }
    PTHREAD_MUTEX_UNLOCK(&(ctrl->offline_queue.mutex));
  }
  return NULL;
}

/* c2d_thread_process_queue_event:
 *
 * Description:
 *
 **/
static int32_t c2d_thread_process_queue_event(c2d_module_ctrl_t *ctrl,
  c2d_module_event_t* c2d_event)
{
  int32_t rc = 0;
  if(!ctrl || !c2d_event) {
    C2D_ERR("failed, ctrl=%p, c2d_event=%p", ctrl, c2d_event);
    return -EINVAL;
  }
  /* if the event is invalid, no need to process, just free the memory */
  if(c2d_event->invalid == TRUE) {
    C2D_DBG("invalidated event received.");
    free(c2d_event);
    return 0;
  }
  switch(c2d_event->type) {
  case C2D_MODULE_EVENT_DIVERT_BUF:
    C2D_LOW("C2D_MODULE_EVENT_DIVERT_BUF");
    rc = c2d_thread_handle_divert_buf_event(ctrl, c2d_event);
    break;
  case C2D_MODULE_EVENT_PROCESS_BUF:
    C2D_LOW("C2D_MODULE_EVENT_PROCESS_BUF");
    rc = c2d_thread_handle_process_buf_event(ctrl, c2d_event);
    break;
  case C2D_MODULE_EVENT_STREAM_OFF:
    rc = c2d_thread_handle_streamoff_event(ctrl, c2d_event);
    break;
  default:
    C2D_ERR("failed, bad event type=%d", c2d_event->type);
    free(c2d_event);
    return -EINVAL;
  }
  /* free the event memory */
  free(c2d_event);
  if (rc < 0) {
    C2D_ERR("failed, rc=%d", rc);
  }
  return rc;
}

/* c2d_thread_process_pipe_message:
 *
 * Description:
 *
 **/
int32_t c2d_thread_process_pipe_message(c2d_module_ctrl_t *ctrl,
  c2d_thread_msg_t msg)
{
  int32_t rc = 0;
  c2d_hardware_cmd_t cmd;
  switch(msg.type) {
  case C2D_THREAD_MSG_ABORT: {
    C2D_HIGH("C2D_THREAD_MSG_ABORT: c2d_thread exiting..");
    ctrl->c2d_thread_started = FALSE;
#if 0
    cmd.type = C2D_HW_CMD_UNSUBSCRIBE_EVENT;
    c2d_hardware_process_command(ctrl->c2dhw, cmd);
#endif
    pthread_exit(NULL);
  }
  case C2D_THREAD_MSG_NEW_EVENT_IN_Q: {
    C2D_LOW("C2D_THREAD_MSG_NEW_EVENT_IN_Q:");
    c2d_module_event_t* c2d_event;
    /* while there is some valid event in queue process it */
    while(1) {
      c2d_event = c2d_thread_get_event_from_queue(ctrl);
      if(!c2d_event) {
        break;
      }
      rc = c2d_thread_process_queue_event(ctrl, c2d_event);
      if(rc < 0) {
        C2D_ERR("c2d_thread_process_queue_event() failed");
      }
    }
    break;
  }
  default:
    C2D_ERR("error: bad msg type=%d", msg.type);
    return -EINVAL;
  }
  return rc;
}

/* c2d_thread_fatal_exit:
 *
 * Description:
 *
 **/
void c2d_thread_fatal_exit(c2d_module_ctrl_t *ctrl, boolean post_to_bus)
{
  C2D_ERR(", fatal error: killing c2d_thread....!");
  if(post_to_bus) {
    C2D_DBG("posting error to MCT BUS!");
    /* TODO: add code to post error on mct bus */
  }
  ctrl->c2d_thread_started = FALSE;
  pthread_exit(NULL);
}

/* c2d_thread_create:
 *
 * Description:
 *
 **/
int32_t c2d_thread_create(mct_module_t *module)
{
  int32_t rc;
  if(!module) {
    C2D_ERR("failed");
    return -EINVAL;
  }
  c2d_module_ctrl_t *ctrl = (c2d_module_ctrl_t *) MCT_OBJECT_PRIVATE(module);
  if(ctrl->c2d_thread_started == TRUE) {
    C2D_ERR("failed, thread already started, "
            "can't create the thread again!");
    return -EFAULT;
  }
  ctrl->c2d_thread_started = FALSE;
  rc = pthread_create(&(ctrl->c2d_thread), NULL, c2d_thread_func, module);
  pthread_setname_np(ctrl->c2d_thread, "CAM_c2d");
  if(rc < 0) {
    C2D_ERR("pthread_create() failed, rc = %d ", rc);
    return rc;
  }
  /* wait to confirm if the thread is started */
  PTHREAD_MUTEX_LOCK(&(ctrl->c2d_mutex));
  while(ctrl->c2d_thread_started == FALSE) {
    pthread_cond_wait(&(ctrl->th_start_cond), &(ctrl->c2d_mutex));
  }
  PTHREAD_MUTEX_UNLOCK(&(ctrl->c2d_mutex));
  return 0;
}
