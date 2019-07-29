/*==========================================================================
Description
  It has implementation for ring buffer.

# Copyright (c) 2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#include <string.h>
#include <stdlib.h>
#include <utils/Log.h>
#include <unistd.h>

#include "ring_buffer.h"
#include "logger.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.bluetooth@1.0-ring_buffer"

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

const char* str_rx_event_pre_call_back = "event call back-->";
const char* str_rx_event_post_call_back = "event call back<--";
const char* str_rx_acl_pre_call_back = "data call back-->";
const char* str_rx_acl_post_call_back = "data call back<--";
const char* str_rx_select_pre_call_back = "select call back-->";
const char* str_rx_select_post_call_back = "select call back<--";

#define IN_RINGBUFF_EVENTS_SIZE (192)

void RingBuffer::AddLog(short int dir, HciPacketType type, uint8_t *pkt, uint16_t len)
{
  if (dir == HOST_TO_SOC) {
    std::unique_lock<std::mutex> guard(tx_ring_buffer_mutex_);
    tx_.AddBuffNode(type, len, pkt);
  }
  else {
    std::unique_lock<std::mutex> guard(rx_ring_buffer_mutex_);
    rx_.AddBuffNode(type, len, pkt);
  }
}

void RingBuffer::UnlockMutex()
{
  ALOGE("%s: unlocking Ringbuffer Mutex(s)", __func__);
  rx_ring_buffer_mutex_.unlock();
  tx_ring_buffer_mutex_.unlock();
}

void RingBuffer::UpdateRxEventTag(RxThreadEventType rx_event)
{
  switch (rx_event) {
    case RX_PRE_STACK_EVT_CALL_BACK:
      gettimeofday(&time_pre_stack_event_call_back, NULL);
      break;
    case RX_POST_STACK_EVT_CALL_BACK:
      gettimeofday(&time_post_stack_event_call_back, NULL);
      break;
    case RX_PRE_STACK_ACL_CALL_BACK:
      gettimeofday(&time_pre_stack_acl_call_back, NULL);
      break;
    case RX_POST_STACK_ACL_CALL_BACK:
      gettimeofday(&time_post_stack_acl_call_back, NULL);
      break;
    case RX_PRE_SELECT_CALL_BACK:
      gettimeofday(&time_pre_select_call_back, NULL);
      break;
    case RX_POST_SELECT_CALL_BACK:
      gettimeofday(&time_post_select_call_back, NULL);
      break;
    default:
      ALOGE("%s, received other Rx event tag", __func__);
      break;
  }
}

void RingBuffer::SaveLastStackHciCommand(char * cmd)
{
  strlcpy(last_hci_cmd, cmd, HCICMD_TIMESTAMP_LEN);
}

void RingBuffer::AddRxTagEvent(char* dest_tag_str, const char* rx_tag_event, struct timeval& time_val)
{
  uint32_t w_index = 0;

  memset(dest_tag_str, 0, RX_TAG_STR_LEN);
  add_time_str(dest_tag_str, &time_val);
  w_index += TIME_STR_SIZE;
  snprintf(dest_tag_str+w_index, strlen(rx_tag_event) + 1, "%s",
        rx_tag_event);
}

void RingBuffer::DumpData(ReasonCode reason)
{
  ALOGD("%s", __func__);
  {
    std::unique_lock<std::mutex> guard(tx_ring_buffer_mutex_);
    ALOGD("%s, last HCI cmd from stack is= %s", __func__, last_hci_cmd);
    tx_.DumpData(HOST_TO_SOC, reason);
  }

  {
    char tag_event_call_back[RX_TAG_STR_LEN];

    std::unique_lock<std::mutex> guard(rx_ring_buffer_mutex_);

    AddRxTagEvent(tag_event_call_back, str_rx_event_pre_call_back,
                  time_pre_stack_event_call_back);
    ALOGE("%s: TS of pre_stack_event_call_back %s", __func__, tag_event_call_back);
    rx_.AddBuffNode(strlen(tag_event_call_back) + 1,
          (uint8_t*)tag_event_call_back);

    AddRxTagEvent(tag_event_call_back, str_rx_event_post_call_back,
                  time_post_stack_event_call_back);
    ALOGE("%s: TS of post_stack_event_call_back %s", __func__, tag_event_call_back);
    rx_.AddBuffNode(strlen(tag_event_call_back) + 1,
          (uint8_t*)tag_event_call_back);

    AddRxTagEvent(tag_event_call_back, str_rx_acl_pre_call_back,
                  time_pre_stack_acl_call_back);
    ALOGE("%s: TS of pre_stack_acl_call_back %s", __func__, tag_event_call_back);
    rx_.AddBuffNode(strlen(tag_event_call_back) + 1,
          (uint8_t*)tag_event_call_back);

    AddRxTagEvent(tag_event_call_back, str_rx_acl_post_call_back,
                  time_post_stack_acl_call_back);
    ALOGE("%s: TS of post_stack_acl_call_back %s", __func__, tag_event_call_back);
    rx_.AddBuffNode(strlen(tag_event_call_back) + 1,
          (uint8_t*)tag_event_call_back);

    AddRxTagEvent(tag_event_call_back, str_rx_select_pre_call_back,
                  time_pre_select_call_back);
    ALOGE("%s: TS of pre_select_call_back %s", __func__, tag_event_call_back);
    rx_.AddBuffNode(strlen(tag_event_call_back) + 1,
          (uint8_t*)tag_event_call_back);

    AddRxTagEvent(tag_event_call_back, str_rx_select_post_call_back,
                  time_post_select_call_back);
    ALOGE("%s: TS of post_select_call_back %s", __func__, tag_event_call_back);
    rx_.AddBuffNode(strlen(tag_event_call_back) + 1,
          (uint8_t*)tag_event_call_back);

    rx_.DumpData(SOC_TO_HOST, reason);
  }
}

void RingBuffer::AddIbsCmd(short int dir, uint8_t ibs_data)
{
  if (dir == HOST_TO_SOC) {
    std::unique_lock<std::mutex> guard(tx_ring_buffer_mutex_);
    tx_.AddIbsData(ibs_data);
  }
  else {
    std::unique_lock<std::mutex> guard(rx_ring_buffer_mutex_);
    rx_.AddIbsData(ibs_data);
  }
}

PacketBuff :: PacketBuff()
{
  memset(buff_index_, 0, sizeof(buff_index_));
  active_buff_ = 0;

  map_code_reason_[REASON_SOC_CRASHED]= (char *)"SOC crashed";
  map_code_reason_[REASON_SOC_CRASHED_DIAG_SSR]=  (char *)"SOC crashed with diag initiated SSR";
  map_code_reason_[REASON_UNABLE_TO_WAKE_SOC]=  (char *)"Unable to wake SOC";
  map_code_reason_[REASON_PATCH_DLDNG_FAILED_SOCINIT]=  (char *)"SOC init failed during patch downloading";
  map_code_reason_[REASON_CLOSE_RCVD_DURING_INIT]=  (char *)"Close received from stack during SOC init";
  map_code_reason_[REASON_ERROR_READING_DATA_FROM_UART]=  (char *)"Error reading data from UART";
  map_code_reason_[REASON_WRITE_FAIL_SPCL_BUFF_CRASH_SOC]=  (char *)"Failed to write special bytes to crash SOC";
  map_code_reason_[REASON_SPURIOUS_WAKEUP_SOC_WAIT_TIMEOUT]=  (char *)"Spurious wake and SOC crash wait timeout";
  map_code_reason_[REASON_CMD_TIMEDOUT_SOC_WAIT_TIMEOUT]=  (char *)"Command timedout and SOC crash wait timeout";
  map_code_reason_[REASON_SSR_CMD_TIMEDOUT]=  (char *)"SSR due to command timed out";
  map_code_reason_[REASON_SSR_SPURIOUS_WAKEUP]=  (char *)"SSR due to spurious wakeup";
  map_code_reason_[REASON_SSR_INVALID_BYTES_RCVD]=  (char *)"Invalid HCI cmd type received";
  map_code_reason_[REASON_SSR_RCVD_LARGE_PKT_FROM_SOC]=  (char *)"Large packet received from SOC";
  map_code_reason_[REASON_DEFAULT_NONE]=  (char *)"Invalid reason";
  map_code_reason_[REASON_RX_THREAD_STRUCK]=  (char *)"Rx Thread Struck";
}

PacketBuff::~PacketBuff()
{
  memset(buff_index_, 0, sizeof(buff_index_));
  active_buff_ = 0;
  map_code_reason_.clear();
}

void PacketBuff::AddIbsData(uint8_t ibs_data)
{
  uint16_t log_len = TIME_STR_SIZE;

  if (log_len + 2 > max_buff_size_ - buff_index_[active_buff_]) {
    //switch active buffer if current active buffer is full.
    active_buff_ = (active_buff_ + 1) % 2;
    buff_index_[active_buff_] = 0;
    memset(&buff_[active_buff_][0], 0x00, max_buff_size_);
  }

  //Add time string to current active ring buffer.
  add_time_str((char*)&buff_[active_buff_][buff_index_[active_buff_]], NULL);
  buff_[active_buff_][TIME_STR_SIZE + buff_index_[active_buff_]] = ibs_data;
  buff_index_[active_buff_] += (TIME_STR_SIZE + 1);  //1 is for the IBS byte
}

void PacketBuff::AddBuffNode(HciPacketType type, uint16_t len, uint8_t *pkt)
{
  uint16_t log_len = TIME_STR_SIZE + len;

  if (log_len + 2 > max_buff_size_ - buff_index_[active_buff_]) {
    //switch active buffer if current active buffer is full.
    active_buff_ = (active_buff_ + 1) % 2;
    buff_index_[active_buff_] = 0;
    memset(&buff_[active_buff_][0], 0x00, max_buff_size_);
  }
  //Add time string to current active ring buffer.
  add_time_str((char*)&buff_[active_buff_][buff_index_[active_buff_]], NULL);
  buff_[active_buff_][TIME_STR_SIZE + buff_index_[active_buff_]] = (uint8_t)type;
  buff_index_[active_buff_] += (TIME_STR_SIZE + 1);  //1 is for the type byte

  memcpy(&buff_[active_buff_][buff_index_[active_buff_]], pkt, len);
  buff_index_[active_buff_] += len;
}

void PacketBuff::AddBuffNode(uint16_t len, uint8_t *pkt) {
  uint16_t log_len = TIME_STR_SIZE + len;

  if (log_len > max_buff_size_ - buff_index_[active_buff_]) {
    //switch active buffer if current active buffer is full.
    active_buff_ = (active_buff_ + 1) % 2;
    buff_index_[active_buff_] = 0;
    memset(&buff_[active_buff_][0], 0x00, max_buff_size_);
  }

  memcpy(&buff_[active_buff_][buff_index_[active_buff_]], pkt, len);
  buff_index_[active_buff_] += len;
}

void PacketBuff::DumpStatInfo(int dst_fd) {
  write(dst_fd, &buff_[(active_buff_ + 1) % 2][0], buff_index_[(active_buff_ + 1) % 2]);
  write(dst_fd, &buff_[active_buff_][0], buff_index_[active_buff_]);
  fsync(dst_fd);
}

void PacketBuff::DumpData(short int dir, ReasonCode reason)
{
#ifdef DUMP_RINGBUF_LOG
  struct timespec ts;
  struct timeval systs;
  char ts_buff[HCICMD_TIMESTAMP_LEN];
  char kts_buff[SYS_TIMESTAMP_LEN_MAX];
  char sts_buff[SYS_TIMESTAMP_LEN_MAX];
  char file_name[RING_BUFF_FILE_NAME_LEN];
  int fd;
  int buffsz1 = 0;
  int buffsz2 = 0;

  if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
    ALOGE("%s: Error in clock_gettime()", __func__);
  }

  gettimeofday(&systs, NULL);
  add_time_str(sts_buff, &systs);

  if (dir == RingBuffer::SOC_TO_HOST) {
    snprintf(kts_buff, sizeof(kts_buff),"%lu:%lu", ts.tv_sec, ts.tv_nsec);
    snprintf(ts_buff, sizeof(ts_buff),"Sys TS->%s Kernel TS->%s", sts_buff, kts_buff);
    ALOGE("%s: TS System %s TS kernel %lu:%lu", __func__, sts_buff, ts.tv_sec, ts.tv_nsec);
  } else {
    ALOGE("%s: Dumping tx data from static buffer", __func__);
  }

  // Check if something is there to dump. Avoid creating and dumping 0 byte files
  buffsz1 = buff_index_[(active_buff_ + 1) % 2];
  buffsz2 = buff_index_[active_buff_];
  if (((dir == RingBuffer::SOC_TO_HOST) && ((buffsz1 + buffsz2) <= IN_RINGBUFF_EVENTS_SIZE)) || // Ignore if only incoming event/data logs are present
      ((dir == RingBuffer::HOST_TO_SOC) && ((buffsz1 + buffsz2) == 0))) {
    ALOGE("%s: No data to dump", __func__);
    return;
  }

  logger_ = static_cast<void*>(Logger::Get());
  static_cast<Logger*>(logger_)->GetRingBufFileName(dir,file_name);

  fd = open(file_name, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);

  if (fd < 0) {
    ALOGE("%s: File open (%s) failed: errno: %d", __func__, file_name, errno);
    return;
  }

  if (buffsz1 > 0)
    write(fd, &buff_[(active_buff_ + 1) % 2][0], buffsz1);

  if (buffsz2 > 0)
    write(fd, &buff_[active_buff_][0], buffsz2);
  if (dir == RingBuffer::SOC_TO_HOST) {
    ALOGE("%s: Reason for dumping data->%s", __func__, GetReason(reason));
    write(fd, GetReason(reason), strlen(GetReason(reason))+1);
    write(fd, ts_buff, strlen(ts_buff)+1);
  }
  fsync(fd);
  close(fd);

  memset(buff_index_, 0, sizeof(buff_index_));
  active_buff_ = 0;
#endif /* DUMP_RINGBUF_LOG */
}

char* PacketBuff::GetReason(ReasonCode reason)
{
#ifdef DUMP_RINGBUF_LOG
  return (map_code_reason_.find(reason)->second);
#else
  return NULL;
#endif /* DUMP_RINGBUF_LOG */
}

void add_time_str(char *src, struct timeval* p_tval)
{
  unsigned short int t_hh, t_mm, t_ss;
  unsigned int temp = 0;
  struct timeval tval;

  if (!p_tval) {
    //Get current time stamp
    p_tval = &tval;
    gettimeofday(p_tval, NULL);
  }

  temp = p_tval->tv_sec;
  t_ss = temp%60;
  temp /= 60;
  t_mm = temp%60;
  temp /= 60;
  t_hh = temp%24;

  snprintf(src, TIME_STR_SIZE + 1, "%.2d:%.2d:%.2d:%.3d-", t_hh, t_mm, t_ss,
     (int)(p_tval->tv_usec / 1000));
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace bluetooth
}  // namespace hardware
}  // namespace android
