/* afd_thread.c
 *
 * Copyright (c) 2013-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */
#include <pthread.h>
#include "mct_queue.h"
#include "afd_thread.h"
#include "afd_port.h"
#include "camera_dbg.h"


/** afd_thread_free_msg
 *    @msg_pp: Address to the pointer of the msg to free
 *
 *  Free the msg and set it to NULL.
 *  This function free all structures related the the msg, including non-flat
 *  structures that have been allocated by the caller.
 *
 *  Return void
 **/
void afd_thread_free_msg(afd_thread_msg_t **msg_pp)
{
  afd_thread_msg_t *msg = *msg_pp;
  if (NULL == msg) {
    return;
  }

  /* Stats messages are not synced, so we have to free the payload here */
  switch (msg->type) {
    case MSG_AFD_STATS: {
      /* enq-msg has struct of pointers to stats buffer. Free up the memory
       * allocated for struct of pointers.
       * enq-msg carrying this pointer's struct will be deallocated just before
       * exiting this function
       * No need to free the actual stats buffers allocated by ISP. That will be
       * handled in circular_stats_data_ack call */
      free(msg->u.stats);
      msg->u.stats = NULL;
    }
      break;
    case MSG_AFD_SET: {
    /* placeholder to free any additional memory allocated by set-param */
      switch (msg->u.afd_set_parm.type) {
        case AFD_SET_PARAM_INIT_CHROMATIX:
        case AFD_SET_AEC_PARAM:
        case AFD_SET_SENSOR_PARAM:
        case AFD_SET_AF_PARAM:
        case AFD_SET_ENABLE:
        case AFD_SET_RESET:
        case AFD_SET_STATS_DEBUG_MASK:
        case AFD_SET_SOF:
        default:
          break;
      }
    }
      break;
    default:
      break;
  }
  free(msg);
  *msg_pp = NULL;
}

/** afd_thread_init
 *
 *  Initialize afd thread
 **/
afd_thread_data_t* afd_thread_init(void)
{
  afd_thread_data_t *thread_data;

  thread_data = calloc(1, sizeof(afd_thread_data_t));
  if (thread_data == NULL)
    return NULL;

  thread_data->msg_q = (mct_queue_t *)mct_queue_new;
  if (!thread_data->msg_q) {
    free(thread_data);
    thread_data = NULL;
    return NULL;
  }

  pthread_mutex_init(&thread_data->msg_q_lock, NULL);
  mct_queue_init(thread_data->msg_q);
  pthread_cond_init(&(thread_data->thread_cond), NULL);
  pthread_mutex_init(&(thread_data->thread_mutex), NULL);
  sem_init(&thread_data->sem_launch, 0, 0);
  return thread_data;
}

/** afd_thread_deinit
 *    @p: mct_port_t object
 *
 *  deinitialize AFD thread related resources
 *
 *  Return: No
 **/
void afd_thread_deinit(void *p)
{
  afd_thread_data_t   *thread_data;
  afd_port_private_t  *private;
  mct_port_t *port    = (mct_port_t *)p;

  private = (afd_port_private_t *)port->port_private;
  if (!private) {
    AFD_ERR("port private is NULL");
    return;
  }
  AFD_LOW("thread_data: %p", private->thread_data);
  thread_data = private->thread_data;
  pthread_mutex_destroy(&thread_data->thread_mutex);
  pthread_cond_destroy(&thread_data->thread_cond);
  mct_queue_free(thread_data->msg_q);
  pthread_mutex_destroy(&thread_data->msg_q_lock);
  sem_destroy(&thread_data->sem_launch);
  free(thread_data);
  thread_data = NULL;
  private->thread_data = NULL;
}

/**
 *
 **/
boolean afd_thread_en_q_msg(void *afd_data,
  afd_thread_msg_t  *msg)
{
  afd_thread_data_t *thread_data = (afd_thread_data_t *)afd_data;
  boolean rc = FALSE;
  boolean        sync_flag_set = FALSE;
  afd_msg_sync_t msg_sync;

  pthread_mutex_lock(&thread_data->msg_q_lock);
  sync_flag_set = msg->sync_flag;
  if (thread_data->active &&
      !(msg->type == MSG_AFD_STATS && thread_data->no_stats_mode)) {

    if (TRUE == sync_flag_set) {
      msg->sync_obj = &msg_sync;
      sem_init(&msg_sync.msg_sem, 0, 0);
      sync_flag_set = TRUE;
    }

    mct_queue_push_tail(thread_data->msg_q, msg);
    if (msg->type == MSG_STOP_AFD_THREAD) {
      thread_data->active = 0;
    }
    rc = TRUE;
  }
  pthread_mutex_unlock(&thread_data->msg_q_lock);

  if (rc) {
    pthread_mutex_lock(&thread_data->thread_mutex);
    pthread_cond_signal(&thread_data->thread_cond);
    pthread_mutex_unlock(&thread_data->thread_mutex);

    if (TRUE == sync_flag_set) {
      sem_wait(&msg_sync.msg_sem);
      sem_destroy(&msg_sync.msg_sem);
    }
  } else {
    AFD_HIGH("AFD thread_data not active: %d", thread_data->active);
    /* Only free if sync flag is not set, caller must free */
    if (FALSE == sync_flag_set) {
      afd_thread_free_msg(&msg);
    }
  }

  return rc;
}

/** afd_thread_handler:
 *    @port_info: pointer to afd port.
 **/
static void* afd_thread_handler(void *port_info)
{
  afd_port_private_t  *private;
  afd_thread_msg_t    *msg = NULL;
  afd_thread_data_t   *thread_data;
  afd_module_object_t *afd_obj;
  mct_port_t *port = (mct_port_t *)port_info;
  int exit_flag = 0;
  private = (afd_port_private_t *)port->port_private;
  if (!private)
    return NULL;

  sem_post(&private->thread_data->sem_launch);

  thread_data = private->thread_data;
  afd_obj     = &(private->afd_object);

  do {
    pthread_mutex_lock(&thread_data->thread_mutex);
    while (thread_data->msg_q->length == 0) {
        pthread_cond_wait(&thread_data->thread_cond,
          &thread_data->thread_mutex);
    }
    pthread_mutex_unlock(&thread_data->thread_mutex);

    /* Get the message */
    pthread_mutex_lock(&thread_data->msg_q_lock);
    msg = (afd_thread_msg_t *)
      mct_queue_pop_head(thread_data->msg_q);
    pthread_mutex_unlock(&thread_data->msg_q_lock);

    if (!msg) {
      continue;
    }

    if(private->thread_data->active == 0) {
      if(msg->type != MSG_STOP_AFD_THREAD) {
        if (msg->sync_flag == TRUE) {
          sem_post(&msg->sync_obj->msg_sem);
          /* Don't free msg, the sender will do */
          msg = NULL;
        } else {
          if (msg->type == MSG_AFD_STATS && msg->u.stats) {
            /* ACK the unused the STATS buffer from ISP */
            afd_obj->afd_stats_cb(port, msg->u.stats);
          }
          afd_thread_free_msg(&msg);
        }
        continue;
      }
    }

    AFD_LOW("got event msgtype %d", msg->type);
    /* Process message accordingly */
    switch (msg->type) {
    case MSG_AFD_SET:
      AFD_LOW("got set evt fn  %p", afd_obj->set_parameters);
      if (afd_obj->set_parameters) {
        int rc;
        rc = afd_obj->set_parameters(&msg->u.afd_set_parm, afd_obj->afd,
          &(afd_obj->output));
        if (rc > 1) {
          afd_obj->output.type = AFD_CB_OUTPUT;
          afd_obj->afd_cb(&(afd_obj->output), port);
        }
      }
      break;

    case MSG_STOP_AFD_THREAD:
      exit_flag = 1;
      break;
    case MSG_AFD_STATS: {
      boolean rc = TRUE;
      if (!thread_data->no_stats_mode) {
        ATRACE_BEGIN("Camera:AFD");
        rc = afd_obj->process(msg->u.stats, afd_obj->afd,
          &(afd_obj->output));
        ATRACE_END();
      } else {
        AFD_LOW("no_stats_mode");
      }

      if (rc == TRUE) {
        afd_obj->output.type = AFD_CB_OUTPUT;
        afd_obj->afd_cb(&(afd_obj->output), port);
      }
      afd_obj->afd_stats_cb(port, msg->u.stats);
    }
      break;
    case MSG_AFD_GET: {
      if (afd_obj->get_parameters) {
        int rc;
        rc = afd_obj->get_parameters(&msg->u.afd_get_parm, afd_obj->afd);
        if (rc) {
          afd_output_data_t        output;
          output.type = AFD_CB_STATS_CONFIG;
          output.max_algo_hnum = msg->u.afd_get_parm.stats_hnum;
          output.max_algo_vnum = msg->u.afd_get_parm.stats_vnum;
          afd_obj->afd_cb(&output, port);
        }
      }
    }
    break;
    default:
      break;
    }

    if (msg->sync_flag == TRUE) {
      sem_post(&msg->sync_obj->msg_sem);
      /*don't free msg, the sender will do*/
      msg = NULL;
    } else {
      afd_thread_free_msg(&msg);
    }
  } while (!exit_flag);

  return NULL;
}

/** afd_thread_start
 *
 **/
boolean afd_thread_start(void *p)
{
  pthread_t id;
  afd_port_private_t  *private;
  afd_thread_data_t   *thread_data;
  mct_port_t *port    = (mct_port_t *)p;
  int32_t ret;
  ret = pthread_create(&id, NULL, afd_thread_handler, (void *)port);
  if (ret < 0) {
    AFD_ERR("Failed to create afd thread");
    return FALSE;
  }
  pthread_setname_np(id, "CAM_AFD");
  private = (afd_port_private_t *)port->port_private;
  if (!private)
    return FALSE;
  thread_data = private->thread_data;
  thread_data->thread_id = id;
  thread_data->active = 1;
  return TRUE;
}

/**
 *
 **/
boolean afd_thread_stop(afd_thread_data_t   *thread_data)
{
  boolean rc ;

  sem_wait(&thread_data->sem_launch);

  afd_thread_msg_t *msg = malloc(sizeof(afd_thread_msg_t));

  if (msg) {
    AFD_LOW("MSG_STOP_AFD_THREAD");
    memset(msg, 0, sizeof(afd_thread_msg_t));
    msg->type = MSG_STOP_AFD_THREAD;
    rc = afd_thread_en_q_msg(thread_data,msg);
    if (rc) {
      pthread_join(thread_data->thread_id, NULL);
      AFD_LOW("pthread_join");
    }
  } else {
    rc = FALSE;
  }
  return rc;
}

