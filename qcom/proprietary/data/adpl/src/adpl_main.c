/******************************************************************************

                        ADPL_MAIN.C

******************************************************************************/

/******************************************************************************

  @file    adpl_main.c
  @brief   Accelerated Data Path Logging module

  DESCRIPTION
  Implementation of Accelerated Data Path Logging module.

  ---------------------------------------------------------------------------
  Copyright (c) 2012-2015,2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
04/25/18   vv         Initial version

******************************************************************************/

/*===========================================================================
                              INCLUDE FILES
===========================================================================*/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/if.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "adpl_cmdq.h"
#include "adpl.h"

/*===========================================================================
                              VARIABLE DECLARATIONS
===========================================================================*/

static adpl_param             adpl_config_param;
static odl_param              odl_config_param;

qcmap_sk_fd_set_info_t           sk_fdset;

/* Global Netlink Socket. variable */
qcmap_sk_info_t     sk_info;

/*===========================================================================
                              FUNCTION DEFINITIONS
===========================================================================*/

/*==========================================================================

FUNCTION PRINT_MSG()

DESCRIPTION

  This function is used to print QTI logs to a file

DEPENDENCIES
  None.

RETURN VALUE


SIDE EFFECTS
  None

==========================================================================*/

void PRINT_msg
(
  const char *funcName,
  uint32_t lineNum,
  const char*fmt, ...
)
{
}

/*===========================================================================

FUNCTION adpl_print_logcat_msg()

DESCRIPTION
- print the message in logcat
- Used for LA targets only

DEPENDENCIES
  None.

RETURN VALUE
  None

SIDE EFFECTS
  None

==========================================================================*/
void adpl_print_logcat_msg
(
  uint32_t msgType,
  const char *funcName,
  uint32_t lineNum,
  const char *fileInfo,
  const char *fmt, ...
)
{
  char msg[ADPL_MAX_COMMAND_STR_LEN] = {0};
  int len = 0;
  va_list args;
  char *fileName = NULL;

  if (funcName == NULL || fileInfo == NULL || fmt == NULL)
  {
    LOG_MSG_ERROR("Invalid params", 0, 0, 0);
    return ;
  }

  fileName = strrchr(fileInfo, '/'); //absolute path to only filename
  if (fileName == NULL)
  {
    len = snprintf(msg, ADPL_MAX_COMMAND_STR_LEN, "[%s:%d] %s():",
                   fileInfo, lineNum, funcName);
  }
  else
  {
    len = snprintf(msg, ADPL_MAX_COMMAND_STR_LEN, "[%s:%d] %s():",
                   fileName + 1, lineNum, funcName);
  }
  if (len > 0 && len < ADPL_MAX_COMMAND_STR_LEN)
  {
    va_start (args, fmt);
    vsnprintf(msg + len, ADPL_MAX_COMMAND_STR_LEN - len, fmt, args);
    va_end (args);
  }

  if (msgType == LOGCAT_INFO)
    __android_log_print( ANDROID_LOG_INFO, MAIN_LOG_TAG,"%s", msg);
  else
    __android_log_print( ANDROID_LOG_ERROR, MAIN_LOG_TAG,"%s", msg);

  return ;
}

/*==========================================================================

FUNCTION MAIN()

DESCRIPTION

  The main function for ADPL which is first called when ADPL gets started on
  boot up.

DEPENDENCIES
  None.

RETURN VALUE
  0 on SUCCESS
  -1 on FAILURE

SIDE EFFECTS
  None

==========================================================================*/
int main(int argc, char ** argv)
{
  int                       ret_val;
  ds_target_t               target;
/*-----------------------------------------------------------------------
    Initialize ADPL variables
------------------------------------------------------------------------*/
  memset(&sk_fdset, 0, sizeof(qcmap_sk_fd_set_info_t));
  memset(&adpl_config_param, 0, sizeof(adpl_param));

/*-------------------------------------------------------------------------
  Initializing Diag for QXDM logs
-------------------------------------------------------------------------*/
  if (TRUE != Diag_LSM_Init(NULL))
  {
     printf("Diag_LSM_Init failed !!");
  }

  LOG_MSG_INFO1("Start ADPL", 0, 0, 0);

  target = ds_get_target();
  adpl_config_param.target = target;
  odl_config_param.target = target;

  if( !((DS_TARGET_MSMNILE == adpl_config_param.target) ||
      (DS_TARGET_FUSION_SM8150_SDX50 == adpl_config_param.target)))
  {
     LOG_MSG_ERROR("Non Hana Target, exiting from ADPL daemon", 0, 0, 0);
     property_set("ctl.stop","vendor.dataadpl");
  }
  odl_config_param.is_odl_ipa_enabled = ODL_INVALID_VALUE;

  if( DS_TARGET_FUSION_SDM845_SDX24 == adpl_config_param.target ||
      DS_TARGET_FUSION4_5_PCIE == adpl_config_param.target ||
      DS_TARGET_FUSION_8084_9X45 == adpl_config_param.target ||
      DS_TARGET_FUSION_8096_9X55 == adpl_config_param.target)
  {
    if(DS_TARGET_FUSION_SDM845_SDX24 == adpl_config_param.target)
    {
      memcpy(adpl_config_param.adpl_iface_device_file,
           ADPL_USB_DEV_NODE,
           strlen(ADPL_USB_DEV_NODE));
    }
    else
    {
      memcpy(adpl_config_param.adpl_iface_device_file,
           ADPL_USB_DEV_FILE,
           strlen(ADPL_USB_DEV_FILE));
    }
  }
  else
  {
    memcpy(adpl_config_param.adpl_iface_device_file,
           ADPL_USB_DEV_FILE,
           strlen(ADPL_USB_DEV_FILE));

    memcpy(odl_config_param.odl_iface_device_file,
           ODL_CTL_DEV_FILE,
           strlen(ODL_CTL_DEV_FILE));
  }

/*---------------------------------------------------------------------
  Initialize ADPL modem interface
---------------------------------------------------------------------*/
  if ((DS_TARGET_FUSION_SDM845_SDX24 == adpl_config_param.target ||
       DS_TARGET_FUSION4_5_PCIE == adpl_config_param.target ||
       DS_TARGET_FUSION_8084_9X45 == adpl_config_param.target ||
       DS_TARGET_FUSION_8096_9X55 == adpl_config_param.target))
  {
    ret_val = adpl_modem_init_thru_qmuxd(&adpl_config_param,
                                        (qcmap_sock_thrd_fd_read_f)adpl_modem_recv_msg_thru_qmuxd);
    if(ret_val != ADPL_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to initialize ADPL modem interface",
                    0, 0, 0);
      return ADPL_FAILURE;
    }
  }

/*---------------------------------------------------------------------
      Initialize ADPL command queue
----------------------------------------------------------------------*/
    adpl_cmdq_init();

/*---------------------------------------------------------------------
  Initialize DPM client
---------------------------------------------------------------------*/

    if(DS_TARGET_FUSION_SDM845_SDX24 != adpl_config_param.target &&
       DS_TARGET_FUSION4_5_PCIE != adpl_config_param.target &&
       DS_TARGET_FUSION_8084_9X45 != adpl_config_param.target &&
       DS_TARGET_FUSION_8096_9X55 != adpl_config_param.target)
    {
      ret_val = adpl_dpm_init(&adpl_config_param, &odl_config_param);

      if(ret_val != ADPL_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize DPM",
                      0, 0, 0);
        return ADPL_FAILURE;
      }
      ret_val = adpl_modem_init(&adpl_config_param, &odl_config_param);
      if(ret_val != ADPL_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize Modem",
                      0, 0, 0);
        return ADPL_FAILURE;
      }
    }

/*---------------------------------------------------------------------
  Initialize peripheral interface for ADPL logging
---------------------------------------------------------------------*/
  ret_val = adpl_ph_init(&adpl_config_param,
                        &sk_fdset,
                        adpl_ph_recv_msg);
  if(ret_val != ADPL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to initialize peripheral interface for ADPL logging over USB",
                  0, 0, 0);
  }

/*---------------------------------------------------------------------
  Initialize peripheral interface for ODL logging
---------------------------------------------------------------------*/
  ret_val = odl_ph_init(&odl_config_param,
                        &sk_fdset,
                        odl_ph_recv_msg);
  if(ret_val != ADPL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to initialize ODL char device %s for ADPL logging over ODL",
                  ODL_CTL_DEV_FILE, 0, 0);
  }

/*--------------------------------------------------------------------
  Start the listener which listens to netlink events and QMI packets
  coming on USB-Rmnet device file
---------------------------------------------------------------------*/
  ret_val = qcmap_listener_start(&sk_fdset);

  if(ret_val != QCMAP_UTIL_SUCCESS)
  {
    LOG_MSG_ERROR("Failed to start NL listener",
                   0, 0, 0);
  }

  return ADPL_SUCCESS;
}


