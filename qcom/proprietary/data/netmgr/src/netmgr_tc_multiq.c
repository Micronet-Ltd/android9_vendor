/******************************************************************************

                      N E T M G R _ T C _ M U L T I Q. C

******************************************************************************/

/******************************************************************************

  @file    netmgr_tc.c
  @brief   Network Manager traffic control multiq

  DESCRIPTION
  Implementation of NetMgr Linux traffic control multiq module.

******************************************************************************/
/*===========================================================================

  Copyright (c) 2010-2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#ifdef NETMGR_QOS_ENABLED

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>
#include "ds_list.h"
#include "ds_string.h"
#include "netmgr_defs.h"
#include "netmgr_platform.h"
#include "netmgr_util.h"
#include "netmgr_tc_i.h"
#include "netmgr_cb.h"
#include "netmgr_qmi_cb.h"
#include "netmgr_tc_multiq.h"

/*===========================================================================
                     LOCAL DEFINITIONS AND DECLARATIONS
===========================================================================*/

#define NETMGR_TC_DEFAULT_MULTIQ_NUM 0
#define NETMGR_TC_DEFAULT_ACK_MULTIQ_NUM 1

/*===========================================================================
                            FORWARD DECLARATIONS
===========================================================================*/


/*===========================================================================
                            LOCAL FUNCTION DEFINITIONS
===========================================================================*/

/*===========================================================================
  FUNCTION netmgr_tc_assign_num_multiq
===========================================================================*/
/*!
@brief
  Assigne a multiq number starting from 1

@return
  The assigned multiq number. 0 if not assigned.

@note
*/
/*=========================================================================*/
LOCAL uint32_t
netmgr_tc_assign_num_multiq
(
  int                    link,
  netmgr_tc_flow_info_t *flow_info
)
{
  uint32_t n = NETMGR_TC_DEFAULT_MULTIQ_NUM;
  uint32_t i;

  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);
  NETMGR_ASSERT(flow_info);

  if (NETMGR_IS_DEFAULT_FLOW(flow_info->qos_flow.flow_id))
  {
    return NETMGR_TC_DEFAULT_MULTIQ_NUM;
  }

  for (i = 0; i < NETMGR_TC_MAX_MULTIQ_NUM; i++)
  {
    if (i == NETMGR_TC_DEFAULT_MULTIQ_NUM ||
        i == NETMGR_TC_DEFAULT_ACK_MULTIQ_NUM)
    {
      continue;
    }

    /* Find a match */
    if (netmgr_tc_cfg.links[link].mq_info[i].ref &&
        netmgr_tc_cfg.links[link].mq_info[i].bearer_id ==
                   flow_info->qos_flow.bearer_id)
    {
      netmgr_tc_cfg.links[link].mq_info[i].ref++;
      n = i;
      goto out;
    }

    /* Find an empty slot */
    if (n == NETMGR_TC_DEFAULT_MULTIQ_NUM &&
        !netmgr_tc_cfg.links[link].mq_info[i].ref)
    {
      n = i;
    }
  }

  if (n != NETMGR_TC_DEFAULT_MULTIQ_NUM)
  {
    netmgr_tc_cfg.links[link].mq_info[n].bearer_id =
                        flow_info->qos_flow.bearer_id;
    netmgr_tc_cfg.links[link].mq_info[n].ref = 1;
  }

out:
  netmgr_log_high("Assigned multiq num %u for bearer %u flow %u",
                  n, flow_info->qos_flow.bearer_id,
                  flow_info->qos_flow.flow_id);
  return n;
}

/*===========================================================================
  FUNCTION netmgr_tc_release_num_multiq
===========================================================================*/
/*!
@brief
  Release a multiq number

@return
  None

@note
*/
/*=========================================================================*/
LOCAL void
netmgr_tc_release_num_multiq
(
  int                    link,
  netmgr_tc_flow_info_t *flow_info
)
{
  uint32_t i;

  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);
  NETMGR_ASSERT(flow_info);

  if (NETMGR_IS_DEFAULT_FLOW(flow_info->qos_flow.flow_id))
  {
    return;
  }

  for (i = 0; i < NETMGR_TC_MAX_MULTIQ_NUM; i++)
  {
    if (i == NETMGR_TC_DEFAULT_MULTIQ_NUM ||
        i == NETMGR_TC_DEFAULT_ACK_MULTIQ_NUM)
    {
      continue;
    }

    if (netmgr_tc_cfg.links[link].mq_info[i].ref &&
        netmgr_tc_cfg.links[link].mq_info[i].bearer_id ==
                   flow_info->qos_flow.bearer_id)
    {
      netmgr_log_high("Released multiq num %u for bearer %u flow %u",
                      i, flow_info->qos_flow.bearer_id,
                      flow_info->qos_flow.flow_id);
      netmgr_tc_cfg.links[link].mq_info[i].ref--;
      return;
    }
  }

  netmgr_log_err("Multiq num for bearer %u not found",
                 flow_info->qos_flow.bearer_id);
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_2nd_default_flow_multiq
===========================================================================*/
/*!
@brief
  Create second default flow in case V4V6 have different bearers

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int netmgr_tc_create_2nd_default_flow_multiq
(
  int link,
  netmgr_tc_flow_info_t   *flow_info,
  netmgr_tc_handle_info_t *class_handle
)
{
  /* Sanity checks */
  NETMGR_ASSERT(flow_info && class_handle);
  NETMGR_ASSERT(NETMGR_IS_DEFAULT_FLOW(flow_info->qos_flow.flow_id));

  /* Create 2nd child class */
  class_handle->major = NETMGR_TC_QDISC_ROOT_MAJOR;
  NETMGET_TC_ASSIGN_CLASS_MINOR_ID(link, class_handle->minor);
  class_handle->handle = NETMGR_TC_DEFAULT_MULTIQ_NUM;

  /* Hook up the flow */
  ds_dll_enq(netmgr_tc_cfg.links[link].flow_list, NULL, (void*)flow_info);

  netmgr_tc_cfg.links[link].default_class2 = class_handle;

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_root_qdisc_multiq
===========================================================================*/
/*!
@brief
  Create root queue discipline for specified link.
  The HTB qdisc is used for shaping and scheduling capabilities.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_create_root_qdisc_multiq
(
  int                       link,
  netmgr_tc_handle_info_t * class_handle
)
{
  netmgr_tc_handle_info_t * root_handle = NULL;

  (void)class_handle;

  NETMGR_LOG_FUNC_ENTRY;

  /* Configure the object handles */
  root_handle = netmgr_tc_handle_info_alloc();
  if( !root_handle )
  {
    netmgr_log_err("failed to allocate root handle for link %d", link);
    goto error;
  }

  root_handle->major = NETMGR_TC_QDISC_ROOT_MAJOR;
  root_handle->minor = 0;

  netmgr_tc_cfg.links[link].root_qdisc = root_handle;

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  netmgr_free( root_handle );
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_create_tcp_ack_class_multiq
===========================================================================*/
/*!
@brief
 Create TCP ack class for a flow.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_create_tcp_ack_class_multiq
(
  int                      link,
  netmgr_tc_flow_info_t  * flow_info
)
{
  netmgr_tc_handle_info_t * tcp_ack_class_handle = NULL;
  int result = NETMGR_FAILURE;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(link >= 0 && link < NETMGR_MAX_LINK);
  NETMGR_ASSERT(flow_info);

  /* Create child class for TCP ack flow */
  tcp_ack_class_handle = netmgr_tc_handle_info_alloc();
  if (!tcp_ack_class_handle)
  {
    netmgr_log_err("failed to allocate tcp ack class handle for flow %u",
                        flow_info->qos_flow.flow_id);
    goto bail;
  }

  tcp_ack_class_handle->major = NETMGR_TC_QDISC_ROOT_MAJOR;
  NETMGET_TC_ASSIGN_CLASS_MINOR_ID(link, tcp_ack_class_handle->minor);
  tcp_ack_class_handle->handle = NETMGR_TC_DEFAULT_ACK_MULTIQ_NUM;

  flow_info->tcp_ack_class = tcp_ack_class_handle;
  result = NETMGR_SUCCESS;

bail:
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_create_multiq
===========================================================================*/
/*!
@brief
 Create flow object and install the traffic control elements.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
LOCAL int
netmgr_tc_flow_create_multiq
(
  int                                link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  int result;
  netmgr_tc_flow_info_t * flow_info = NULL;
  netmgr_tc_handle_info_t * class_handle = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT( qos_flow );

  /* Allocate a flow info buffer */
  flow_info = netmgr_tc_flow_alloc();
  NETMGR_ASSERT( flow_info );
  memset(flow_info, 0x0, sizeof(netmgr_tc_flow_info_t));

  /* Assign flow attributes */
  flow_info->phys_net_dev = netmgr_main_cfg.phys_net_dev;
  flow_info->link_name = netmgr_kif_get_name(link);
  flow_info->qos_flow = *qos_flow;

  /* Allocate handle for classes */
  class_handle = netmgr_tc_handle_info_alloc();
  if( !class_handle )
  {
    netmgr_log_err("failed to allocate class handle for flow 0x%lx", (unsigned long)qos_flow->flow_id);
    goto error;
  }

  netmgr_tc_flow_filter_dump(qos_flow);

  /* Check for primary flow */
  if( NETMGR_IS_DEFAULT_FLOW( qos_flow->flow_id ) )
  {
    if (netmgr_tc_cfg.links[link].root_qdisc != NULL)
    {
      result = netmgr_tc_create_2nd_default_flow_multiq(link, flow_info, class_handle);
      if (result == NETMGR_FAILURE)
      {
        netmgr_log_err("failed to create 2nd tc default flow");
        goto error;
      }
      else
      {
        netmgr_log_high("created 2nd tc default flow");
        goto out;
      }
    }

    /* Install kernel traffic control elements for primary flow */
    /* Create root HTB qdisc */
    result = netmgr_tc_create_root_qdisc_multiq(link, NULL);

    if (result == NETMGR_FAILURE)
    {
      netmgr_log_err("error creating root qdisc for flow 0x%lx", (unsigned long)qos_flow->flow_id);
      goto error;
    }

    /* Create child class for default flow */
    class_handle->major = NETMGR_TC_QDISC_ROOT_MAJOR;
    NETMGET_TC_ASSIGN_CLASS_MINOR_ID(link, class_handle->minor);
    class_handle->handle = NETMGR_TC_DEFAULT_MULTIQ_NUM;

    netmgr_tc_cfg.links[link].root_class = NULL;
    netmgr_tc_cfg.links[link].default_class = class_handle;
    flow_info->class_handle = class_handle;

    /* We don't need filter, driver should map default to multiq num 0 */

    /* Add flow object to list for this link */
    flow_info->qos_flow.state = NETMGR_TC_FLOW_ACTIVE;
    ds_dll_enq( netmgr_tc_cfg.links[link].flow_list, NULL, (void*)flow_info );

    /* Primary flow does not have classifier/filter, relies only on
       root qdisc default class assignment. */
  }
  else
  {
    /* Check whether root class is non-null */
    if( NULL == netmgr_tc_cfg.links[link].root_qdisc )
    {
      netmgr_log_err("Root class is NULL for flow 0x%lx", (unsigned long)qos_flow->flow_id);
      goto error;
    }

    /* Install kernel traffic control elements for secondary flow */
    class_handle->major = NETMGR_TC_QDISC_ROOT_MAJOR;
    NETMGET_TC_ASSIGN_CLASS_MINOR_ID(link, class_handle->minor);
    class_handle->handle = netmgr_tc_assign_num_multiq(link, flow_info);

    flow_info->class_handle = class_handle;

    netmgr_log_high("num_filters: %d",qos_flow->num_filter);

    /* Mark all matching packets with the value of the link */
    if (NETMGR_SUCCESS != netmgr_tc_create_flow_chain(link, qos_flow->flow_id,
                                    qos_flow->filter_list[0].qos_filter.ip_version))
    {
      netmgr_tc_release_num_multiq(link, flow_info);
      flow_info->class_handle = NULL;
      goto error;
    }
    /* Link the chain into the mangle table */
    if (netmgr_tc_flow_enable( link,flow_info ) != NETMGR_SUCCESS)
    {
      netmgr_tc_release_num_multiq(link, flow_info);
      flow_info->class_handle = NULL;
      goto error;
    }

    /* Add flow object to list for this link only if the flow is enabled successfully*/
    ds_dll_enq( netmgr_tc_cfg.links[link].flow_list, NULL, (void*)flow_info );
  }

out:
  if (netmgr_main_get_tcpackprio_enabled() &&
      NETMGR_IS_DEFAULT_FLOW(flow_info->qos_flow.flow_id))
  {
    /* Create ack class only for default flows */
    netmgr_tc_create_tcp_ack_class_multiq(link, flow_info);
  }
  netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_ACTIVATE, flow_info);

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  netmgr_tc_flow_free( &flow_info );
  netmgr_free( class_handle );
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_is_default_flow_activated_multiq
===========================================================================*/
/*!
@brief
  Check if default flow is already activated.

@return
  TRUE/FALSE

@note
*/
/*=========================================================================*/
boolean
netmgr_tc_is_default_flow_activated_multiq
(
  int                                link,
  const netmgr_qmi_qos_flow_info_t * qos_flow
)
{
  netmgr_tc_flow_info_t * flow_info = NULL;
  netmgr_tc_flow_info_t * new_flow_info = NULL;
  netmgr_tc_handle_info_t * new_class_handle = NULL;

  NETMGR_ASSERT(qos_flow &&
                NETMGR_IS_DEFAULT_FLOW(qos_flow->flow_id));

  flow_info = netmgr_tc_find_flow_info(
                 link, qos_flow->flow_id, qos_flow->ip_ver);

  if (flow_info != NULL)
  {
    /* Bearer id could have been changed if one of dual-ip sub-call
     * is disconnect and reconnected */
    netmgr_log_high("Default flow (bearer_id %d) ip %d found\n",
                         flow_info->qos_flow.bearer_id,
                         flow_info->qos_flow.ip_ver);

    if (flow_info->qos_flow.bearer_id != qos_flow->bearer_id)
    {
      netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_DELETE, flow_info);
      flow_info->qos_flow.bearer_id = qos_flow->bearer_id;
      netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_ACTIVATE, flow_info);
    }

    return TRUE;
  }
  else
  {
    flow_info = netmgr_tc_find_flow_from_bearer(link, qos_flow->bearer_id);
    if (flow_info)
    {
      /* Dual-IP calls have the same bearer -- use same TC class */
      netmgr_log_high("Flow with the same bearer_id (flow 0x%d) ip %d found\n",
                         flow_info->qos_flow.flow_id,
                         flow_info->qos_flow.ip_ver);

      new_flow_info = netmgr_tc_flow_alloc();
      new_class_handle = netmgr_tc_handle_info_alloc();
      NETMGR_ASSERT(new_flow_info && new_class_handle);

      memset(new_flow_info, 0, sizeof(netmgr_tc_flow_info_t));
      memcpy(new_class_handle, flow_info->class_handle,
                           sizeof(netmgr_tc_handle_info_t));

      new_flow_info->phys_net_dev = flow_info->phys_net_dev;
      new_flow_info->link_name = flow_info->link_name;
      new_flow_info->qos_flow = *qos_flow;
      new_flow_info->class_handle = new_class_handle;
      new_flow_info->tcp_ack_class = NULL;

      if (netmgr_main_get_tcpackprio_enabled())
      {
        netmgr_tc_create_tcp_ack_class_multiq(link, new_flow_info);
      }

      ds_dll_enq(netmgr_tc_cfg.links[link].flow_list, NULL, new_flow_info);

      netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_ACTIVATE, new_flow_info);

      return TRUE;
    }
  }

  return FALSE;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_activate_hdlr_multiq
===========================================================================*/
/*!
@brief
  Install the traffic control elements on Modem QoS flow create/resume.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_tc_flow_activate_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  int result = NETMGR_FAILURE;
  netmgr_tc_flow_info_t *flow_info = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT( qos_flow );
  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);

  NETMGR_LOCK_MUTEX(tc_lock);

  netmgr_log_low( "flow handle=0x%08x\n", (unsigned int)qos_flow->flow_id );
  netmgr_log_low( "flow is_new=%s\n", ((TRUE==qos_flow->is_new)?"TRUE":"FALSE") );
  netmgr_log_low( "flow priority=%d\n", qos_flow->priority );
  netmgr_log_low( "flow datarate=%" PRIu64 "\n", qos_flow->datarate );
  netmgr_log_low( "flow num_filter=%d\n", qos_flow->num_filter );
  netmgr_log_low( "flow filter_list=%p\n", qos_flow->filter_list );
  netmgr_log_low( "flow bearer_id=%d\n", qos_flow->bearer_id );
  netmgr_log_low( "flow ip_ver=%d\n", qos_flow->ip_ver );

  /* Check if default flow is already activated -
   * Could be one of the dual-IP sub call */
  if (NETMGR_IS_DEFAULT_FLOW(qos_flow->flow_id))
  {
    if (netmgr_tc_is_default_flow_activated_multiq(link, qos_flow))
    {
      netmgr_log_high("Default flow activated\n");
      result = NETMGR_SUCCESS;
      goto out;
    }
  }
  else
  {
    flow_info = netmgr_tc_find_flow_info(
                  link, qos_flow->flow_id, qos_flow->ip_ver);
  }

  /* Check for newly created flow */
  if( qos_flow->is_new ) {
    /* Check for pre-existing flow ID */
    if( !flow_info )
    {
      /* Create flow object and install QoS measures */
      result = netmgr_tc_flow_create_multiq( link, qos_flow );
    }
    else
    {
      /* This may happen if previously a suspend or activated flow event
       * happened. In this case network may be updating either flow or filter
       * or both. Hence we need to modify the flow and filter spec installed
       * in the kernel
       */
      netmgr_log_med( "flow is already active, modifying flow.\n" );
      result = netmgr_tc_flow_modify_hdlr_multiq(link,qos_flow);
    }
  }
  else
  {
    if( flow_info )
    {
      /* This may happen if previously a suspend or activated flow event
       * happened. In this case network may be updating either flow or filter
       * or both. Hence we need to modify the flow and filter spec installed
       * in the kernel
      */
      netmgr_log_med( "flow is already active, modifying flow.\n" );
      netmgr_tc_flow_modify_hdlr_multiq(link,qos_flow);
    }
    else
    {
      netmgr_log_err( "Cannot find flow object[0x%x], ignoring\n", qos_flow->flow_id );
    }
  }

out:
  NETMGR_UNLOCK_MUTEX(tc_lock);
  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_delete_hdlr_multiq
===========================================================================*/
/*!
@brief
 Remove the traffic control elements on Modem QoS flow release.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_tc_flow_delete_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  ds_dll_el_t* node = NULL;
  ds_dll_el_t* tail = NULL;
  netmgr_tc_flow_info_t flow_buf;
  netmgr_tc_flow_info_t* flow_info = NULL;
  const void* dummy = NULL;
  const void* data = NULL;
  int result = NETMGR_FAILURE;
  const char * dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(qos_flow);
  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);

  dev_name = netmgr_kif_get_name(link);
  if(NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto done;
  }

  NETMGR_LOCK_MUTEX(tc_lock);

  if (NETMGR_IS_DEFAULT_FLOW(qos_flow->flow_id))
  {
    /* Empty the flow list */
    netmgr_log_med("Empty the flow list");
    while (NULL != (node = ds_dll_deq(netmgr_tc_cfg.links[link].flow_list, NULL, &data)))
    {
      flow_info = (netmgr_tc_flow_info_t*) (data);
      if (flow_info)
      {
        netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_DELETE, flow_info);
        netmgr_log_med("Freeing flow_id=%u ip=%d",
                    flow_info->qos_flow.flow_id, flow_info->qos_flow.ip_ver);
        netmgr_tc_flow_free(&flow_info);
      }

      ds_dll_free(node);
    }

    if (netmgr_tc_cfg.links[link].default_class)
    {
      /* Default class handle already freed when freeing flow_info */
      netmgr_tc_cfg.links[link].default_class = NULL;
    }

    if (netmgr_tc_cfg.links[link].default_class2)
    {
      netmgr_tc_cfg.links[link].default_class2 = NULL;
    }

    if (netmgr_tc_cfg.links[link].root_class)
    {
      netmgr_free(netmgr_tc_cfg.links[link].root_class);
      netmgr_tc_cfg.links[link].root_class = NULL;
    }

    if (netmgr_tc_cfg.links[link].root_qdisc)
    {
      netmgr_free(netmgr_tc_cfg.links[link].root_qdisc);
      netmgr_tc_cfg.links[link].root_qdisc = NULL;
    }

    /* Start with the initial MINOR_ID value */
    netmgr_tc_cfg.links[ link ].next_class_minor = NETMGR_TC_CLASS_MINOR_ID_INIT;
    memset(&netmgr_tc_cfg.links[ link ].mq_info, 0,
           sizeof(netmgr_tc_cfg.links[ link ].mq_info));

    result = NETMGR_SUCCESS;
  }
  else
  {
    /* find the tail of the flow list */
    node = ds_dll_next(netmgr_tc_cfg.links[link].flow_list, &dummy);
    while (NULL != node)
    {
      tail = node;
      node = ds_dll_next(tail, &dummy);
    }

    /* search for the flow object and remove it from the dll */
    flow_buf.qos_flow.flow_id = qos_flow->flow_id;
    flow_buf.qos_flow.ip_ver = qos_flow->ip_ver;
    node = ds_dll_delete(netmgr_tc_cfg.links[link].flow_list,
                         &tail,
                         &flow_buf,
                         netmgr_tc_match_flows);

    if (node == NULL)
    {
      netmgr_log_err("flow %lx not found",(unsigned long) qos_flow->flow_id);
      goto done;
    }

    flow_info = (netmgr_tc_flow_info_t*) ds_dll_data(node);

    if (flow_info == NULL)
    {
      netmgr_log_err("flow %lx data not found",(unsigned long) qos_flow->flow_id);
      goto done;
    }

    netmgr_cb_runall(NETMGR_QMI_CB_TABLE, NETMGR_QMI_CB_FLOW_DELETE, flow_info);

    netmgr_tc_filter_delete_hdlr(flow_info, link, qos_flow->flow_id);

    if (flow_info->class_handle)
    {
      netmgr_tc_release_num_multiq(link, flow_info);
    }
    netmgr_tc_flow_free(&flow_info);
    ds_dll_free(node);
    result = NETMGR_SUCCESS;
  }

done:
  NETMGR_UNLOCK_MUTEX(tc_lock);

  NETMGR_LOG_FUNC_EXIT;
  return result;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_control_hdlr_multiq
===========================================================================*/
/*!
@brief
  Enable/disable packet scheduling from specific leaf qdisc.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_tc_flow_control_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow,
  netmgr_tc_flow_state_t state
)
{
  (void)link;
  (void)qos_flow;
  (void)state;

  return NETMGR_SUCCESS;
}

/*===========================================================================
  FUNCTION  netmgr_tc_flow_modify_hdlr_multiq
===========================================================================*/
/*!
@brief
 Change the traffic control elements on Modem QoS flow modify.

@return
  int - NETMGR_SUCCESS on successful operation,
        NETMGR_FAILURE otherwise

@note

  - Dependencies
    - None

  - Side Effects
    - None
*/
/*=========================================================================*/
int netmgr_tc_flow_modify_hdlr_multiq
(
  int link,
  const netmgr_qmi_qos_flow_info_t  *qos_flow
)
{
  netmgr_tc_flow_info_t *flow_info = NULL;
  const char *dev_name = NULL;

  NETMGR_LOG_FUNC_ENTRY;

  NETMGR_ASSERT(link >= 0 && link < netmgr_tc_cfg.nlink);
  NETMGR_ASSERT(qos_flow != NULL);

  NETMGR_LOCK_MUTEX(tc_lock);

  flow_info = netmgr_tc_find_flow_info(link, qos_flow->flow_id, qos_flow->ip_ver);

  netmgr_tc_flow_filter_dump(qos_flow);

  if (flow_info == NULL)
  {
    netmgr_log_err("flow_modify_hdlr: the flow object does not exist");
    goto error;
  }

  dev_name = netmgr_kif_get_name(link);
  if(NULL == dev_name)
  {
    netmgr_log_err("%s(): unable to determine name for link=%d\n", __func__, link);
    goto error;
  }

  /*disble flow on the leaf qdisc before modifying parent class*/
  if (NETMGR_SUCCESS != ( netmgr_tc_flow_control_hdlr_multiq(link,
                                                      qos_flow,
                                                      NETMGR_TC_FLOW_DISABLED)))
  {
    netmgr_log_err("could not disable exisiting flow");
    goto error;
  }

  netmgr_tc_delete_flow_chain_ref(flow_info, link);

  /* Since the flowID does not change, there is no need to
   * change the tc filter installed at the root
   */
  /*replace old qos spec with the new one*/
  flow_info->qos_flow = *qos_flow;

  /* Link the chain into the mangle table */
  if (netmgr_tc_flow_enable(link, flow_info ) != NETMGR_SUCCESS)
  {
    netmgr_log_err("error in linking iptables chain to mangle table");
    goto error;
  }

  /* If we reach here QOS modification is successul,
   * re-enable the leaf qdisc at this point
   */
  if (NETMGR_SUCCESS != ( netmgr_tc_flow_control_hdlr_multiq(link,
                                                      qos_flow,
                                                      NETMGR_TC_FLOW_ACTIVE)))
  {
    netmgr_log_err("could not re-enable flow");
    goto error;
  }

  NETMGR_UNLOCK_MUTEX(tc_lock);

  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_SUCCESS;

error:
  NETMGR_UNLOCK_MUTEX(tc_lock);
  NETMGR_LOG_FUNC_EXIT;
  return NETMGR_FAILURE;
}

#endif /* NETMGR_QOS_ENABLED */
