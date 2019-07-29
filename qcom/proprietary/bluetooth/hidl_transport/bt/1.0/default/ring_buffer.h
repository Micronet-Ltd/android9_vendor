/*==========================================================================
Description
  It has implementation for ring buffer.

# Copyright (c) 2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#pragma once

#include <fcntl.h>
#include <errno.h>
#include <mutex>
#include "hci_internals.h"
#include <map>
#define RING_BUFF_FILE_NAME_LEN 255
#define HCICMD_TIMESTAMP_LEN (64)
#define SYS_TIMESTAMP_LEN_MAX (32)
#define RING_BUFF_LOG_TX_FILE_NAME    "/data/vendor/ssrdump/ramdump_bt_out_ringbuf_%.02d-%.02d-%.02d_%.02d-%.02d-%.02d.log"
#define RING_BUFF_LOG_RX_FILE_NAME    "/data/vendor/ssrdump/ramdump_bt_in_ringbuf_%.02d-%.02d-%.02d_%.02d-%.02d-%.02d.log"

#define TIME_STR_SIZE 13
#define RX_TAG_STR_LEN    64
namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

enum RxThreadEventType {
  RX_PRE_STACK_EVT_CALL_BACK = 0,
  RX_POST_STACK_EVT_CALL_BACK = 1,
  RX_PRE_STACK_ACL_CALL_BACK = 2,
  RX_POST_STACK_ACL_CALL_BACK = 3,
  RX_PRE_SELECT_CALL_BACK = 4,
  RX_POST_SELECT_CALL_BACK = 5
};

enum ReasonCode  {
  REASON_SOC_CRASHED = 0,                       //SOC WAS CRASHED
  REASON_SOC_CRASHED_DIAG_SSR = 1,              //SOC CRASHED DIAG INITIATED SSR
  REASON_UNABLE_TO_WAKE_SOC = 2,                //UNABLE TO WAKE UP SOC
  REASON_PATCH_DLDNG_FAILED_SOCINIT = 3,        //CONTROLLED INIT FAILED
  REASON_CLOSE_RCVD_DURING_INIT = 4,            //CLOSE RECEIVED FROM STACK DURING SOC INIT
  REASON_ERROR_READING_DATA_FROM_UART = 5,      //ERROR READING DATA FROM UART
  REASON_WRITE_FAIL_SPCL_BUFF_CRASH_SOC = 6,    //FAILED TO WRITE SPECIAL BYTES TO CRASH SOC
  REASON_SPURIOUS_WAKEUP_SOC_WAIT_TIMEOUT = 7,  //SPURIOUS WAKE AND SOC CRASH WAIT TIMEOUT
  REASON_CMD_TIMEDOUT_SOC_WAIT_TIMEOUT = 8,     //COMMAND TIMEOUT AND SOC CRASH WAIT TIMEOUT
  REASON_SSR_CMD_TIMEDOUT = 0x22,               //SSR DUE TO CMD TIMED OUT
  REASON_SSR_SPURIOUS_WAKEUP = 0x23,            //SSR DUE TO SPURIOUS WAKE UP
  REASON_SSR_INVALID_BYTES_RCVD = 0x24,         //INVALID HCI CMD TYPE RECEIVED
  REASON_SSR_RCVD_LARGE_PKT_FROM_SOC = 0x25,    //SSR DUE TO LARGE PKT RECVIVED FROM SOC
  REASON_RX_THREAD_STRUCK = 0x26,               //RX THREAD STRUCK
  REASON_DEFAULT_NONE  = 0x27                   //INVALID REASON
};

class PacketBuff
{
 private:
  const static long int max_buff_size_ = 1024*50;

  unsigned int active_buff_;
  uint8_t buff_[2][max_buff_size_];
  long int buff_index_[2];  
  void *logger_;
  std::map<int, char*> map_code_reason_;
 public:
  PacketBuff(void);
  ~PacketBuff();
  void AddBuffNode(HciPacketType, uint16_t, uint8_t *);
  void AddBuffNode(uint16_t len, uint8_t *pkt);
  void AddIbsData(uint8_t ibs_data);
  void DumpData(short int dir, ReasonCode reason);
  void DumpStatInfo(int dst_fd);
  char* GetReason(ReasonCode reason);
};

class RingBuffer
{
 private:
  PacketBuff tx_;
  PacketBuff rx_;
  std::mutex tx_ring_buffer_mutex_;
  std::mutex rx_ring_buffer_mutex_;
  struct timeval time_pre_stack_event_call_back;
  struct timeval time_post_stack_event_call_back;
  struct timeval time_pre_stack_acl_call_back;
  struct timeval time_post_stack_acl_call_back;
  struct timeval time_pre_select_call_back;
  struct timeval time_post_select_call_back;

  void AddRxTagEvent(char* dest_tag_str, const char* rx_tag_event,
       struct timeval& time_val);
  char last_hci_cmd[HCICMD_TIMESTAMP_LEN];
 public:
  enum {
    HOST_TO_SOC,
    SOC_TO_HOST,
  } pkt_direction;

  RingBuffer(void) {};
  void AddLog(short int, HciPacketType, uint8_t *, uint16_t);
  void AddIbsCmd(short int dir, uint8_t ibs_data);
  void DumpData(ReasonCode reason);
  void UpdateRxEventTag(RxThreadEventType rx_event);
  void SaveLastStackHciCommand(char*);
  void UnlockMutex();
};

void add_time_str(char *src, struct timeval* p_tval);
}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
