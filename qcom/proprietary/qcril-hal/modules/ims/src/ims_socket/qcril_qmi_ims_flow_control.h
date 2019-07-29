/******************************************************************************
#  Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/*!
  @file
  qcril_qmi_ims_flow_control.h

  @brief

*/


#ifndef QCRIL_QMI_IMS_FLOW_CONTROL_H
#define QCRIL_QMI_IMS_FLOW_CONTROL_H

#include <framework/legacy.h>
#include "qcrili.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "telephony/ril.h"
#ifdef __cplusplus
#include "qcril_qmi_voice.h"
#endif

typedef enum qcril_qmi_ims_flow_control_req_type
{
  QCRIL_QMI_IMS_FLOW_CONTROL_HANDLE_REQ = 0,
  QCRIL_QMI_IMS_FLOW_CONTROL_REQ_COMPLETE,
  QCRIL_QMI_IMS_FLOW_CONTROL_HANDLE_NEXT_REQ,
  QCRIL_QMI_IMS_FLOW_CONTROL_CLEAR_LIST
}qcril_qmi_ims_flow_control_req_type;

typedef struct qcril_ims_flow_control_list_struct
{
  struct qcril_ims_flow_control_list_struct *next;
  struct qcril_ims_flow_control_list_struct *prev;
  boolean data_must_be_freed;
  qcril_qmi_ims_flow_control_req_type       req_type;
  qcril_evt_e_type                          event_id;
  void                                      *data;
  size_t                                    datalen;
  RIL_Token                                 t;
} qcril_ims_flow_control_list_type;

typedef struct
{
    pthread_t                         ims_flow_control_thread_id;
    pthread_mutex_t                   ims_flow_control_mutex;
    pthread_mutex_t                   list_mutex;
    qcril_ims_flow_control_list_type  list;
    fd_set                            readFds;
    int                               fdWakeupRead;
    int                               fdWakeupWrite;
} qcril_ims_flow_control_type;

typedef enum qcril_qmi_ims_flow_control_fw_request_state
{
  QCRIL_QMI_IMS_REQ_STATE_NONE = 0,
  QCRIL_QMI_IMS_REQ_STATE_IN_PROGRESS, //request already posted to event queue
  QCRIL_QMI_IMS_REQ_STATE_IN_QUEUE,    //request waiting in flow control queue
}qcril_qmi_ims_flow_control_fw_request_state;

//Structure for storing req contents
typedef struct qcril_qmi_ims_flow_control_fw_request_holder
{
  RIL_Token token;        //Android token
  qcril_evt_e_type       req_id;       //Android request
  void      *payload;     //Android request payload
  size_t    payload_len;
  qcril_qmi_ims_flow_control_fw_request_state  req_state;         //Request state
}qcril_qmi_ims_flow_control_fw_request_holder;

typedef enum qcril_qmi_ims_flow_control_fw_request_action
{
  QCRIL_QMI_IMS_NONE = 0,
  QCRIL_QMI_IMS_SEND_SUCCESS_RESP, //Immediately send success resp
  QCRIL_QMI_IMS_SEND_FAILURE_RESP, //Immediately send failure resp
  QCRIL_QMI_IMS_WAIT_FOR_RESP      //Wait for the response of pending request
}qcril_qmi_ims_flow_control_fw_request_action;

//Structure to hold requests from same group
typedef struct qcril_qmi_ims_flow_control_fw_request_list
{
  qcril_qmi_ims_flow_control_fw_request_holder *req_node;         //Request received fro processing
  qcril_qmi_ims_flow_control_fw_request_action action_on_dup_req; //Action to be taken when a duplicate req received
  int timer;                                                      //Timer to wait for processing next req after receiving the resp for this req
  struct qcril_qmi_ims_flow_control_fw_request_list *next;        //Next req to be processed when resp recieved for this request
}qcril_qmi_ims_flow_control_fw_request_list;

//Structure to hold requests from different groups
typedef struct qcril_qmi_ims_flow_control_fw_req_overview
{
  int req_token;                                          //Internal token for search purpose
  qcril_qmi_ims_flow_control_fw_request_list *list_head;  //Request list for different kind of groups
  struct qcril_qmi_ims_flow_control_fw_req_overview *next;
}qcril_qmi_ims_flow_control_fw_req_overview;

//IMS flow control init routine
void qcril_ims_flow_control_pre_init();

//Extern routine to queue request recieved on IMS socket
RIL_Errno qcril_qmi_ims_flow_control_event_queue
(
  qcril_qmi_ims_flow_control_req_type req_type,
  qcril_data_src_e_type data_src,
  qcril_evt_e_type event_id,
  void *data,
  size_t datalen,
  RIL_Token t
);

//routine to process queued flow control requests
void *qcril_qmi_ims_flow_control_main(void *);

void qcril_ims_flow_control_process_request
(
  qcril_evt_e_type   event_id,
  void               *data,
  size_t             datalen,
  RIL_Token          t
);

void qcril_ims_flow_control_request_complete
(
  qcril_evt_e_type   event_id,
  RIL_Token          token
);

void qcril_ims_flow_control_handle_next_request
(
  qcril_evt_e_type   event_id,
  RIL_Token          token
);

void qcril_ims_flow_control_clear_list();

//IMS flow control request holder node
qcril_qmi_ims_flow_control_fw_request_holder *qcril_qmi_ims_flow_control_fw_create_node();

//Check for similar req exists or not
void qcril_qmi_ims_flow_control_fw_check_req_from_family_ring
(
  qcril_qmi_ims_flow_control_fw_request_holder *req_node,
  qcril_qmi_ims_flow_control_fw_request_list **orig_req_list
);

//Queue current request to flow control fw
void qcril_qmi_ims_flow_control_add_req_node
(
  qcril_qmi_ims_flow_control_fw_request_holder *req_node,
  qcril_qmi_ims_flow_control_fw_request_list **req_list_head
);

qcril_qmi_ims_flow_control_fw_request_list *
qcril_qmi_ims_flow_control_get_req_list_entry
(
  RIL_Token         token,
  qcril_evt_e_type  req_id
);

#ifdef __cplusplus
}
#endif
#endif /* QCRIL_QMI_IMS_FLOW_CONTROL_H */
