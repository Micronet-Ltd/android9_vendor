/*==========================================================================
Description
  This file taps regular BT UART data (hci command, event, ACL packets)
  and writes into DiagInterface. It also writes the controller logs (controller's
  printf strings, LMP RX and TX data) received from BT SOC over the UART.

# Copyright (c) 2013, 2016-2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc..

===========================================================================*/
#include <unistd.h>
#include "diag_interface.h"
#include "ibs_handler.h"
#include <cutils/properties.h>
#include <unistd.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.bluetooth@1.0-diag_interface"

#define DIAG_SSR_BT_CMD     0x0007
#define PRINT_BUF_SIZE      (260 * 3 + 2)

/* QXDM ID for LMP packers */
#define LOG_BT_DIAG_LMP_LOG_ID 0x1041
#define LOG_BT_DIAG_LMP_RX_ID  0x1048
#define LOG_BT_DIAG_LMP_TX_ID  0x1049

/* To format LMP logs */
#define LOG_BT_QXDM_PKT_LENGTH_POS    0
#define LOG_BT_QXDM_PKT_LENGTH2_POS   1
#define LOG_BT_QXDM_DEVICE_IDX_POS    2
#define LOG_BT_QXDM_PKT_POS           3

#define LOG_BT_DBG_DEVICE_IDX_POS 0
#define LOG_BT_DBG_PKT_LENGTH_POS 1
#define LOG_BT_DBG_PKT_POS 2

#define DIAG_CMD_SSR_SW_ERR_FAULT           38
#define DIAG_CMD_SSR_SW_EXCPTN_DIV_ZERO     39
#define DIAG_CMD_SSR_SW_EXCPTN_DIV_ZERO     40
#define DIAG_CMD_SSR_WATCHDOG               41

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

typedef struct {
  log_hdr_type hdr;
  byte data[1];
} bt_log_pkt;

const uint16_t DiagInterface::log_bt_hdr_size_ = (sizeof(bt_log_pkt) - 1);
bool DiagInterface :: is_cleanup_in_progress_ = false;
bool DiagInterface :: is_forced_ssr_triggered_ = false;
bool DiagInterface :: is_diag_ssr_triggered_ = false;

DiagInterface :: DiagInterface()
{
}

bool DiagInterface :: Init(HciTransport *hci_transport)
{
  hci_transport_ = hci_transport;
  {
    std::unique_lock<std::mutex> guard(diag_mutex);
    is_diag_ssr_triggered_ = false;
    is_cleanup_in_progress_ = false;
    is_forced_ssr_triggered_ = false;
  }
  init_status = Diag_LSM_Init(NULL);
  if (!init_status) {
    ALOGE("Failed to Init diag");
    return false;
  }
  return true;
}

void DiagInterface :: CleanUp()
{
  if (init_status) {
    Diag_LSM_DeInit();
    init_status = false;
  }
  hci_transport_ = nullptr;
  is_diag_ssr_triggered_ = false;
}

void DiagInterface :: SendLogs(const uint8_t *buff, int len, int type)
{
  if (buff == NULL)
    return;

  int ret;


  ret = log_status(type);
  if (ret) {
    char timebuff[8];
    bt_log_pkt*  bt_log_pkt_ptr = NULL;

    GetTime(timebuff);

    bt_log_pkt_ptr = (bt_log_pkt*)log_alloc(type, log_bt_hdr_size_ + len);

    if (bt_log_pkt_ptr != NULL) {
      if (type == LOG_BT_DIAG_LMP_RX_ID || type == LOG_BT_DIAG_LMP_TX_ID)
        FormatLmp(bt_log_pkt_ptr->data, (uint8_t*)buff, len);
      else
        memcpy(bt_log_pkt_ptr->data, (void*)buff, len);

      //((log_header_type *)&(bt_log_pkt_ptr->hdr))->ts_lo = *(uint32 *)&timebuf[0];
      //((log_header_type *)&(bt_log_pkt_ptr->hdr))->ts_hi = *(uint32 *)&timebuf[4];

      log_commit(bt_log_pkt_ptr);
    } else {
      ALOGE(": log_alloc returned NULL");
    }
  }
}

void DiagInterface :: GetTime(char *timebuff)
{
  char *temp;
  struct timeval tv;
  uint64_t microseconds;

  gettimeofday(&tv, NULL);

  microseconds = (uint64_t)tv.tv_sec * 1000;
  microseconds += ((uint64_t)tv.tv_usec / 1000);
  microseconds *= 4;
  microseconds = microseconds / 5;
  microseconds = microseconds << 16;

  temp = (char*)(&microseconds);

  for (unsigned long i = 0; i < sizeof(microseconds); i++)
    *(timebuff + i) = *(temp + i);
}

void DiagInterface :: FormatLmp(uint8 *dst, uint8 *src, int len)
{
  dst[LOG_BT_QXDM_PKT_LENGTH_POS] = src[LOG_BT_DBG_PKT_LENGTH_POS];
  dst[LOG_BT_QXDM_PKT_LENGTH2_POS] = 0x0;

  dst[LOG_BT_QXDM_DEVICE_IDX_POS] = src[LOG_BT_DBG_DEVICE_IDX_POS];

  memcpy( (dst + LOG_BT_QXDM_PKT_POS), (src + LOG_BT_DBG_PKT_POS), (len - LOG_BT_QXDM_PKT_POS) );
}

void * DiagInterface :: SsrBtHandler(void *req_pkt, uint16_t pkt_len)
{
  {
    std::unique_lock<std::mutex> guard(diag_mutex);
    if (is_diag_ssr_triggered_ || is_forced_ssr_triggered_) {
      ALOGE("%s: SSR is in progress - returning!", __func__);
      return NULL;
    } else if (is_cleanup_in_progress_) {
      ALOGE("%s: HAL Cleanup in progress - returning!", __func__);
      return NULL;
    }
    is_diag_ssr_triggered_ = true;
  }

  void *rsp = NULL;
  unsigned char *pkt_ptr = (unsigned char*)req_pkt + 4;
  int i, retval = 0;
  unsigned short p_len, p_opcode;
#ifdef WCNSS_IBS_ENABLED
  bool ibs_enabled = IbsHandler::IsEnabled();
#else
  bool ibs_enabled = false;
#endif

  char data_buf[PRINT_BUF_SIZE] = { 0, };

  rsp = diagpkt_subsys_alloc( DIAG_SUBSYS_BT, DIAG_SSR_BT_CMD, pkt_len);
  if (rsp != NULL) {
    p_len = *(pkt_ptr + 3); /* VS Command packet length */
    p_opcode = (*(pkt_ptr + 2) << 8) | *(pkt_ptr + 1);

    ALOGI("%s : p_len: %d, pkt_len -8: %d, p_opcode:%.04x \n", __func__,
          p_len, pkt_len - 8, p_opcode);

    if (p_len != (pkt_len - 8) || ( p_opcode != 0xFC00 && p_opcode != 0xFC0C)) {
      ResetSsrTriggeredFlag();
      return rsp;
    }

    for (i = 0; (i < (p_len + 4) && (i * 3 + 2) < PRINT_BUF_SIZE); i++)
      snprintf((char*)data_buf, PRINT_BUF_SIZE, "%s %.02x ",
               (char*)data_buf, *(pkt_ptr + i));

    ALOGD("Received data : %s", data_buf);

    if ((*(pkt_ptr + 4) < DIAG_CMD_SSR_SW_ERR_FAULT) ||
        (*(pkt_ptr + 4) > DIAG_CMD_SSR_WATCHDOG)) {
      ALOGW("Received Invalid last byte for SSR : %.02x ", *(pkt_ptr + 4));
      ResetSsrTriggeredFlag();
      return rsp;
    }

    /* Send VS Command from DiagInterface input */
#ifdef WCNSS_IBS_ENABLED
    /* Send wake up byte to controller */
    if (ibs_enabled)
      IbsHandler::Get()->WakeAssert();
#endif
    retval = write(hci_transport_->GetCtrlFd(), pkt_ptr, (p_len + 4));
    if (retval < 0) {
      ALOGE("%s:error in writing buf: %d: %s", __func__, retval, strerror(errno));
      ResetSsrTriggeredFlag();
    }

    memcpy(rsp, req_pkt, pkt_len);
#ifdef WCNSS_IBS_ENABLED
    if (ibs_enabled)
      IbsHandler::Get()->TransmitDone(true);
#endif
  }
  return (void*)rsp;
}

bool DiagInterface :: isSsrTriggered()
{
  std::unique_lock<std::mutex> guard(diag_mutex);
  return (is_diag_ssr_triggered_ || is_forced_ssr_triggered_);
}

bool DiagInterface :: isDiagSsrTriggered()
{
  std::unique_lock<std::mutex> guard(diag_mutex);
  return is_diag_ssr_triggered_;
}

void DiagInterface :: ResetSsrTriggeredFlag()
{
  std::unique_lock<std::mutex> guard(diag_mutex);
  is_diag_ssr_triggered_ = false;
}

bool DiagInterface :: SetSsrTriggeredFlag()
{
  std::unique_lock<std::mutex> guard(diag_mutex);
  if (is_diag_ssr_triggered_ == true || is_cleanup_in_progress_ == TRUE
    || is_forced_ssr_triggered_ == true )
    return false;
  is_forced_ssr_triggered_ = true;
  return true;
}

void DiagInterface :: SignalEndofCleanup()
{
  std::unique_lock<std::mutex> guard(diag_mutex);
  is_cleanup_in_progress_ = false;
}

bool DiagInterface :: SignalHALCleanup()
{
  std::unique_lock<std::mutex> guard(diag_mutex);
  if (is_diag_ssr_triggered_ == true || is_cleanup_in_progress_ == TRUE
    || is_forced_ssr_triggered_ == true )
    return false;
  is_cleanup_in_progress_ = true;
  return true;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
