/*==========================================================================
Description
  It has implementation for logger class

# Copyright (c) 2017 Qualcomm Technologies, Inc.
# All Rights Reserved.
# Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#include <hidl/HidlSupport.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <utils/Log.h>
#include <cutils/properties.h>
#include "logger.h"
#include "uart_controller.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.bluetooth@1.0-logger"

#define SOC_DUMP_FIRST_PKT 0x0000
#define SOC_DUMP_LAST_PKT 0xFFFF

#define SOC_DUMP_PATH "/data/vendor/ssrdump/ramdump_bt_fw_crashdump_%.02d-%.02d-%.02d_%.02d-%.02d-%.02d.bin"
#define LOG_COLLECTION_DIR "/data/vendor/ssrdump/"
#define SOC_DUMP_PATH_BUF_SIZE 255
#define SOC_DUMP_SIGNATURE_SIZE 24

#define HAL_DUMP_FILE_SIZE 256
#define HAL_DUMP_TIMESTAMP_LENGTH 20
#define HAL_DUMP_TIMESTAMP_OFFSET 23
#define HAL_DUMP_SIZE 85
#ifdef USER_DEBUG
#define HAL_DUMP_ITERATION_LIMIT 80
#else
#define HAL_DUMP_ITERATION_LIMIT 10
#endif

#define BT_VND_SPECIFIC_EVENT 0xFF

#define BT_CONTROLLER_LOG        0x01
#define LOG_BT_MESSAGE_TYPE_VSTR     0x02
#define LOG_BT_MESSAGE_TYPE_PACKET   0x05
#define LOG_BT_MESSAGE_TYPE_MEM_DUMP 0x08
#define DIAG_SSR_BT_CMD     0x0007

#define LAST_SEQUENCE_NUM 0xFFFF

namespace android {
namespace hardware {
namespace bluetooth {
namespace V1_0 {
namespace implementation {

Logger * Logger :: instance_ = NULL;

Logger * Logger :: Get()
{
  if (!instance_) {
    instance_ = new Logger();
  }
  return instance_;
}

Logger :: Logger()
{
  /*To me initiated dynamically by config input*/
  ibs_log_level_ = MID;
  rx_log_level_ = MID;
  tx_log_level_ = MID;
  gen_log_level_ = MID;
  is_crash_dump_in_progress_ = false;
  crash_dump_status_cb_ = nullptr;
  dump_reason_ = REASON_SOC_CRASHED;
}

bool Logger :: IsSnoopLogEnabled(void)
{
  char value[PROPERTY_VALUE_MAX] = {'\0'};
  property_get("persist.vendor.service.bdroid.snooplog", value, "false");
  return (strcmp(value, "true") == 0);
}

bool Logger :: IsHciFwSnoopEnabled(void)
{
#ifdef USER_DEBUG
  return true;
#else
  char value[PROPERTY_VALUE_MAX] = {'\0'};
  property_get("persist.vendor.service.bdroid.fwsnoop", value, "false");
  return (strcmp(value, "true") == 0);
#endif
}

bool Logger::IsControllerLogPkt(HciPacketType type, const uint8_t *buff, uint16_t len)
{
  if (len < 3) {
    return false;
  } else if ((HCI_PACKET_TYPE_EVENT == type) &&
             (BT_VND_SPECIFIC_EVENT == buff[0]) &&
             (BT_CONTROLLER_LOG == buff[2])) {
    return true;
  } else {
    return false;
  }
}

bool Logger :: IsCrashDumpStarted(void)
{
  return is_crash_dump_in_progress_;
}

bool Logger:: RegisterCrashDumpCallback(CrashDumpStatusCallback crash_dump_cb)
{
  crash_dump_status_cb_ = crash_dump_cb;
  return true;
}


bool Logger :: ProcessRx(HciPacketType type, const uint8_t *buff, uint16_t len)
{
  bool ret  = true;

  switch (type) {
    case HCI_PACKET_TYPE_ACL_DATA:
#ifdef DUMP_RINGBUF_LOG
      ring_buff_.AddLog(RingBuffer::SOC_TO_HOST, type, (uint8_t*)buff, len);
#endif
      if (IsSnoopLogEnabled()) {
        diag_interface_.SendLogs(buff, len, LOG_BT_HCI_RX_ACL_C);
      }
      break;

    case HCI_PACKET_TYPE_EVENT:
      ret =  HandleHciEvent(type, (uint8_t*)buff, len);
      if (IsSnoopLogEnabled()) {
        diag_interface_.SendLogs(buff, len, LOG_BT_HCI_EV_C);
      }
      break;

    default:
      ret =  false;
      break;
  }
  return ret;
}

bool Logger :: ProcessTx(HciPacketType type, const uint8_t *buff, uint16_t len)
{
#ifdef DUMP_RINGBUF_LOG
  ring_buff_.AddLog(RingBuffer::HOST_TO_SOC, type, (uint8_t*)buff, len);
#endif

  if (IsSnoopLogEnabled()) {
    if (type == HCI_PACKET_TYPE_COMMAND)
      diag_interface_.SendLogs(buff, len, LOG_BT_HCI_CMD_C);
    else if (type == HCI_PACKET_TYPE_ACL_DATA)
      diag_interface_.SendLogs(buff, len, LOG_BT_HCI_TX_ACL_C);
  }
  return true;
}

bool Logger :: HandleHciEvent(HciPacketType type, uint8_t *buff, uint16_t len)
{
  bool ret = true;

  if ((buff[0] == BT_VND_SPECIFIC_EVENT) &&
      (buff[2] == BT_CONTROLLER_LOG)) {
    switch (buff[3]) {
      case LOG_BT_MESSAGE_TYPE_VSTR:
      case LOG_BT_MESSAGE_TYPE_PACKET:
#ifdef DUMP_RINGBUF_LOG
        ring_buff_.AddLog(RingBuffer::SOC_TO_HOST, type, buff, len);
#endif
        break;

      case LOG_BT_MESSAGE_TYPE_MEM_DUMP:
        if (isDiagSsrTriggered())
          dump_reason_ = REASON_SOC_CRASHED_DIAG_SSR;
        SaveSocMemDump(buff, len, dump_reason_);
        break;

      default:
        ret = false;
        break;
    }
  } else {
#ifdef DUMP_RINGBUF_LOG
    ring_buff_.AddLog(RingBuffer::SOC_TO_HOST, type, buff, len);
#endif
  }
  return ret;
}

const diagpkt_user_table_entry_type Logger :: ssr_bt_tbl_[] = {
  { DIAG_SSR_BT_CMD, DIAG_SSR_BT_CMD, SsrBtHandler },
};

void Logger :: UnlockRingbufferMutex()
{
#ifdef DUMP_RINGBUF_LOG
  ring_buff_.UnlockMutex();
#endif
}

void * Logger :: SsrBtHandler(void *req_pkt, uint16_t pkt_len)
{
  Logger *logger = Logger :: Get();

  if (logger) {
    return logger->diag_interface_.SsrBtHandler(req_pkt, pkt_len);
  } else {
    return nullptr;
  }
}

void Logger :: SaveSocMemDump(uint8_t *eventBuf, uint16_t packet_len, ReasonCode reason)
{
  static unsigned int dump_size = 0, total_size = 0;
  unsigned short seq_num = 0;
  static unsigned short seq_num_cnt = 0;
  uint8_t *dump_ptr = NULL;
  static char *temp_buf, *p, path[SOC_DUMP_PATH_BUF_SIZE + 1];
  static int dump_fd = -1;
  char nullBuff[255] = { 0 };

  dump_ptr = &eventBuf[7];
  seq_num = eventBuf[4] | (eventBuf[5] << 8);
  packet_len -= 7;

  ALOGV("%s: LOG_BT_MESSAGE_TYPE_MEM_DUMP (%d) ", __func__, seq_num);

  if ((seq_num != seq_num_cnt) && seq_num != LAST_SEQUENCE_NUM) {
    ALOGE("%s: current sequence number: %d, expected seq num: %d ", __func__,
          seq_num, seq_num_cnt);
  }

  if (seq_num == 0x0000) {
    PrepareDumpProcess();
#ifdef DUMP_RINGBUF_LOG
    ring_buff_.DumpData(reason);
#endif

#ifdef DUMP_IPC_LOG
    ipc_log_.DumpUartLogs();
#endif

    dump_size = (unsigned int)
                (eventBuf[7] | (eventBuf[8] << 8) | (eventBuf[9] << 16) | (eventBuf[10] << 24));
    dump_ptr = &eventBuf[11];
    total_size = seq_num_cnt = 0;
    packet_len -= 4;

    //memset(path, 0, SOC_DUMP_PATH_BUF_SIZE);
    /* first pack has total ram dump size (4 bytes) */
    ALOGD("%s: Crash Dump Start - total Size: %d ", __func__, dump_size);
    is_crash_dump_in_progress_ = true;
    p = temp_buf = (char*)malloc(dump_size);
    if (p != NULL) {
      memset(p, 0, dump_size);
    } else {
      ALOGE("Failed to allocate mem for the crash dump size: %d", dump_size);
    }

    GetCrashDumpFileName(path);
    dump_fd = open(path, O_CREAT | O_SYNC | O_WRONLY,  S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (dump_fd < 0) {
      ALOGE("%s: File open (%s) failed: errno: %d", __func__, path, errno);
      seq_num_cnt++;
      return;
    }
    ALOGV("%s: File Open (%s) successfully ", __func__, path);
  }

  if (dump_fd >= 0) {
    for (; (seq_num > seq_num_cnt) && (seq_num != LAST_SEQUENCE_NUM); seq_num_cnt++) {
      ALOGE("%s: controller missed packet : %d, write null (%d) into file ",
            __func__, seq_num_cnt, packet_len);

      if (p != NULL) {
        memcpy(temp_buf, nullBuff, packet_len);
        temp_buf = temp_buf + packet_len;
      }
    }

    if (p != NULL) {
      memcpy(temp_buf, dump_ptr, packet_len);
      temp_buf = temp_buf + packet_len;
    }
    total_size += packet_len;
  }

  seq_num_cnt++;
  if (seq_num == LAST_SEQUENCE_NUM && p != NULL) {
    ALOGE("Writing crash dump of size %d bytes", total_size);
    write(dump_fd, p, total_size);
    free(p);
    temp_buf = NULL;
    p = NULL;
    seq_num_cnt = 0;
  }

  if (seq_num == LAST_SEQUENCE_NUM) {
    if (dump_fd >= 0) {
      if ( fsync(dump_fd) < 0 ) {
        ALOGE("%s: File flush (%s) failed: %s, errno: %d", __func__,
              path, strerror(errno), errno);
      }
      close(dump_fd);
      dump_fd = -1;

      //sync the directory as well
      int dir_fd;
      DIR *dirp;
      if (!(dirp = opendir(LOG_COLLECTION_DIR))) {
        ALOGE("%s: Dir open (%s) failed: %s, errno: %d", __func__,
              LOG_COLLECTION_DIR, strerror(errno), errno);
        return;
      }
      dir_fd = dirfd(dirp);
      if (dir_fd == -1) {
        ALOGE("%s: Failed to find dir fd: %s, errno: %d\n", __func__,
            strerror(errno), errno);
        return;
      }

      if (fsync(dir_fd) == -1) {
        ALOGE("%s: Dir flush (%s) failed: %s, errno: %d", __func__,
              LOG_COLLECTION_DIR, strerror(errno), errno);
        return;
      }

      closedir(dirp);
      sync();
      ALOGI("%s: Write crashdump successfully : \n"
            "\t\tFile: %s\n\t\tdump_size: %d\n\t\twritten_size: %d",
            __func__, path, dump_size, total_size );
      int cntr = 0;
      size_t readsize;
      unsigned char *dumpinfo, *tempptr;
      uint32_t ucFilename;
      uint32_t uLinenum;
      uint32_t uPCAddr;
      uint32_t uResetReason;
      uint32_t uBuildVersion;
      dump_fd = -1;
      int i = 0;
      char filenameBuf [SOC_DUMP_PATH_BUF_SIZE];
      memset(filenameBuf, 0, SOC_DUMP_PATH_BUF_SIZE);
      if (-1 != (dump_fd = open(path, O_RDONLY))) {
        if (NULL != (dumpinfo = (unsigned char*)malloc(SOC_DUMP_SIGNATURE_SIZE))) {
          tempptr = dumpinfo;
          lseek(dump_fd, 0xFEE0, SEEK_SET);
          readsize = SOC_DUMP_SIGNATURE_SIZE;
          while (readsize) {
            cntr = read(dump_fd, (void*)tempptr, readsize);
            tempptr += cntr;
            readsize -= cntr;
          }

          tempptr = dumpinfo;
          memcpy(&ucFilename, tempptr, 4); tempptr += 4;
          memcpy(&uLinenum, tempptr, 4); tempptr += 4;
          memcpy(&uPCAddr, tempptr, 4); tempptr += 4;
          memcpy(&uResetReason, tempptr, 4); tempptr += 4;
          memcpy(&uBuildVersion, tempptr, 4); tempptr += 4;

          if (0 != ucFilename) {
            lseek(dump_fd, (off_t)ucFilename, SEEK_SET);
            cntr = read(dump_fd, (void*)filenameBuf, SOC_DUMP_PATH_BUF_SIZE);
            while (i < SOC_DUMP_PATH_BUF_SIZE && filenameBuf[i++] != '\0' ) ;
            if (i < SOC_DUMP_PATH_BUF_SIZE) {
              ALOGE("Filename::%s\n", filenameBuf);
            }
          }
          ALOGE("Linenum::%d\n", uLinenum);
          ALOGE("PC Addr::0x%x\n", uPCAddr);
          //ALOGE("Reset reason::%s\n", get_reset_reason_str(uResetReason));
          ALOGE("Build Version::0x%x\n", uBuildVersion);
        }
        if (NULL != dumpinfo) free(dumpinfo);
        close(dump_fd);
      }
    }

    is_crash_dump_in_progress_ = false;
  }

  if ( (0 == seq_num || LAST_SEQUENCE_NUM == seq_num) && crash_dump_status_cb_) {
    crash_dump_status_cb_(is_crash_dump_in_progress_);
  }
}

void Logger :: Init(HciTransport *hci_transport)
{
  diag_interface_.Init(hci_transport);
  DIAGPKT_DISPATCH_TABLE_REGISTER(DIAG_SUBSYS_BT, ssr_bt_tbl_);
}

void Logger :: Cleanup(void)
{
  diag_interface_.CleanUp();
  dump_init = false;
}

void Logger :: GetCrashDumpFileName(char* path)
{
  snprintf(path, SOC_DUMP_PATH_BUF_SIZE, SOC_DUMP_PATH, time_year, time_month, time_day, time_hour, time_min, time_sec);
}

#ifdef DUMP_IPC_LOG
void Logger :: GetUartDumpFilename(short int uart_dump, char* path)
{
  switch (uart_dump){
  case IpcLogs::TX_LOG:
    snprintf(path, UART_IPC_PATH_BUF_SIZE, UART_IPC_TX_LOGS_DEST_PATH, time_year, time_month, time_day, time_hour, time_min, time_sec);
    break;
  case IpcLogs::RX_LOG:
    snprintf(path, UART_IPC_PATH_BUF_SIZE, UART_IPC_RX_LOGS_DEST_PATH, time_year, time_month, time_day, time_hour, time_min, time_sec);
    break;
  case IpcLogs::STATE_LOG:
    snprintf(path, UART_IPC_PATH_BUF_SIZE, UART_IPC_STATE_LOGS_DEST_PATH, time_year, time_month, time_day, time_hour, time_min, time_sec);
    break;
  case IpcLogs::POWER_LOG:
    snprintf(path, UART_IPC_PATH_BUF_SIZE, UART_IPC_PWR_LOGS_DEST_PATH, time_year, time_month, time_day, time_hour, time_min, time_sec);
    break;
  }
}
#endif

#ifdef DUMP_RINGBUF_LOG
void Logger :: GetRingBufFileName(short int dir, char* path)
{
  switch (dir) {
  case  RingBuffer::HOST_TO_SOC:
    snprintf(path, RING_BUFF_FILE_NAME_LEN, RING_BUFF_LOG_TX_FILE_NAME, time_year, time_month, time_day, time_hour, time_min, time_sec); //HOST_TO_SOC
    break;
  case RingBuffer::SOC_TO_HOST:
    snprintf(path, RING_BUFF_FILE_NAME_LEN, RING_BUFF_LOG_RX_FILE_NAME, time_year, time_month, time_day, time_hour, time_min, time_sec); //SOC_TO_HOST
    break;
  }
}
#endif

bool Logger::PrepareDumpProcess()
{
  SetDumpTimestamp();
#ifndef DONT_DELETE_DUMPS_SET
  DeleteDumpsIfRequired();
#endif
  return true;
}

void Logger::SetDumpTimestamp()
{
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  time_year = tm.tm_year + 1900;
  time_month = tm.tm_mon+ 1;
  time_day = tm.tm_mday;
  time_hour = tm.tm_hour;
  time_min = tm.tm_min;
  time_sec = tm.tm_sec;
}

void Logger::SaveCrashReason(ReasonCode reason)
{
  dump_reason_ = reason;
}

void Logger :: DeleteDumpsIfRequired()
{
  struct DIR* p_dir;
  struct dirent* p_dirent;
  char *first_entry = NULL;
  int count = 0;

#ifdef DUMP_IPC_LOG
  char *look_for = (char *)"uart_ipc_rx";
#else
  char *look_for = (char *)"fw_crashdump";
#endif

  p_dir = opendir(LOG_COLLECTION_DIR);
  if (p_dir == NULL) {
    ALOGE("%s: Cannot open dump location %s", __func__, LOG_COLLECTION_DIR);
    return;
  }
  while ((p_dirent = readdir(p_dir)) != NULL) {
    if (strstr(p_dirent->d_name, look_for)) {
      count++;
      if(count == 1) {
        first_entry = p_dirent->d_name;
      }
    }
  }
  closedir(p_dir);
  if(count < HAL_DUMP_ITERATION_LIMIT) {
     return;
  } else {
     DeleteDumps(first_entry);
  }
}

void Logger :: DeleteDumps(char *first_entry)
{
  struct DIR* p_dir;
  struct dirent* p_dirent;
  int ret = 0;

#ifdef DUMP_IPC_LOG
  char *look_for = (char *)"uart_ipc_rx";
#else
  char *look_for = (char *)"fw_crashdump";
#endif
  char timestamp[HAL_DUMP_TIMESTAMP_LENGTH];
  char path[HAL_DUMP_SIZE];
  char oldest_file[HAL_DUMP_FILE_SIZE];

  strlcpy(oldest_file, first_entry, HAL_DUMP_FILE_SIZE);

  p_dir = opendir(LOG_COLLECTION_DIR);
  if (p_dir == NULL) {
    ALOGE("%s: Cannot open dump location %s", __func__, LOG_COLLECTION_DIR);
    return;
  }

  while ((p_dirent = readdir(p_dir)) != NULL) {
    if (strstr(p_dirent->d_name, look_for) ) {
      if(strcmp(p_dirent->d_name, oldest_file) < 0) {
        strlcpy(oldest_file, p_dirent->d_name, HAL_DUMP_FILE_SIZE);
      }
    }
  }
  closedir(p_dir);

#ifdef DUMP_IPC_LOG
  strlcpy(timestamp, oldest_file + HAL_DUMP_TIMESTAMP_OFFSET, HAL_DUMP_TIMESTAMP_LENGTH);
#else
  strlcpy(timestamp, oldest_file + HAL_DUMP_TIMESTAMP_OFFSET + 1, HAL_DUMP_TIMESTAMP_LENGTH);
#endif

  p_dir = opendir(LOG_COLLECTION_DIR);
  if (p_dir == NULL) {
    ALOGE("%s: Cannot open dump location %s", __func__, LOG_COLLECTION_DIR);
    return;
  }

  while ((p_dirent = readdir(p_dir)) != NULL) {
   if (strstr(p_dirent->d_name, timestamp)) {
     strlcpy(path, LOG_COLLECTION_DIR, HAL_DUMP_SIZE);
     strlcat(path, p_dirent->d_name, HAL_DUMP_SIZE);
     ALOGE("%s: Deleting oldest dump file: %s", __func__, path);
     ret = remove(path);
     if(ret < 0) ALOGE("%s: Cannot delete file %s", __func__, path);
   }
  }
  closedir(p_dir);
}

bool Logger :: isSsrTriggered()
{
  return diag_interface_.isSsrTriggered();
}

bool Logger :: isDiagSsrTriggered()
{
  return diag_interface_.isDiagSsrTriggered();
}

void Logger :: ResetSsrTriggeredFlag()
{
  return diag_interface_.ResetSsrTriggeredFlag();
}

bool Logger :: SetSsrTriggeredFlag()
{
  return diag_interface_.SetSsrTriggeredFlag();
}

} // namespace implementation
} // namespace V1_0
} // namespace bluetooth
} // namespace hardware
} // namespace android
