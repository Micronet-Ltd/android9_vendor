/*!
  @file
  qcril_qmi_ims_sms.h

  @brief

*/

/*===========================================================================

   Copyright (c) 2018 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef IMS_SMS_H
#define IMS_SMS_H

#include <vendor/qti/hardware/radio/ims/1.2/IImsRadio.h>
#include <hidl/HidlSupport.h>

using namespace vendor::qti::hardware::radio::ims::V1_2;

struct QcrilImsSmsAckData {
    QcrilImsSmsAckData() = delete;
    inline QcrilImsSmsAckData(uint32_t ref, ImsSmsDeliverStatusResult result):
        messageRef(ref), smsDeliverResult(result) { }
#if 0
    inline uint32_t getMessageRef() { return messageRef; }
    inline ImsSmsStatusReportResult getSmsDeliverStatus() { return smsDeliverResult; }
private:
#endif
    uint32_t messageRef;
    ImsSmsDeliverStatusResult smsDeliverResult;
};

struct QcrilImsSmsAckReportData {
    QcrilImsSmsAckReportData() = delete;
    inline QcrilImsSmsAckReportData(uint32_t ref, ImsSmsStatusReportResult result):
        messageRef(ref), smsStatusReport(result) { }
#if 0
    inline uint32_t getMessageRef() { return messageRef; }
    inline ImsSmsStatusReportResult getSmsStatusReport() { return smsStatusReport; }
private:
#endif
    uint32_t messageRef;
    ImsSmsStatusReportResult smsStatusReport;
};

struct QcrilImsSmsResponseData {
    uint32_t messageRef;
    ImsSmsSendStatusResult smsStatusResult;
    ImsSmsSendFailureReason reason;
};

#endif
