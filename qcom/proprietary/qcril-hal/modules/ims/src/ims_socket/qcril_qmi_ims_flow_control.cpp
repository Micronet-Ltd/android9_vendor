/******************************************************************************
#  Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_ims_flow_control.c
  @brief   IMS request flow control logic

  DESCRIPTION


******************************************************************************/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include <stdarg.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include <cutils/properties.h>

#include "telephony/ril.h"
#include "qcrili.h"
#include "qcril_log.h"
#include "qcril_other.h"
#include "qcril_reqlist.h"
#include "qcril_qmi_client.h"
#include "qcril_qmi_ims_flow_control.h"
#include "qcril_qmi_ims_radio_agent.h"
#include "qcril_qmi_ims_misc.h"
#include "qcril_qmi_ims_msg_meta.h"
#include "qcril_qmi_npb_utils.h"
#include "qcril_qmi_ims_sms.h"
#include "qcril_qmi_ims_sms_util.h"


/*===========================================================================

                   INTERNAL DEFINITIONS AND TYPES

===========================================================================*/

#define IMS_FLOW_CONTROL_LIST_LOCK() do { \
        QCRIL_LOG_INFO("LOCK IMS_FLOW_CONTROL_LIST_LOCK"); \
        pthread_mutex_lock(&qcril_ifc_info.list_mutex); \
    }while(0)

#define IMS_FLOW_CONTROL_LIST_UNLOCK() do { \
        QCRIL_LOG_INFO("UNLOCK IMS_FLOW_CONTROL_LIST_LOCK"); \
        pthread_mutex_unlock(&qcril_ifc_info.list_mutex); \
    }while(0)

#define IMS_FLOW_CONTROL_PROCESS_LOCK() do { \
        QCRIL_LOG_INFO("LOCK IMS_FLOW_CONTROL_PROCESS_LOCK"); \
        pthread_mutex_lock(&qcril_ifc_info.ims_flow_control_mutex); \
    }while(0)

#define IMS_FLOW_CONTROL_PROCESS_UNLOCK() do { \
        QCRIL_LOG_INFO("UNLOCK IMS_FLOW_CONTROL_PROCESS_LOCK"); \
        pthread_mutex_unlock(&qcril_ifc_info.ims_flow_control_mutex); \
    }while(0)

#define IMS_FLOW_CONTROL_MSG_COUNT 3

/*===========================================================================

                         LOCAL VARIABLES

===========================================================================*/
static qcril_ims_flow_control_type qcril_ifc_info;
static qcril_qmi_ims_flow_control_fw_req_overview *qcril_ims_flow_control_req_overview = NULL;


typedef struct {
    uint8_t length;
    int timer;
    qcril_qmi_ims_flow_control_fw_request_action action_on_dup_req;
    qcril_evt_e_type family_ring_table[IMS_FLOW_CONTROL_MSG_COUNT];
} ims_flow_control_msg_info_t;

// flow control table
const static ims_flow_control_msg_info_t ims_flow_control_msg_info[] = {
    { .length = 3,
      .timer = QCRIL_QMI_VOICE_DTMF_INTERVAL_VAL,
      .action_on_dup_req = QCRIL_QMI_IMS_WAIT_FOR_RESP,
      { QCRIL_EVT_IMS_SOCKET_REQ_DTMF,
        QCRIL_EVT_IMS_SOCKET_REQ_DTMF_START,
        QCRIL_EVT_IMS_SOCKET_REQ_DTMF_STOP } },
    { .length = 1,
      .timer = 0,
      .action_on_dup_req = QCRIL_QMI_IMS_WAIT_FOR_RESP,
      { QCRIL_EVT_IMS_SOCKET_REQ_SEND_SMS } },
    { .length = 1,
      .timer = 0,
      .action_on_dup_req = QCRIL_QMI_IMS_WAIT_FOR_RESP,
      { QCRIL_EVT_IMS_SOCKET_REQ_ACK_SMS } },
};

const static int ims_flow_control_groups = sizeof(ims_flow_control_msg_info) /
        sizeof(ims_flow_control_msg_info[0]);

// using property to overwrite the default time setting in the above
// flow control info table
static int user_specified_dtmf_pause_time = 0;
static int user_specified_dtmf_start_stop_interval = 0;

/*===========================================================================

                    EXTERNAL FUNCTION PROTOTYPES


===========================================================================*/



/*===========================================================================

                    INTERNAL FUNCTION PROTOTYPES


===========================================================================*/
void qcril_qmi_ims_flow_control_add_list_overview
(
qcril_qmi_ims_flow_control_fw_request_list *req_list
);

qcril_qmi_ims_flow_control_fw_request_holder *
qcril_qmi_ims_flow_control_fw_create_node
(
  RIL_Token token,
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen
);

boolean qcril_qmi_is_flow_control_required(qcril_evt_e_type event_id);
void qcril_qmi_ims_flow_control_req_timeout_handler
(
  qcril_timed_callback_handler_params_type *param
);
void qcril_qmi_ims_flow_control_get_next_req_list_entry
(
  qcril_qmi_ims_flow_control_fw_request_list *req_list,
  qcril_qmi_ims_flow_control_fw_request_list **next_req_list
);


/*===========================================================================

                                FUNCTIONS

===========================================================================*/
//////////////////////////////////////////////////////////////////////////////
//                             Main routines                                //
//////////////////////////////////////////////////////////////////////////////

//IMS flow control init routine
/***************************************************************************************************
    @function
    qcril_ims_flow_control_pre_init
***************************************************************************************************/
void qcril_ims_flow_control_pre_init()
{
  pthread_mutexattr_t attr;
  pthread_attr_t      pthread_attr;
  int                 ret;

  QCRIL_LOG_FUNC_ENTRY();

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init( &qcril_ifc_info.ims_flow_control_mutex, &attr );
  pthread_attr_init (&pthread_attr);
#ifndef QMI_RIL_UTF
   pthread_attr_setdetachstate(&pthread_attr, PTHREAD_CREATE_DETACHED);
   ret = pthread_create(&qcril_ifc_info.ims_flow_control_thread_id,
           &pthread_attr, qcril_qmi_ims_flow_control_main, NULL);
#else
   ret = utf_pthread_create_handler(&qcril_ifc_info.ims_flow_control_thread_id,
           &pthread_attr, qcril_qmi_ims_flow_control_main, NULL);
#endif

  if (ret == 0)
  {
    qmi_ril_set_thread_name(qcril_ifc_info.ims_flow_control_thread_id, QMI_RIL_IFC_THREAD_NAME);
    QCRIL_LOG_INFO("RIL IMS flow control thread started");
  }
  else
  {
    QCRIL_LOG_ERROR("Failed to start IMS flow control thread!");
    QCRIL_ASSERT(0); // this is a noop in release build
    return;
  }

  qmi_ril_get_property_value_from_integer(QCRIL_QMI_VOICE_DTMF_PAUSE_INTERVAL,
        &user_specified_dtmf_pause_time, 0);
  qmi_ril_get_property_value_from_integer(QCRIL_QMI_VOICE_DTMF_INTERVAL,
        &user_specified_dtmf_start_stop_interval, 0);

  pthread_mutexattr_destroy(&attr);

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_init_list

===========================================================================*/
/*!
    @brief
    Initializes the list

    @return
    Void
*/
/*=========================================================================*/
static void qcril_ims_flow_control_init_list
(
  qcril_ims_flow_control_list_type *list
)
{
  memset(list, 0, sizeof(qcril_ims_flow_control_list_type));
  list->next = list;
  list->prev = list;
  list->event_id = QCRIL_EVT_NONE;
} /* qcril_ims_flow_control_init_list() */

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_add_to_list

===========================================================================*/
/*!
    @brief
    Adds an entry to the list

    @return
*/
/*=========================================================================*/
static void qcril_ims_flow_control_add_to_list
(
  qcril_ims_flow_control_list_type *ev,
  qcril_ims_flow_control_list_type *list
)
{
  ev->next = list;
  ev->prev = list->prev;
  ev->prev->next = ev;
  list->prev = ev;
} /* qcril_ims_flow_control_add_to_list() */

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_remove_from_list

===========================================================================*/
/*!
    @brief
    Removes an entry from the list.

    @return
*/
/*=========================================================================*/
static void qcril_ims_flow_control_remove_from_list
(
  qcril_ims_flow_control_list_type *ev
)
{
  ev->next->prev = ev->prev;
  ev->prev->next = ev->next;
  ev->next = NULL;
  ev->prev = NULL;
} /* qcril_ims_flow_control_remove_from_list() */

static boolean qcril_qmi_ims_free_req_data
(
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen
)
{
  QCRIL_NOTUSED(datalen);
  QCRIL_LOG_FUNC_ENTRY();

  if (event_id == QCRIL_EVT_IMS_SOCKET_REQ_SEND_SMS)
  {
    freeRILImsSmsFormatPayload(static_cast<RIL_IMS_SMS_Message*>(data));
  }
  else if (event_id == QCRIL_EVT_IMS_SOCKET_REQ_ACK_SMS)
  {
    delete static_cast<QcrilImsSmsAckData*>(data);
  }
  else if (event_id == QCRIL_EVT_IMS_SOCKET_REQ_ACK_SMS_REPORT)
  {
    delete static_cast<QcrilImsSmsAckReportData*>(data);
  }
  else if (event_id == QCRIL_EVT_IMS_SOCKET_REQ_REGISTER_MULTI_IDENTITY_LINES)
  {
    delete static_cast<std::vector<QcrilImsMultiIdentityInfo>*>(data);
  }
  else if (event_id == QCRIL_EVT_IMS_SOCKET_REQ_QUERY_VIRTUAL_LINE_INFO)
  {
    delete static_cast<std::string*>(data);
  }
  else
  {
    ims_MsgId msg_id = (ims_MsgId) qcril_qmi_ims_map_event_to_request(event_id);
    QCRIL_LOG_INFO("freeing request message for %s(id: %d)",
          qcril_qmi_ims_get_msg_log_str(msg_id, ims_MsgType_REQUEST));
    const pb_field_t* pb_fields = qcril_qmi_ims_get_msg_pb_fields(msg_id,
          ims_MsgType_REQUEST);
    if (pb_fields)
    {
      qcril_qmi_npb_release(pb_fields, data);
    }
    else
    {
      QCRIL_LOG_ERROR("missing pb fields");
    }
    qcril_free(data);
  }

  return TRUE;
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_main

===========================================================================*/
/*!
    @brief
    Main loop of the thread to process events queued in ims
    flow control thread

    @return
    None
*/
/*=========================================================================*/
void *qcril_qmi_ims_flow_control_main(void *arg)
{
    qcril_ims_flow_control_list_type *ev;
    int    filedes[2];
    fd_set rfds;
    char   buff[16];
    int    go_on;
    int    ret;
    int    n;

    QCRIL_NOTUSED(arg);
    QCRIL_LOG_FUNC_ENTRY();

    /*-----------------------------------------------------------------------*/
    pthread_mutex_init(&qcril_ifc_info.ims_flow_control_mutex, NULL);
    qcril_ims_flow_control_init_list(&qcril_ifc_info.list);

    FD_ZERO(&qcril_ifc_info.readFds);

    /* Create a pipe so main thread can wake us up */
    ret = pipe(filedes);

    if (ret < 0)
    {
        QCRIL_LOG_ERROR("Error opening pipe (%d)", errno);
        qmi_ril_clear_thread_name(pthread_self());
        return NULL;
    }

    qcril_ifc_info.fdWakeupRead = filedes[0];
    qcril_ifc_info.fdWakeupWrite = filedes[1];

    fcntl(qcril_ifc_info.fdWakeupRead, F_SETFL, O_NONBLOCK);
    FD_SET(qcril_ifc_info.fdWakeupRead, &qcril_ifc_info.readFds);

    do
    {
        memcpy(&rfds, &qcril_ifc_info.readFds, sizeof(fd_set));

        QCRIL_LOG_DEBUG("Waiting...");

        /* Block waiting for a event to be put on the queue */
        n = select(qcril_ifc_info.fdWakeupRead + 1, &rfds, NULL, NULL, NULL);
        if (n < 0)
        {
            if (errno == EINTR) continue;
            QCRIL_LOG_ERROR("QCRIL event select error (%d)", errno);
            qmi_ril_clear_thread_name(pthread_self());
            return NULL;
        }

        /* Empty the socket */
        do
        {
            ret = read(qcril_ifc_info.fdWakeupRead, &buff, sizeof(buff));
            if (ret > 0)
            {
                QCRIL_LOG_DEBUG("%d items on queue", ret);
            }
        } while (ret > 0 || (ret < 0 && errno == EINTR));

        IMS_FLOW_CONTROL_LIST_LOCK();

        do
        {
            if ((NULL != (ev = qcril_ifc_info.list.next) && (ev != &qcril_ifc_info.list)))
            {
                qcril_ims_flow_control_remove_from_list(ev);

                QCRIL_LOG_DEBUG("De-queued ims flow control event %s(%d)",
                   qcril_log_lookup_event_name(ev->event_id), ev->event_id);

#ifdef QMI_RIL_UTF
                // shutdown thread upon request
                if (ev->event_id == QCRIL_EVT_IMS_SOCKET_REQ_UTF_SHUTDOWN)
                {
                  close(filedes[0]);
                  close(filedes[1]);
                  qmi_ril_clear_thread_name(pthread_self());
                  IMS_FLOW_CONTROL_LIST_UNLOCK();
                  pthread_mutex_unlock( &qcril_ifc_info.ims_flow_control_mutex );
                  pthread_exit(NULL);
                }
#endif
                IMS_FLOW_CONTROL_LIST_UNLOCK();
                switch(ev->req_type)
                {
                  case QCRIL_QMI_IMS_FLOW_CONTROL_HANDLE_REQ:
                    qcril_ims_flow_control_process_request( ev->event_id,
                                                            ev->data,
                                                            ev->datalen,
                                                            ev->t);
                    break;

                  case QCRIL_QMI_IMS_FLOW_CONTROL_REQ_COMPLETE:
                    qcril_ims_flow_control_request_complete( ev->event_id,
                                                             ev->t );
                    break;

                  case QCRIL_QMI_IMS_FLOW_CONTROL_HANDLE_NEXT_REQ:
                    qcril_ims_flow_control_handle_next_request( ev->event_id,
                                                                ev->t );
                    break;

                  case QCRIL_QMI_IMS_FLOW_CONTROL_CLEAR_LIST:
                    qcril_ims_flow_control_clear_list();
                    break;

                  default:
                    QCRIL_LOG_ERROR("Invalid req_type %d", ev->req_type);
                    break;
                }
                IMS_FLOW_CONTROL_LIST_LOCK();

                if(ev)
                {
                  if(ev->data && QCRIL_DATA_ON_STACK == ev->data_must_be_freed)
                  {
                    qcril_free(ev->data);
                  }
                  qcril_free(ev);
                }
            }
            go_on = ((NULL != (ev = qcril_ifc_info.list.next) && (ev != &qcril_ifc_info.list)));
        } while ( go_on );

        IMS_FLOW_CONTROL_LIST_UNLOCK();
    } while(1);

    qmi_ril_clear_thread_name(pthread_self());

    QCRIL_LOG_FUNC_RETURN();
    return NULL;
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_event_queue

===========================================================================*/
/*!
    @brief
    Queues an event to be dispatched in the the ims flow control thread.

    @return
    E_SUCCESS If the queue operation was successful, approprate failure code otherwise.

*/
/*=========================================================================*/
RIL_Errno qcril_qmi_ims_flow_control_event_queue
(
  qcril_qmi_ims_flow_control_req_type req_type,
  qcril_data_src_e_type data_src,
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen,
  RIL_Token t
)
{
    qcril_ims_flow_control_list_type *ev;
    RIL_Errno           result;
    int                 ret;

    /*-----------------------------------------------------------------------*/
    do
    {
        result = RIL_E_SUCCESS;
        ev = (qcril_ims_flow_control_list_type *) qcril_malloc( sizeof(qcril_ims_flow_control_list_type) );
        if (!ev) {
            result = RIL_E_GENERIC_FAILURE;
            QCRIL_LOG_ERROR("Failed to allocate memory, aborting!");
            QCRIL_ASSERT(0); // this is a noop in release build
            break;
        }

        ev->req_type = req_type;
        ev->event_id = event_id;
        ev->t = t;
        /* If the caller is passing a pointer to data on the stack,
           make a copy and free it when when we are done */
        ev->data_must_be_freed = data_src;

        if ( data == NULL )
        {
          QCRIL_LOG_DEBUG("Event [%s, %x] data was NULL", qcril_log_lookup_event_name( event_id ), event_id );
          ev->data = NULL;
          ev->datalen = 0;
        }
        else if ( data_src == QCRIL_DATA_ON_STACK )
        {
          /* Caller passed a pointer to stack data; make a copy */
          ev->data = (void*)qcril_malloc( datalen );
          if (!ev->data)
          {
            result = RIL_E_GENERIC_FAILURE;
            QCRIL_LOG_ERROR("Failed to allocate memory, aborting!");
            QCRIL_ASSERT(0); // this is a noop in release build
            break;
          }
          memcpy(ev->data, data, datalen );
          ev->datalen = datalen;
        }
        else
        {
          /* Caller passed a pointer to heap data */
          ev->data = data;
          ev->datalen = datalen;
        }

        IMS_FLOW_CONTROL_LIST_LOCK();
        qcril_ims_flow_control_add_to_list( ev, &qcril_ifc_info.list );
        IMS_FLOW_CONTROL_LIST_UNLOCK();

        if (!pthread_equal(pthread_self(), qcril_ifc_info.ims_flow_control_thread_id))
        {
            /* Wake up the IMS flow control thread. */
            do
            {
                ret = write (qcril_ifc_info.fdWakeupWrite, " ", 1);
            } while (ret < 0 && errno == EINTR);
        }
        QCRIL_LOG_DEBUG("Queued IMS event %s(0x%" PRIxPTR ")",
            qcril_log_lookup_event_name(event_id), (intptr_t)event_id);
    } while(0);

    if(RIL_E_SUCCESS != result)
    {
      if(ev)
      {
        if(ev->data && QCRIL_DATA_ON_STACK == ev->data_must_be_freed)
        {
          qcril_free(ev->data);
        }
        qcril_free(ev);
      }
    }

    return result;
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_process_request

===========================================================================*/
/*!
    @brief

    @return
    E_SUCCESS of the event was handled normally
    E_NOT_SUPPORTED if the event_id was invalid
    E_NOT_ALLOWED if the event is not supported in the current state

*/
/*=========================================================================*/
void qcril_ims_flow_control_process_request
(
  qcril_evt_e_type                    event_id,
  void                                *data,
  size_t                              datalen,
  RIL_Token                           t
)
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *orig_req_list = NULL;
  boolean flow_control_required = TRUE;

  QCRIL_LOG_FUNC_ENTRY();

  IMS_FLOW_CONTROL_PROCESS_LOCK();

  if(qcril_qmi_is_flow_control_required(event_id) == TRUE)
  {
    QCRIL_LOG_INFO("Flow control required for %s(0x%" PRIxPTR ")",
                qcril_log_lookup_event_name(event_id), (intptr_t)event_id);

    req_node = qcril_qmi_ims_flow_control_fw_create_node(t, event_id, data, datalen);
    if(req_node)
    {
      qcril_qmi_ims_flow_control_fw_check_req_from_family_ring(req_node, &orig_req_list);

      if(orig_req_list == NULL)
      {
        QCRIL_LOG_INFO("Req not found in flow control list %s(0x%" PRIxPTR ")",
                  qcril_log_lookup_event_name(event_id), (intptr_t)event_id);
        qcril_event_queue_with_data_free(
                       QCRIL_DEFAULT_INSTANCE_ID,
                       QCRIL_DEFAULT_MODEM_ID,
                       QCRIL_DATA_NOT_ON_STACK,
                       event_id, data, datalen,
                       qcril_qmi_ims_free_req_data,
                       t);

        req_node->req_state = QCRIL_QMI_IMS_REQ_STATE_IN_PROGRESS;
        //Create qcril_qmi_ims_flow_control_fw_request_list and add req_node to it
        //Add qcril_qmi_ims_flow_control_fw_request_list to overview_list
        qcril_qmi_ims_flow_control_add_req_node(req_node, &orig_req_list);
        qcril_qmi_ims_flow_control_add_list_overview(orig_req_list);
      }
      else
      {
        QCRIL_LOG_INFO("Req found in flow control list %s(0x%" PRIxPTR ")",
                  qcril_log_lookup_event_name(event_id), (intptr_t)event_id);
        switch(orig_req_list->action_on_dup_req)
        {
          case QCRIL_QMI_IMS_SEND_SUCCESS_RESP:
            QCRIL_LOG_INFO("Sending success response for duplicate request");
            imsRadioSendMessage(t, ims_MsgType_RESPONSE,
                                      qcril_qmi_ims_map_event_to_request(event_id),
                                      ims_Error_E_SUCCESS,
                                      NULL, 0);
            break;

          case QCRIL_QMI_IMS_SEND_FAILURE_RESP:
            QCRIL_LOG_INFO("Sending error response for duplicate request");
            imsRadioSendMessage(t, ims_MsgType_RESPONSE,
                                      qcril_qmi_ims_map_event_to_request(event_id),
                                      ims_Error_E_GENERIC_FAILURE,
                                      NULL, 0);
            break;

          case QCRIL_QMI_IMS_WAIT_FOR_RESP:
            QCRIL_LOG_INFO("Queue req and wait for prev request response");
            qcril_qmi_ims_flow_control_add_req_node(req_node, &orig_req_list);
            req_node->req_state = QCRIL_QMI_IMS_REQ_STATE_IN_QUEUE;
            break;

          default:
            QCRIL_LOG_ERROR("No action defined for this family ring");
            break;
        }
      }
    }
    else
    {
      QCRIL_LOG_ERROR("Failed to create node... Skip flow control...");
      flow_control_required = FALSE;
    }
  }
  else
  {
    QCRIL_LOG_INFO("Flow control not required for %s(%d)",
              qcril_log_lookup_event_name(event_id), event_id);
    flow_control_required = FALSE;
  }

  if(flow_control_required == FALSE)
  {
    qcril_event_queue_with_data_free(
                   QCRIL_DEFAULT_INSTANCE_ID,
                   QCRIL_DEFAULT_MODEM_ID,
                   QCRIL_DATA_NOT_ON_STACK,
                   event_id, data, datalen,
                   qcril_qmi_ims_free_req_data,
                   t);
  }
  IMS_FLOW_CONTROL_PROCESS_UNLOCK();

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_handle_next_request

===========================================================================*/
/*!
    @brief
    Retrieve next event queued in flow control, if any
    and process the same.

    NOTE: The token is already freed when this function is called.

    @return

*/
/*=========================================================================*/
void qcril_ims_flow_control_handle_next_request
(
  qcril_evt_e_type  event_id,
  RIL_Token         token
)
{
  qcril_qmi_ims_flow_control_fw_request_list *req_list = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *next_req_list = NULL;

  QCRIL_LOG_FUNC_ENTRY();
  IMS_FLOW_CONTROL_PROCESS_LOCK();
  req_list = qcril_qmi_ims_flow_control_get_req_list_entry(token, event_id);
  if(req_list != NULL)
  {
    QCRIL_LOG_INFO("Req found in flow control list for token 0x%" PRIxPTR, (intptr_t)token);
    qcril_qmi_ims_flow_control_get_next_req_list_entry(req_list, &next_req_list);
  }
  else
  {
    QCRIL_LOG_INFO("Req not found in flow control list for token 0x%" PRIxPTR, (intptr_t)token);
  }

  if(next_req_list)
  {
    if(next_req_list->req_node)
    {
      qcril_event_queue_with_data_free(
                     QCRIL_DEFAULT_INSTANCE_ID,
                     QCRIL_DEFAULT_MODEM_ID,
                     QCRIL_DATA_NOT_ON_STACK,
                     next_req_list->req_node->req_id,
                     next_req_list->req_node->payload,
                     next_req_list->req_node->payload_len,
                     qcril_qmi_ims_free_req_data,
                     next_req_list->req_node->token);
      next_req_list->req_node->req_state = QCRIL_QMI_IMS_REQ_STATE_IN_PROGRESS;
    }
    else
    {
      QCRIL_LOG_ERROR("req_node not found");
    }
  }
  else
  {
    QCRIL_LOG_INFO("End of the list");
  }

  IMS_FLOW_CONTROL_PROCESS_UNLOCK();

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_request_complete

===========================================================================*/
/*!
    @brief
    Retrieve next event queued in flow control, if any
    and process the same.

    @return

*/
/*=========================================================================*/
void qcril_ims_flow_control_request_complete
(
  qcril_evt_e_type  event_id,
  RIL_Token         token
)
{
  qcril_qmi_ims_flow_control_fw_request_list *req_list = NULL;
  struct timeval timeout_value = {0, 0};

  QCRIL_LOG_FUNC_ENTRY();
  IMS_FLOW_CONTROL_PROCESS_LOCK();
  req_list = qcril_qmi_ims_flow_control_get_req_list_entry(token, event_id);

  if(req_list != NULL)
  {
    if(req_list->req_node != NULL)
    {
      if(req_list->timer > QMI_RIL_ZERO)
      {
        QCRIL_LOG_INFO("Start timer of %d ms to process next request",
                                                       req_list->timer);
        timeout_value.tv_sec = QMI_RIL_ZERO;
        timeout_value.tv_usec = (req_list->timer) * 1000;
        qcril_setup_timed_callback_ex_params( QCRIL_DEFAULT_INSTANCE_ID,
                                              QCRIL_DEFAULT_MODEM_ID,
                                              qcril_qmi_ims_flow_control_req_timeout_handler,
                                              (void *)(req_list->req_node),
                                              &timeout_value,
                                              NULL);
      }
      else
      {
        IMS_FLOW_CONTROL_PROCESS_UNLOCK();
        qcril_ims_flow_control_handle_next_request(req_list->req_node->req_id,
                                                   req_list->req_node->token);
        IMS_FLOW_CONTROL_PROCESS_LOCK();
      }
    }
    else
    {
      QCRIL_LOG_ERROR("req_node is empty - Not an expected scenario");
    }
  }
  else
  {
     QCRIL_LOG_INFO("No req_list entry with this token %d", token);
  }
  IMS_FLOW_CONTROL_PROCESS_UNLOCK();
  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_req_timeout_handler

===========================================================================*/
/*!
    @brief
    Adds an entry to the list

    @return
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_req_timeout_handler
(
  qcril_timed_callback_handler_params_type *param
)
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node;

  QCRIL_LOG_FUNC_ENTRY();

  req_node = (qcril_qmi_ims_flow_control_fw_request_holder *)param->custom_param;

  qcril_qmi_ims_flow_control_event_queue(QCRIL_QMI_IMS_FLOW_CONTROL_HANDLE_NEXT_REQ,
                                         QCRIL_DATA_NOT_ON_STACK,
                                         req_node->req_id, NULL, 0,
                                         req_node->token);

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_clear_list

===========================================================================*/
/*!
    @brief
    Clear all the flow control entrie

    @return
*/
/*=========================================================================*/
void qcril_ims_flow_control_clear_list()
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *req_list = NULL;
  qcril_qmi_ims_flow_control_fw_req_overview *req_overview = NULL;

  req_overview = qcril_ims_flow_control_req_overview;

  QCRIL_LOG_FUNC_ENTRY();

  IMS_FLOW_CONTROL_PROCESS_LOCK();

  while(req_overview)
  {
    req_list = req_overview->list_head;
    while(req_list)
    {
      req_node = req_list->req_node;
      //Send Cancelled response for events in queue
      if(req_node->req_state == QCRIL_QMI_IMS_REQ_STATE_IN_QUEUE)
      {
        imsRadioSendMessageToFlowControl( req_node->token,
                        ims_MsgType_RESPONSE,
                        qcril_qmi_ims_map_event_to_request(req_node->req_id),
                        ims_Error_E_CANCELLED, NULL, 0 );
      }
      qcril_free(req_node);

      req_overview->list_head = req_list->next;
      qcril_free(req_list);

      req_list = req_overview->list_head;
    }

    req_overview = req_overview->next;
    qcril_free(qcril_ims_flow_control_req_overview);

    qcril_ims_flow_control_req_overview = req_overview;
  }

  qcril_ims_flow_control_req_overview = NULL;

  IMS_FLOW_CONTROL_PROCESS_UNLOCK();

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_ims_flow_control_add_to_list

===========================================================================*/
/*!
    @brief
    Adds an entry to the list

    @return
*/
/*=========================================================================*/
qcril_qmi_ims_flow_control_fw_request_holder *
qcril_qmi_ims_flow_control_fw_create_node
(
  RIL_Token token,
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen
)
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node = NULL;

  req_node = (qcril_qmi_ims_flow_control_fw_request_holder*)
                    qcril_malloc(sizeof(qcril_qmi_ims_flow_control_fw_request_holder));
  if(req_node)
  {
    req_node->token = token;
    req_node->req_id = event_id;
    req_node->payload = data;
    req_node->payload_len = datalen;
    req_node->req_state = QCRIL_QMI_IMS_REQ_STATE_NONE;
  }
  else
  {
    QCRIL_LOG_ERROR("Failed to create req_node");
  }

  return req_node;
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_fw_check_req_from_family_ring

===========================================================================*/
/*!
    @brief
    Check any req_list entry exists in overview list
    from event_id family ring

    @return
    qcril_qmi_ims_flow_control_fw_request_list
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_fw_check_req_from_family_ring
(
  qcril_qmi_ims_flow_control_fw_request_holder *req_node,
  qcril_qmi_ims_flow_control_fw_request_list **orig_req_list
)
{
  QCRIL_LOG_FUNC_ENTRY();

  auto req_overview = qcril_ims_flow_control_req_overview;

  if (req_overview)
  {
    bool req_found = false;
    const ims_flow_control_msg_info_t *req_grp_list = NULL;
    for (int grp_idx = 0; (grp_idx < ims_flow_control_groups &&
            !req_found); grp_idx++)
    {
      req_grp_list = &ims_flow_control_msg_info[grp_idx];
      for (int itr = 0; itr < req_grp_list->length; itr++)
      {
        if (req_node->req_id == req_grp_list->family_ring_table[itr])
        {
          req_found = true;
          break;
        }
      }
    }

    QCRIL_LOG_INFO("Check for entry in overview list now");
    if (req_found)
    {
      while (req_overview)
      {
        auto req_list = req_overview->list_head;
        if (req_list && req_list->req_node)
        {
          for (int itr=0; itr < req_grp_list->length; itr++)
          {
            if(req_list->req_node->req_id ==
                req_grp_list->family_ring_table[itr])
            {
              QCRIL_LOG_INFO("Entry found in overview list");
              *orig_req_list = req_list;
              break;
            }
          }
        }
        if (*orig_req_list) break;
        req_overview = req_overview->next;
      }
    }
    else
    {
      QCRIL_LOG_INFO("Req not found in family ring");
    }
  }
  else
  {
    QCRIL_LOG_INFO("Overview list is empty");
  }

  QCRIL_LOG_FUNC_RETURN();
}

//////////////////////////////////////////////////////////////////////////////
//                            Helper routines                               //
//////////////////////////////////////////////////////////////////////////////

/*===========================================================================

  FUNCTION:  qcril_qmi_is_flow_control_required

===========================================================================*/
/*!
    @brief
    Checks whether event_id requires flow control

    @return
*/
/*=========================================================================*/
boolean qcril_qmi_is_flow_control_required(qcril_evt_e_type event_id)
{
  boolean found = FALSE;
  for (int grp_idx = 0; (grp_idx < ims_flow_control_groups &&
        !found); grp_idx++)
  {
    for (int itr = 0; itr < ims_flow_control_msg_info[grp_idx].length; itr++)
    {
      if (event_id == ims_flow_control_msg_info[grp_idx].family_ring_table[itr])
      {
        found = TRUE;
        break;
      }
    }
  }

  return found;
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_get_req_params

===========================================================================*/
/*!
    @brief
    Read event associated family ring params

    @return
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_get_req_params
(
  qcril_evt_e_type event_id,
  qcril_qmi_ims_flow_control_fw_request_action *req_action,
  int *timer
)
{
  QCRIL_LOG_FUNC_ENTRY();

  bool req_node_found = false;
  for (int grp_idx = 0; (grp_idx < ims_flow_control_groups &&
        !req_node_found); grp_idx ++)
  {
    for (int itr = 0; itr < ims_flow_control_msg_info[grp_idx].length; itr++)
    {
      if (event_id == ims_flow_control_msg_info[grp_idx].family_ring_table[itr])
      {
        if (event_id == QCRIL_EVT_IMS_SOCKET_REQ_DTMF ||
                event_id == QCRIL_EVT_IMS_SOCKET_REQ_DTMF_STOP)
        {
            *timer = (user_specified_dtmf_pause_time) ?
                user_specified_dtmf_pause_time : ims_flow_control_msg_info[grp_idx].timer;
        }
        else if (event_id == QCRIL_EVT_IMS_SOCKET_REQ_DTMF_START)
        {
            *timer = (user_specified_dtmf_start_stop_interval) ?
                user_specified_dtmf_start_stop_interval :
                ims_flow_control_msg_info[grp_idx].timer;
        }
        else
        {
            *timer = ims_flow_control_msg_info[grp_idx].timer;
        }
        *req_action = ims_flow_control_msg_info[grp_idx].action_on_dup_req;
        QCRIL_LOG_INFO("req_action: %d, timer: %d", *req_action, *timer);
        req_node_found = true;
        break;
      }
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_add_req_node

===========================================================================*/
/*!
    @brief
    Adds request node to family rings request list

    @return
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_add_req_node
(
  qcril_qmi_ims_flow_control_fw_request_holder *req_node,
  qcril_qmi_ims_flow_control_fw_request_list **req_list
)
{
  qcril_qmi_ims_flow_control_fw_request_list *trav_req_list = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  trav_req_list = *req_list;

  if(trav_req_list)
  {
    while(trav_req_list->next)
    {
      trav_req_list = trav_req_list->next;
    }
    trav_req_list->next =
         (qcril_qmi_ims_flow_control_fw_request_list*)qcril_malloc(sizeof(qcril_qmi_ims_flow_control_fw_request_list));
    trav_req_list = trav_req_list->next;
  }
  else
  {
    //Create new req_list entry
    QCRIL_LOG_INFO("Create new req_list entry");
    trav_req_list =
         (qcril_qmi_ims_flow_control_fw_request_list*)qcril_malloc(sizeof(qcril_qmi_ims_flow_control_fw_request_list));
    *req_list = trav_req_list;
  }

  if(trav_req_list)
  {
    trav_req_list->req_node = req_node;
    qcril_qmi_ims_flow_control_get_req_params(req_node->req_id,
                                  &trav_req_list->action_on_dup_req,
                                  &trav_req_list->timer);
  }
  else
  {
    QCRIL_LOG_ERROR("Failed to create req_list entry");
  }

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_add_list_overview

===========================================================================*/
/*!
    @brief
    Add req_list to flow control overview list

    @return
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_add_list_overview
(
qcril_qmi_ims_flow_control_fw_request_list *req_list
)
{
  qcril_qmi_ims_flow_control_fw_req_overview *req_overview = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  req_overview = qcril_ims_flow_control_req_overview;

  if(req_overview)
  {
    while(req_overview->next)
    {
      req_overview = req_overview->next;
    }
    req_overview->next =
             (struct qcril_qmi_ims_flow_control_fw_req_overview*)qcril_malloc(sizeof(qcril_qmi_ims_flow_control_fw_req_overview));
    req_overview = req_overview->next;
  }
  else
  {
    QCRIL_LOG_INFO("Overview list is empty");
    req_overview =
             (qcril_qmi_ims_flow_control_fw_req_overview*)qcril_malloc(sizeof(qcril_qmi_ims_flow_control_fw_req_overview));
    qcril_ims_flow_control_req_overview = req_overview;
  }

  if(req_overview)
  {
    req_overview->list_head = req_list;
    req_overview->next = NULL;
  }
  else
  {
    QCRIL_LOG_ERROR("Failed to create req_overview");
  }
  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_get_next_req_list_entry

===========================================================================*/
/*!
    @brief
    Add req_list to flow control overview list

    @return
*/
/*=========================================================================*/
void qcril_qmi_ims_flow_control_get_next_req_list_entry
(
  qcril_qmi_ims_flow_control_fw_request_list *req_list,
  qcril_qmi_ims_flow_control_fw_request_list **next_req_list
)
{
  qcril_qmi_ims_flow_control_fw_req_overview *req_overview = NULL;
  qcril_qmi_ims_flow_control_fw_req_overview *prev_req_overview = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *req_list_trav = NULL;

  QCRIL_LOG_FUNC_ENTRY();

  req_overview = qcril_ims_flow_control_req_overview;
  prev_req_overview = qcril_ims_flow_control_req_overview;

  do
  {
    while(req_overview && (req_list != req_overview->list_head))
    {
      prev_req_overview = req_overview;
      req_overview = req_overview->next;
    }

    if(req_overview)
    {
      req_list_trav = req_overview->list_head;
      req_overview->list_head = req_overview->list_head->next;
      *next_req_list = req_overview->list_head;
      if(*next_req_list && (*next_req_list)->req_node)
      {
        QCRIL_LOG_INFO("req_id: %s(%d), payload 0x%x payload_len %d",
                     qcril_log_lookup_event_name((*next_req_list)->req_node->req_id),
                     (*next_req_list)->req_node->req_id,
                     (*next_req_list)->req_node->payload,
                     (*next_req_list)->req_node->payload_len);
      }

      qcril_free(req_list_trav->req_node);
      qcril_free(req_list_trav);

      if(req_overview->list_head == NULL)
      {
        if (req_overview == qcril_ims_flow_control_req_overview) {
          //req_overview is a head node
          qcril_ims_flow_control_req_overview = req_overview->next;
          qcril_free(req_overview);
          req_overview = NULL;
        } else {
          //req_overview is not a head node
          prev_req_overview->next = req_overview->next;
          qcril_free(req_overview);
          req_overview = NULL;
        }
      }
    }
    else
    {
      QCRIL_LOG_ERROR("Overview list not found");
      break;
    }
  } while(FALSE);

  QCRIL_LOG_FUNC_RETURN();
}

/*===========================================================================

  FUNCTION:  qcril_qmi_ims_flow_control_get_req_list_entry

===========================================================================*/
/*!
    @brief
    Find req_list entry based on RIL request token

    @return
    qcril_qmi_ims_flow_control_fw_request_list
*/
/*=========================================================================*/
qcril_qmi_ims_flow_control_fw_request_list *
qcril_qmi_ims_flow_control_get_req_list_entry
(
  RIL_Token         token,
  qcril_evt_e_type  req_id
)
{
  boolean req_list_found = FALSE;
  qcril_qmi_ims_flow_control_fw_req_overview *req_overview = NULL;
  qcril_qmi_ims_flow_control_fw_request_list *req_list = NULL;

  req_overview = qcril_ims_flow_control_req_overview;

  while(req_overview)
  {
    req_list = req_overview->list_head;
    while(req_list)
    {
       if(token == req_list->req_node->token &&
          req_id == req_list->req_node->req_id)
       {
           req_list_found = TRUE;
           break;
       }
       req_list = req_list->next;
    }
    if(req_list_found) break;
    req_overview = req_overview->next;
  }

  if(!req_list_found)
  {
    QCRIL_LOG_INFO("Not found req_list entry with token %d event %d",token,req_id);
  }

  return req_list;
}
