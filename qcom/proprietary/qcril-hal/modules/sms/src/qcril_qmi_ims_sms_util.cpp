/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#
#  Not a Contribution.
#  Apache license notifications and license are retained
#  for attribution purposes only.
#
#  Copyright (C) 2008 The Android Open Source Project
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#******************************************************************************/

#include "qcril_qmi_ims_sms_util.h"
#include "framework/Log.h"

#define TAG                 "SMS_UTILS"
#define HIGHER_4BIT(data)   (((data) & 0xf0) >> 4)
#define LOWER_4BIT(data)    ((data) & 0x0f)
#define INVALID_HEX_CHAR    0

#define IS_BIG_ENDIAN()     \
({ \
  union {uint32_t d32; uint8_t d8;} data = {.d32 = 1}; \
  !data.d8; \
})

static inline uint32_t revertEndian(uint32_t data)
{
    uint32_t result = (data & 0xff000000) >> 24;

    result |= ((data & 0x00ff0000) >> 8);
    result |= ((data & 0x0000ff00) << 8);
    result |= ((data & 0x000000ff) << 24);

    return result;
}

static uint32_t convertCurrentToBigEndian(uint32_t data)
{
    if (!IS_BIG_ENDIAN())
        return revertEndian(data);
    else
        return data;
}

static uint32_t convertBigEndianToCurrent(uint32_t data)
{
    if (!IS_BIG_ENDIAN())
        return revertEndian(data);
    else
        return data;
}

static void getBigEndianArrayFrom32(uint8_t* array32, uint32_t value)
{
    if (!array32) return;

    array32[0] = (value & 0xff000000) >> 24;
    array32[1] = (value & 0x00ff0000) >> 16;
    array32[2] = (value & 0x0000ff00) >> 8;
    array32[3] = (value & 0x000000ff);
}

static inline char IntToHexChar(uint8_t byte_data)
{
    if (byte_data >= 0 && byte_data <= 9)
        return ('0' + byte_data);
    if (byte_data > 9 && byte_data <= 15)
        return ('a' + byte_data - 10);
    return INVALID_HEX_CHAR;
}

static char* convertBytesToHexString(const uint8_t* bytePayload, size_t size)
{
    size_t hex_len = (size << 1) + 1;
    char* hex_buf;

    if ((hex_buf = new char[hex_len]) == nullptr)
        return nullptr;

    unsigned int hex_idx = 0;
    for (unsigned int byte_idx = 0; byte_idx < size; byte_idx++)
    {
        uint8_t tmp;
        // higher bit is in the earlier index
        if ((tmp = IntToHexChar(HIGHER_4BIT(bytePayload[byte_idx]))) == INVALID_HEX_CHAR)
        {
            delete []hex_buf;
            return nullptr;
        }
        hex_buf[hex_idx++] = tmp;
        if ((tmp = IntToHexChar(LOWER_4BIT(bytePayload[byte_idx]))) == INVALID_HEX_CHAR)
        {
            delete []hex_buf;
            return nullptr;
        }
        hex_buf[hex_idx++] = tmp;
    }
    hex_buf[hex_idx] = 0;

    return hex_buf;
}

static uint8_t convertDtmfToAscii(uint8_t dtmfDigit)
{
    uint8_t asciiDigit;

    switch (dtmfDigit) {
    case  0: asciiDigit = 'D'; break;
    case  1: asciiDigit = '1'; break;
    case  2: asciiDigit = '2'; break;
    case  3: asciiDigit = '3'; break;
    case  4: asciiDigit = '4'; break;
    case  5: asciiDigit = '5'; break;
    case  6: asciiDigit = '6'; break;
    case  7: asciiDigit = '7'; break;
    case  8: asciiDigit = '8'; break;
    case  9: asciiDigit = '9'; break;
    case 10: asciiDigit = '0'; break;
    case 11: asciiDigit = '*'; break;
    case 12: asciiDigit = '#'; break;
    case 13: asciiDigit = 'A'; break;
    case 14: asciiDigit = 'B'; break;
    case 15: asciiDigit = 'C'; break;
    default:
        asciiDigit = 32; // Invalid DTMF code
        break;
    }

    return asciiDigit;
}

static bool convertPayloadToCdmaFormat(const uint8_t* payload, size_t size,
    RIL_CDMA_SMS_Message& cdmaMsg)
{
    if (!payload || !size) return false;
    // XXX take care alignment!
    // How to implement data stream in c++?
    uint32_t index = 0;
    // read the first 3 int
    index +=  sizeof(int) * 3;
    if (index > size) return false;
    auto int_buf = reinterpret_cast<const int*>(payload);
    cdmaMsg.uTeleserviceID = convertBigEndianToCurrent(int_buf[0]);
    cdmaMsg.bIsServicePresent = (convertBigEndianToCurrent(int_buf[1]) == 0) ? 0 : 1;
    cdmaMsg.uServicecategory = convertBigEndianToCurrent(int_buf[2]);

    // read bytes to fill sAddress
    if (index + 5 > size) return false;
    cdmaMsg.sAddress.digit_mode = static_cast<RIL_CDMA_SMS_DigitMode>(payload[index++]);
    cdmaMsg.sAddress.number_mode = static_cast<RIL_CDMA_SMS_NumberMode>(payload[index++]);
    cdmaMsg.sAddress.number_type = static_cast<RIL_CDMA_SMS_NumberType>(payload[index++]);
    cdmaMsg.sAddress.number_plan = static_cast<RIL_CDMA_SMS_NumberPlan>(payload[index++]);
    cdmaMsg.sAddress.number_of_digits = payload[index++];
    if (index + cdmaMsg.sAddress.number_of_digits > size)
        return false;
    memcpy(cdmaMsg.sAddress.digits, payload + index,
            cdmaMsg.sAddress.number_of_digits);
    index += cdmaMsg.sAddress.number_of_digits;

    // read bytes to fill sSubAddress
    if (index + 3 > size) return false;
    cdmaMsg.sSubAddress.subaddressType =
        static_cast<RIL_CDMA_SMS_SubaddressType>(payload[index++]);
    cdmaMsg.sSubAddress.odd = (payload[index++] == 0) ? 0 : 1;
    cdmaMsg.sSubAddress.number_of_digits = payload[index++];
    if (index + cdmaMsg.sSubAddress.number_of_digits > size)
        return false;
    memcpy(cdmaMsg.sSubAddress.digits, payload + index,
            cdmaMsg.sSubAddress.number_of_digits);
    index += cdmaMsg.sSubAddress.number_of_digits;

    // read bytes for bear data
    if (index + 1 > size) return false;
    cdmaMsg.uBearerDataLen = payload[index++];
    if (index + cdmaMsg.uBearerDataLen > size) return false;
    memcpy(cdmaMsg.aBearerData, payload + index, cdmaMsg.uBearerDataLen);

    QCRIL_LOG_INFO("payload size = %d, read size = %d", size,
        index + cdmaMsg.uBearerDataLen);

    return true;
}

bool convertCdmaFormatToPseudoPdu(RIL_CDMA_SMS_Message& cdmaMsg, std::vector<uint8_t>& payload)
{
    const uint32_t MESSAGE_TYPE_POINT_TO_POINT = 0x00;
    const uint32_t MESSAGE_TYPE_BROADCAST      = 0x01;
    const uint32_t MESSAGE_TYPE_ACKNOWLEDGE    = 0x02;
    uint32_t messageType;

    if (cdmaMsg.bIsServicePresent) {
        messageType = MESSAGE_TYPE_BROADCAST;
    } else {
        if (cdmaMsg.uTeleserviceID == 0x00) {
            messageType = MESSAGE_TYPE_ACKNOWLEDGE;
        } else {
            messageType = MESSAGE_TYPE_POINT_TO_POINT;
        }
    }
    // fill the first three int
    uint32_t item = convertCurrentToBigEndian(messageType);
    uint8_t* base = reinterpret_cast<uint8_t*>(&item);
    payload.insert(payload.end(), base, base + 4);

    item = convertCurrentToBigEndian(cdmaMsg.uTeleserviceID);
    payload.insert(payload.end(), base, base + 4);

    item = convertCurrentToBigEndian(cdmaMsg.uServicecategory);
    payload.insert(payload.end(), base, base + 4);

    // fill the sAddress
    payload.push_back(cdmaMsg.sAddress.digit_mode);
    payload.push_back(cdmaMsg.sAddress.number_mode);
    payload.push_back(cdmaMsg.sAddress.number_type);
    payload.push_back(cdmaMsg.sAddress.number_plan);
    payload.push_back(cdmaMsg.sAddress.number_of_digits);
    // 4-bit DTMF
    if (cdmaMsg.sAddress.digit_mode == 0) {
        for (int index = 0; index < cdmaMsg.sAddress.number_of_digits; index++)
            payload.push_back(convertDtmfToAscii(cdmaMsg.sAddress.digits[index]));
    } else {
        payload.insert(payload.end(), cdmaMsg.sAddress.digits,
            cdmaMsg.sAddress.digits + cdmaMsg.sAddress.number_of_digits);
    }

    uint8_t array32[4];
    // bearerReply
    getBigEndianArrayFrom32(array32, 1);
    payload.insert(payload.end(), array32, array32 + 4);
    // replySeqNo (not supported)
    payload.push_back(0);
    // errorClass (not supported)
    payload.push_back(0);
    // causeCode (not supported)
    payload.push_back(0);

    // bear Data length
    getBigEndianArrayFrom32(array32, cdmaMsg.uBearerDataLen);
    payload.insert(payload.end(), array32, array32 + 4);
    payload.insert(payload.end(), cdmaMsg.aBearerData,
        cdmaMsg.aBearerData + cdmaMsg.uBearerDataLen);

    return true;
}

RIL_IMS_SMS_Message* getRILImsSmsFormatPayload(const V1_2::ImsSmsMessage& imsSms)
{
    auto ims_ril_msg = new RIL_IMS_SMS_Message();
    if (ims_ril_msg == nullptr)
        return nullptr;

    memset(ims_ril_msg, 0, sizeof(*ims_ril_msg));
    ims_ril_msg->retry = (imsSms.shallRetry) ? 1 : 0;
    ims_ril_msg->messageRef = imsSms.messageRef;

    if (imsSms.format == "3gpp") {
        ims_ril_msg->tech = RADIO_TECH_3GPP;
        char** gsm_message = new char*[2];
        if (gsm_message) {
            if (imsSms.smsc.size() != 0)
                // use "free()" to release the memory
                gsm_message[0] = strdup(imsSms.smsc.c_str());
            else
                gsm_message[0] = nullptr;

            gsm_message[1] = convertBytesToHexString(imsSms.pdu.data(), imsSms.pdu.size());
        }
        ims_ril_msg->message.gsmMessage = gsm_message;
        // if no message payload, release the memory
        if (!gsm_message || !gsm_message[1]) {
            freeRILImsSmsFormatPayload(ims_ril_msg);
            return nullptr;
        }
    } else if (imsSms.format == "3gpp2") {
        ims_ril_msg->tech = RADIO_TECH_3GPP2;
        auto cdma_msg = new RIL_CDMA_SMS_Message;
        if (cdma_msg) {
            bool result = convertPayloadToCdmaFormat(imsSms.pdu.data(), imsSms.pdu.size(), *cdma_msg);
            if (!result) {
                QCRIL_LOG_ERROR("Failed to parse CDMA payload");
                delete cdma_msg;
                cdma_msg = nullptr;
            }
        }
        ims_ril_msg->message.cdmaMessage = cdma_msg;
        if (!cdma_msg) {
            freeRILImsSmsFormatPayload(ims_ril_msg);
            return nullptr;
        }
    } else {
        delete ims_ril_msg;
        return nullptr;
    }

    return ims_ril_msg;
}

void freeRILImsSmsFormatPayload(RIL_IMS_SMS_Message* ril_ims_msg)
{
    if (ril_ims_msg) {
        if (ril_ims_msg->tech == RADIO_TECH_3GPP2) {
            if (ril_ims_msg->message.cdmaMessage) {
                delete ril_ims_msg->message.cdmaMessage;
                ril_ims_msg->message.cdmaMessage = nullptr;
            }
        }
        if (ril_ims_msg->tech == RADIO_TECH_3GPP) {
            if (ril_ims_msg->message.gsmMessage) {
                if (ril_ims_msg->message.gsmMessage[0])
                    free(ril_ims_msg->message.gsmMessage[0]); // release strdup memory
                if (ril_ims_msg->message.gsmMessage[1])
                    delete []ril_ims_msg->message.gsmMessage[1];
                delete []ril_ims_msg->message.gsmMessage;
                ril_ims_msg->message.gsmMessage = nullptr;
            }
        }
        delete ril_ims_msg;
    }
}

// only for debug purpose
void dumpRILCdmaFormat(RIL_CDMA_SMS_Message& cdmaMsg)
{
    QCRIL_LOG_INFO("uTeleserviceID: %d", cdmaMsg.uTeleserviceID);
    QCRIL_LOG_INFO("bIsServicePresent: %d", cdmaMsg.bIsServicePresent);
    QCRIL_LOG_INFO("uServicecategory: %d", cdmaMsg.uServicecategory);

    QCRIL_LOG_INFO("address digit_mode: %d", cdmaMsg.sAddress.digit_mode);
    QCRIL_LOG_INFO("address number_mode: %d", cdmaMsg.sAddress.number_mode);
    QCRIL_LOG_INFO("address number_type: %d", cdmaMsg.sAddress.number_type);
    QCRIL_LOG_INFO("address number_plan: %d", cdmaMsg.sAddress.number_plan);
    QCRIL_LOG_INFO("address number_of_digits: %d", cdmaMsg.sAddress.number_of_digits);

    QCRIL_LOG_INFO("subaddress type: %d", cdmaMsg.sSubAddress.subaddressType);
    QCRIL_LOG_INFO("subaddress odd: %d", cdmaMsg.sSubAddress.odd);
    QCRIL_LOG_INFO("subaddress number_of_digits: %d", cdmaMsg.sSubAddress.number_of_digits);

    QCRIL_LOG_INFO("uBearerDataLen: %d", cdmaMsg.uBearerDataLen);
}
