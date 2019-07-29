/*
 * Copyright (c) 2010 - 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "QComExtractorFactory"

#include "MMParserExtractor.h"

namespace android {

#define MIN_FORMAT_HEADER_SIZE 1024
#define MAX_FORMAT_HEADER_SIZE (128*1024)

#define CREATOR(_x_) [](DataSourceBase *source, void *meta) -> MediaExtractor* { return new MMParserExtractor(source, meta, _x_); }
#define SNIFF_ENTRY(_flag_, _type_, _format_) {_flag_, _type_, _format_, CREATOR(_format_)}

typedef struct
{
    int flag;
    const char* type;
    FileSourceFileFormat format;
    MediaExtractor::CreatorFunc creator;
} SniffInfo;

static const SniffInfo sniffArray[] = {
  SNIFF_ENTRY(PARSER_OGG,    "OGG",    FILE_SOURCE_OGG),
  SNIFF_ENTRY(PARSER_AVI,    "AVI",    FILE_SOURCE_AVI),
  SNIFF_ENTRY(PARSER_WAV,    "WAV",    FILE_SOURCE_WAV),
  SNIFF_ENTRY(PARSER_DSF,    "DSF",    FILE_SOURCE_DSF),
  SNIFF_ENTRY(PARSER_DSDIFF, "DSDIFF", FILE_SOURCE_DSDIFF),
  SNIFF_ENTRY(PARSER_AC3,    "AC3",    FILE_SOURCE_AC3),
  SNIFF_ENTRY(PARSER_ASF,    "ASF",    FILE_SOURCE_ASF),
  SNIFF_ENTRY(PARSER_AMR_NB, "AMRNB",  FILE_SOURCE_AMR_NB),
  SNIFF_ENTRY(PARSER_AMR_WB, "AMRWB",  FILE_SOURCE_AMR_WB),
  SNIFF_ENTRY(PARSER_MKV,    "MKV",    FILE_SOURCE_MKV),
  SNIFF_ENTRY(PARSER_MOV,    "MOV",    FILE_SOURCE_MOV),
  SNIFF_ENTRY(PARSER_3GP,    "3GP",    FILE_SOURCE_MPEG4),
  SNIFF_ENTRY(PARSER_QCP,    "QCP",    FILE_SOURCE_QCP),
#if defined(FLAC_OFFLOAD_ENABLED) || defined(QTI_FLAC_DECODER)
  SNIFF_ENTRY(PARSER_FLAC,   "FLAC",   FILE_SOURCE_FLAC),
#endif
  SNIFF_ENTRY(PARSER_FLV,    "FLV",    FILE_SOURCE_FLV),
  SNIFF_ENTRY(PARSER_MP2TS,  "MP2TS",  FILE_SOURCE_MP2TS),
  SNIFF_ENTRY(PARSER_3G2,    "3G2",    FILE_SOURCE_3G2),
  SNIFF_ENTRY(PARSER_MP2PS,  "MP2PS",  FILE_SOURCE_MP2PS),
  SNIFF_ENTRY(PARSER_AIFF,   "AIFF",   FILE_SOURCE_AIFF),
  SNIFF_ENTRY(PARSER_APE,    "APE",    FILE_SOURCE_APE),
  SNIFF_ENTRY(PARSER_AAC,    "AAC",    FILE_SOURCE_AAC),
  SNIFF_ENTRY(PARSER_MP3,    "MP3",    FILE_SOURCE_MP3),
  SNIFF_ENTRY(PARSER_DTS,    "DTS",    FILE_SOURCE_DTS)
};

#define SNIFF_ARRAY_SIZE (sizeof(sniffArray)/sizeof(SniffInfo))

#define ID3V2_HEADER_SIZE 10
/*! =========================================================================
 * @brief       Get ID3 tag size. This routine is used to skip ID3 tag data
 *              while sniffing.
 *
 * @param
 *   source[in] : Data source to read from.
 *
 * @return        ID3 tag size.
 * =========================================================================*/
uint64_t GetID3Size(DataSourceBase *source)
{
    uint64_t offset = 0;
    //Check if file has ID3Tag present before AAC/MP3/FLAC/APE file header.
    for(;;) {
        uint8_t pProcessBuff[ID3V2_HEADER_SIZE];

        //read atleast 10 bytes of data
        if(source->readAt(offset,pProcessBuff,sizeof(pProcessBuff))
           < (ssize_t)sizeof(pProcessBuff)) {
            break;
        }

        //Stop if there are no more ID3 tags
        if(memcmp("ID3",pProcessBuff,strlen("ID3")))
            break;

        //Calculate size and skip ID3 tag
        //skip 6 byte = ID3(3byte)+ Ver(2byte)+ Flag (1byte)
        uint64_t ulID3TagSize = 0;
        for(int i = 6; i < ID3V2_HEADER_SIZE; i++) {
            ulID3TagSize = (ulID3TagSize << 7) | (pProcessBuff[i] & 0x7F);
        }
        ulID3TagSize += ID3V2_HEADER_SIZE;

        offset += ulID3TagSize;
    }
    return offset;
}

static MediaExtractor::CreatorFunc Sniff(DataSourceBase *source, float *confidence,
                                         void **, MediaExtractor::FreeMetaFunc *) {

    uint8_t* sniffBuffer = (uint8_t*)malloc(sizeof(uint8_t) * MAX_FORMAT_HEADER_SIZE);
    if (!sniffBuffer)
    {
        LOGE("malloc for sniff buffer failed");
        return NULL;
    }

    bool sniffSuccess = false;
    bool isEOS = false;
    int  parser_flags = 0;
    char property_value[PROPERTY_VALUE_MAX] = {0};
    FileSourceStatus status = FILE_SOURCE_INVALID;
    ssize_t sniffBufferSize = 0;
    uint32_t requiredSize = 0;
    uint64_t offset = GetID3Size(source);

    property_get("vendor.mm.enable.qcom_parser", property_value, "0");
    parser_flags = atoi(property_value);
    LOGV("Parser_Flags == %x",parser_flags);
    int index = 0;
    for (index = 0; index < SNIFF_ARRAY_SIZE; ++index) {
        const SniffInfo *sniff = &sniffArray[index];
        if (!(sniff->flag & parser_flags))
          continue;

        LOGV("Sniff %s ==>", sniff->type);
        switch (sniff->format)
        {
            case FILE_SOURCE_AAC:
            case FILE_SOURCE_MP3:
            case FILE_SOURCE_FLAC:
            case FILE_SOURCE_APE:
            case FILE_SOURCE_DTS:
              requiredSize = MAX_FORMAT_HEADER_SIZE;
              break;
            default:
              requiredSize = MIN_FORMAT_HEADER_SIZE;
              break;
        }

        //Check for file format and give validation a chance to request more data
        //if required buffer is not enough to sniff successfully. Maximum data that
        //can be requested is limited to max possible sniff buffer size.
        ssize_t dataRead = sniffBufferSize;
        do {
            //No need to read again until more data required
            if (!isEOS && requiredSize > dataRead) {
                ssize_t readBytes = source->readAt(offset + dataRead,
                                                   sniffBuffer + dataRead,
                                                   requiredSize - dataRead);
                if (readBytes <= 0)
                    break;
                isEOS = ((requiredSize - dataRead) != readBytes);
                dataRead += readBytes;
                LOGV("Data read for sniffing at offset %llu required %lu read %lu",
                     offset, requiredSize, dataRead);
            }

            //Check min sniff data is available
            if (dataRead < MIN_FORMAT_HEADER_SIZE) {
                LOGE("Sniff FAIL :: coundn't pull enough data for sniffing");
                index = SNIFF_ARRAY_SIZE;
                break;
            }

            requiredSize = dataRead;
            status = FileSource::CheckFileFormat(sniff->format, sniffBuffer, &requiredSize);
            LOGV("CheckFileFormat status = %d", status);
        }while (!isEOS && status == FILE_SOURCE_DATA_NOTAVAILABLE &&
                requiredSize <= MAX_FORMAT_HEADER_SIZE &&
                requiredSize > dataRead);

        sniffBufferSize = dataRead;
        if (status == FILE_SOURCE_SUCCESS) {
            LOGV(" Sniff %s success <==", sniff->type);
            sniffSuccess = true;
            *confidence = 0.8;
            break;
        }
    }
    free(sniffBuffer);
    return sniffSuccess ? sniffArray[index].creator : NULL;
}

extern "C" {

// This is the only symbol that needs to be exported
__attribute__ ((visibility ("default")))
MediaExtractor::ExtractorDef GETEXTRACTORDEF() {
    return {
        MediaExtractor::EXTRACTORDEF_VERSION,
        UUID("2cb1d938-46d8-436d-b73e-681c47392cc8"),
        1, // version
        "QCOM Extractor",
        Sniff
    };
}

} // extern "C"

} //namespace android
