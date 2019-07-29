/******************************************************************************

                        QTI_MAIN.C

******************************************************************************/

/******************************************************************************

  @file    qti_main.c
  @brief   Tethering Interface module

  DESCRIPTION
  Implementation of Tethering Interface module.

  ---------------------------------------------------------------------------
  Copyright (c) 2012-2014 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------


******************************************************************************/


/******************************************************************************

                      EDIT HISTORY FOR FILE

  $Id:$

when       who        what, where, why
--------   ---        -------------------------------------------------------
01/22/14   sb         Added QTI for Fusion
12/9/13    sb         Add port mapper functionality. Modem interface file interaction
11/15/12   sb         Initial version

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
#include <sys/inotify.h>

#include "qti_cmdq.h"
#include "qti.h"

/*===========================================================================
                              VARIABLE DECLARATIONS
===========================================================================*/

static qti_conf_t                qti_conf;
static qti_rmnet_param           rmnet_config_param;
static qti_dpl_param             dpl_config_param;
qcmap_sk_fd_set_info_t           sk_fdset;

/* Global Netlink Socket. variable */
qcmap_sk_info_t     sk_info;

#define INOTIFY_EVENT_SIZE  (sizeof(struct inotify_event))
#define INOTIFY_BUF_LEN     (INOTIFY_EVENT_SIZE + 2*sizeof(HEX_DUMPS_FOLDER))


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
  va_list args;

  /*------------------------------------------------------------------------*/
  va_start (args, fmt);
  fprintf(fp_logs,"\n Func: %s Line: %d :", funcName, lineNum);
  vfprintf (fp_logs, fmt, args);
  va_end (args);
}

/*===========================================================================

FUNCTION qti_print_logcat_msg()

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
void qti_print_logcat_msg
(
  uint32_t msgType,
  const char *funcName,
  uint32_t lineNum,
  const char *fileInfo,
  const char *fmt, ...
)
{
  char msg[QTI_MAX_COMMAND_STR_LEN] = {0};
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
    len = snprintf(msg, QTI_MAX_COMMAND_STR_LEN, "[%s:%d] %s():",
                   fileInfo, lineNum, funcName);
  }
  else
  {
    len = snprintf(msg, QTI_MAX_COMMAND_STR_LEN, "[%s:%d] %s():",
                   fileName + 1, lineNum, funcName);
  }
  if (len > 0 && len < QTI_MAX_COMMAND_STR_LEN)
  {
    va_start (args, fmt);
    vsnprintf(msg + len, QTI_MAX_COMMAND_STR_LEN - len, fmt, args);
    va_end (args);
  }

  if (msgType == LOGCAT_INFO)
    __android_log_print( ANDROID_LOG_INFO, MAIN_LOG_TAG,"%s", msg);
  else
    __android_log_print( ANDROID_LOG_ERROR, MAIN_LOG_TAG,"%s", msg);

  return ;
}

/*==========================================================================

FUNCTION HEX_DUMPS_RESET()

DESCRIPTION

  The function which enables the logging in QTI.

DEPENDENCIES
  None.

RETURN VALUE
  0 on SUCCESS

SIDE EFFECTS
  None

==========================================================================*/
int hex_dumps_reset
(
  int hex_dump_fd
)
{
  int length;
  int ret_val;
  char buffer[INOTIFY_BUF_LEN] = {0};
  uint32_t mask = IN_CREATE | IN_DELETE;

  LOG_MSG_INFO1("Received notifications in dir %s", HEX_DUMPS_FOLDER, 0, 0);

  struct inotify_event* event = NULL;

  length = read(hex_dump_fd, buffer, INOTIFY_BUF_LEN);

  if (length < 0)
  {
    LOG_MSG_ERROR("inotify read() error return length: %d and mask: 0x%x\n", length, mask, 0);
    return QTI_FAILURE;
  }
  event = (struct inotify_event*)calloc(1,length);
  if(event == NULL)
  {
    LOG_MSG_ERROR("Unable to allocate memory to event", 0, 0, 0);
    return QTI_FAILURE;
  }

  memcpy(event, buffer, length);

  if (event->len > 0)
  {
     if (!(event->mask & IN_ISDIR))
     {
       if(event->mask & IN_CREATE)
       {
         LOG_MSG_INFO1("event mask is create", 0, 0, 0);
         if(strncmp(event->name, HEX_DUMP_QXDM_FLAG, strlen(HEX_DUMP_QXDM_FLAG))==0)
           hex_dumps_qxdm_mode = TRUE;
         if(strncmp(event->name, HEX_DUMP_FILE_FLAG, strlen(HEX_DUMP_FILE_FLAG))==0)
           hex_dumps_file_mode = TRUE;
       }
       else if(event->mask & IN_DELETE)
       {
         LOG_MSG_INFO1("event mask is delete", 0, 0, 0);
         if(strncmp(event->name, HEX_DUMP_QXDM_FLAG, strlen(HEX_DUMP_QXDM_FLAG))==0)
           hex_dumps_qxdm_mode = FALSE;
         if(strncmp(event->name, HEX_DUMP_FILE_FLAG, strlen(HEX_DUMP_FILE_FLAG))==0)
           hex_dumps_file_mode = FALSE;
       }
     }
  }
  free(event);

  return QTI_SUCCESS;
}
/*==========================================================================

FUNCTION MAIN()

DESCRIPTION

  The main function for QTI which is first called when QTI gets started on
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
  int                       ret_val,i;
  uint8_t                   embd_mode = FALSE;
  char                      command[QTI_MAX_COMMAND_STR_LEN];
  static char               data_mode_value[QTI_PROPERTY_VALUE_MAX];
  char                      data_mode_default_value[PROPERTY_DATA_MODE_SIZE+1];
  int                       fusion_dpl_mode_only = 0;
  FILE                      *fp;
  int                       inotify_fd;
  uint32_t                  mask;
  int                       wd;

/*------------------------------------------------------------------------*/

  file_log_mode = FALSE;
  hex_dumps_qxdm_mode = FALSE;
  hex_dumps_file_mode = FALSE;

/*-----------------------------------------------------------------------
    Initialize QTI variables
------------------------------------------------------------------------*/
  memset(&sk_fdset, 0, sizeof(qcmap_sk_fd_set_info_t));
  memset(&qti_conf, 0, sizeof(qti_conf_t));
  memset(&rmnet_config_param, 0, sizeof(qti_rmnet_param));
  memset(&dpl_config_param, 0, sizeof(qti_dpl_param));

/*-------------------------------------------------------------------------
 qti invoked in embedded mode, during this mode qti does not initialize rndis
 and ecm.
-------------------------------------------------------------------------*/

  for(i = 1; i < argc; i++)
  {
    if(0 == strncasecmp(argv[i],"em",strlen("em")))
    {
      system("echo QTI embd_mode > /dev/kmsg");
      embd_mode = TRUE;
    }

    if(0 == strncasecmp(argv[i],"-d",strlen("-d")))
    {
      file_log_mode = TRUE;

/*-------------------------------------------------------------------------
Delete the LOGs.txt file if present
-------------------------------------------------------------------------*/
      snprintf(command, QTI_MAX_COMMAND_STR_LEN, "rm %s", LOG_FILE);
      ds_system_call(command, strlen(command));

      fp_logs = fopen( LOG_FILE, "w" );
      if(fp_logs == NULL)
      {
        printf(" Error in opening /data/LOGs.txt\n");
        return QTI_FAILURE;
      }

      fprintf( fp_logs, "QTI LOGGING IN /data/LOGs.txt \n" );

/*-------------------------------------------------------------------------
Fetch the file descriptor from file pointer
-------------------------------------------------------------------------*/
      qtilogfd = fileno(fp_logs);
      if( qtilogfd == QTI_FAILURE )
      {
        printf( " File /data/LOGs.txt is not open, errno: %d\n",errno );
        fclose ( fp_logs );
        return QTI_FAILURE;
      }

      if(fcntl(qtilogfd, F_SETFD, FD_CLOEXEC) < 0)
      {
        printf( "Couldn't set Close on Exec, errno: %d", errno );
        fclose ( fp_logs );
        return QTI_FAILURE;
      }
    }
  }

/*-------------------------------------------------------------------------
  Initializing Diag for QXDM logs
-------------------------------------------------------------------------*/
  if (TRUE != Diag_LSM_Init(NULL))
  {
     printf("Diag_LSM_Init failed !!");
  }

/*-------------------------------------------------------------------------
  Read the hex dumps options in the qti folder
-------------------------------------------------------------------------*/
  fp = fopen(HEX_DUMP_QXDM_FILE, "r");
  if (fp!=NULL)
  {
    hex_dumps_qxdm_mode = TRUE;
    fclose(fp);
  }
  fp = fopen(HEX_DUMP_LOG_FILE, "r");
  if (fp!=NULL)
  {
    hex_dumps_file_mode = TRUE;
    fclose(fp);
  }

  HEXDUMP_LOG_MSG("QTI Start \n");

  inotify_fd= inotify_init();
  mask = IN_CREATE | IN_DELETE;

  if (inotify_fd < 0)
  {
    LOG_MSG_ERROR("Error in inotify_init for hex dumps", 0, 0, 0);
  }
  else
  {
    wd = inotify_add_watch(inotify_fd, HEX_DUMPS_FOLDER, mask);
    if(wd > 0)
    {
      LOG_MSG_INFO1("Successfully got the watch descriptor for the hex dumps", 0, 0, 0);
      if( qcmap_nl_addfd_map(&sk_fdset,
                             inotify_fd,
                             hex_dumps_reset,
                             MAX_NUM_OF_FD) != QCMAP_UTIL_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to map inotify fd with the hex dumps reset function",0,0,0);
        inotify_rm_watch(inotify_fd, wd);
        close(inotify_fd);
        inotify_fd = 0;
        return QCMAP_UTIL_FAILURE;
      }
    }
    else
      LOG_MSG_ERROR("Failed to add a watch to the hex dumps folder",0,0,0);
  }

  LOG_MSG_INFO1("Start QTI", 0, 0, 0);

/*-----------------------------------------------------------------------
    Identify target in use
------------------------------------------------------------------------*/
  rmnet_config_param.target = ds_get_target();
  dpl_config_param.target = ds_get_target();

  if( DS_TARGET_FUSION_SDM845_SDX24 == rmnet_config_param.target ||
      DS_TARGET_FUSION4_5_PCIE == rmnet_config_param.target ||
      DS_TARGET_FUSION_8084_9X45 == rmnet_config_param.target ||
      DS_TARGET_FUSION_8096_9X55 == rmnet_config_param.target)
  {
    if(DS_TARGET_FUSION_SDM845_SDX24 == rmnet_config_param.target){
       memcpy(rmnet_config_param.ph_iface[PH_DRIVER_TYPE_USB].ph_iface_device_file,
              RMNET_USB_DEV_NODE,
              strlen(RMNET_USB_DEV_NODE));
    }
    else{
       memcpy(rmnet_config_param.ph_iface[PH_DRIVER_TYPE_USB].ph_iface_device_file,
              RMNET_USB_DEV_FILE,
              strlen(RMNET_USB_DEV_FILE));
    }
    memcpy(rmnet_config_param.ph_iface[PH_DRIVER_TYPE_USB].ph_data_iface_name,
           USB_DATA_INTERFACE,
           strlen(USB_DATA_INTERFACE));

    memcpy(rmnet_config_param.modem_iface_device_file,
           RMNET_MHI_DEV_FILE,
           strlen(RMNET_MHI_DEV_FILE));
    memcpy(rmnet_config_param.modem_data_iface_name,
           MHI_DATA_INTERFACE,
           strlen(MHI_DATA_INTERFACE));
    memcpy(dpl_config_param.dpl_iface_device_file,
           DPL_USB_DEV_FILE,
           strlen(DPL_USB_DEV_FILE));

    rmnet_config_param.qmux_conn_id = QMI_CONN_ID_RMNET_MHI_1;
  }
  else
  {
    memcpy(rmnet_config_param.ph_iface[PH_DRIVER_TYPE_USB].ph_iface_device_file,
           RMNET_USB_DEV_FILE,
           strlen(RMNET_USB_DEV_FILE));
    memcpy(rmnet_config_param.ph_iface[PH_DRIVER_TYPE_MHI].ph_iface_device_file,
           RMNET_MHI_PH_DEV_FILE,
           strlen(RMNET_MHI_PH_DEV_FILE));
    memcpy(rmnet_config_param.modem_iface_device_file,
           RMNET_SMD_DEV_FILE,
           strlen(RMNET_SMD_DEV_FILE));
    memcpy(dpl_config_param.dpl_iface_device_file,
           DPL_USB_DEV_FILE,
           strlen(DPL_USB_DEV_FILE));
    rmnet_config_param.qmux_conn_id = QMI_CONN_ID_RMNET_8;
  }

#ifndef FEATURE_MDM_LE
  if( DS_TARGET_FUSION4_5_PCIE == rmnet_config_param.target ||
      DS_TARGET_FUSION_8084_9X45 == rmnet_config_param.target ||
      DS_TARGET_FUSION_8096_9X55 == rmnet_config_param.target)
  {
    memset(data_mode_value, 0, sizeof(data_mode_value));
    memset( data_mode_default_value, 0x0, sizeof(data_mode_default_value) );

    snprintf( data_mode_default_value, sizeof(data_mode_default_value)-1, "%s", "" );
    ret_val = property_get( QTI_PROPERTY_DATA_MODE, data_mode_value, data_mode_default_value );

    if (ret_val > (int)(PROPERTY_DATA_MODE_SIZE))
    {
      LOG_MSG_ERROR("Android property for data mode has unexpected size(%d)",
                    ret_val, 0, 0);
    }

    if(strncmp(data_mode_value, TETHERED_VAL, strlen(TETHERED_VAL)) == 0)
    {
      fusion_dpl_mode_only = 0;
      LOG_MSG_INFO1("QTI:Not DPL only mode", 0, 0, 0);
      ds_system_call("echo QTI:Not DPL only mode > /dev/kmsg",
                  strlen("echo QTI:Not DPL only mode > /dev/kmsg"));
    }
    else
    {
      fusion_dpl_mode_only = 1;
      LOG_MSG_INFO1("QTI:DPL only mode", 0, 0, 0);
      ds_system_call("echo QTI:DPL only mode > /dev/kmsg",
                  strlen("echo QTI:DPL only mode > /dev/kmsg"));
    }
  }
#endif //FEATURE_MDM_LE
  LOG_MSG_INFO1("DPL mode only %d", fusion_dpl_mode_only, 0, 0);


/*---------------------------------------------------------------------
  Initialize QTI modem interface
---------------------------------------------------------------------*/
  if ((DS_TARGET_FUSION_SDM845_SDX24 == rmnet_config_param.target ||
       DS_TARGET_FUSION4_5_PCIE == rmnet_config_param.target ||
       DS_TARGET_FUSION_8084_9X45 == rmnet_config_param.target ||
       DS_TARGET_FUSION_8096_9X55 == rmnet_config_param.target) &&
       !fusion_dpl_mode_only)
  {
    ret_val = qti_rmnet_modem_init_thru_qmuxd(&rmnet_config_param,
                                              (qcmap_sock_thrd_fd_read_f)qti_rmnet_modem_recv_msg_thru_qmuxd,
                                              &dpl_config_param);
    if(ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to initialize QTI modem interface",
                    0, 0, 0);
      return QTI_FAILURE;
    }
  }

/*---------------------------------------------------------------------
  Initialize QTI peripheral interface
---------------------------------------------------------------------*/

  if(!fusion_dpl_mode_only)
  {
    ret_val = qti_rmnet_ph_init(&rmnet_config_param,
                                &sk_fdset,
                                qti_rmnet_ph_recv_msg);
    if(ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to initialize QTI peripheral interface",
                    0, 0, 0);
      return QTI_FAILURE;
    }

/*---------------------------------------------------------------------
      Initialize QTI command queue
----------------------------------------------------------------------*/
    qti_cmdq_init();

/*---------------------------------------------------------------------
  Initialize DPM client
---------------------------------------------------------------------*/

    if(DS_TARGET_FUSION_SDM845_SDX24 != rmnet_config_param.target &&
       DS_TARGET_FUSION4_5_PCIE != rmnet_config_param.target &&
       DS_TARGET_FUSION_8084_9X45 != rmnet_config_param.target &&
       DS_TARGET_FUSION_8096_9X55 != rmnet_config_param.target)
    {
      ret_val = qti_dpm_init(&rmnet_config_param,
                             &dpl_config_param);

      if(ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize DPM",
                      0, 0, 0);
        return QTI_FAILURE;
      }
      ret_val = qti_rmnet_modem_init(&rmnet_config_param,
                                     &dpl_config_param);
      if(ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize Modem",
                      0, 0, 0);
        return QTI_FAILURE;
      }
    }
  }
#ifdef FEATURE_MDM_LE

  if( !embd_mode )
  {
    /*-----------------------------------------------------------------------
      Initialize QTI interfaces for netlink events
      ------------------------------------------------------------------------*/
    ret_val = qti_netlink_init(&qti_conf);

    if(ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to initialize netlink interfaces for QTI",
          0, 0, 0);
      return QTI_FAILURE;
    }

    /*---------------------------------------------------------------------
      Call into the netlink listener init function which sets up QTI to
      listen to netlink events
      ---------------------------------------------------------------------*/
    if (qcmap_nl_listener_init(
                                 NETLINK_ROUTE,
                                 RTMGRP_LINK,
                                 &sk_fdset,
                                 qti_nl_recv_msg,
                                 &sk_info,
                                 MAX_NUM_OF_FD) == QCMAP_UTIL_SUCCESS)
    {
      LOG_MSG_INFO1("Open netlink socket succeeds",0,0,0);
    }
    else
    {
      LOG_MSG_ERROR("Netlink socket open failed",0,0,0);
      return QTI_FAILURE;
    }
    /*-----------------------------------------------------------------------
      Initialize QTI to be a client of QCMAP
      ------------------------------------------------------------------------*/
    ret_val = qti_qcmap_init(&qti_conf);
    if(ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to post qcmap init command to cmd_queue",
          0, 0, 0);
      return QTI_FAILURE;
    }
  }

#endif //FEATURE_MDM_LE
  if(!fusion_dpl_mode_only)
  {
    if( (DS_TARGET_FUSION_SDM845_SDX24 == rmnet_config_param.target) ||
        (DS_TARGET_FUSION4_5_PCIE == rmnet_config_param.target) ||
        (DS_TARGET_FUSION_8084_9X45 == rmnet_config_param.target) ||
        (DS_TARGET_FUSION_8096_9X55 == rmnet_config_param.target) ||
        (DS_TARGET_JOLOKIA == rmnet_config_param.target) ||
        (DS_TARGET_LYKAN == rmnet_config_param.target) ||
        (DS_TARGET_FEERO == rmnet_config_param.target))
    {
      ret_val = qti_rmnet_qmi_init(&rmnet_config_param);
      if(ret_val != QTI_SUCCESS)
      {
        LOG_MSG_ERROR("Failed to initialize QMI message handler",
              0, 0, 0);
        return QTI_FAILURE;
      }
    }
  }

/*---------------------------------------------------------------------
  Initialize QTI peripheral interface for DPL logging
---------------------------------------------------------------------*/
  if( !((DS_TARGET_MSMNILE == dpl_config_param.target) ||
      (DS_TARGET_FUSION_SM8150_SDX50 == dpl_config_param.target)))
  {
    ret_val = qti_dpl_ph_init(&dpl_config_param,
                              &sk_fdset,
                              qti_dpl_ph_recv_msg);
    if(ret_val != QTI_SUCCESS)
    {
      LOG_MSG_ERROR("Failed to initialize QTI peripheral interface for DPL logging",
                     0, 0, 0);
    }
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
/*--------------------------------------------------------------------
  Wait for the QTI command queue to finish before exiting QTI
---------------------------------------------------------------------*/
#ifdef FEATURE_MDM_LE

  if(!embd_mode)
  {
    qti_qcmap_exit();
  }
#endif //FEATURE_MDM_LE

  if(!fusion_dpl_mode_only)
  {
    qti_cmdq_wait();
  }

  fclose(fp_logs);

  return QTI_SUCCESS;
}


