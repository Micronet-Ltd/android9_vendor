LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_FLAGS:= -D_ANDROID_

ifeq ($(call is-vendor-board-platform,QCOM),true)
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_EXTN_FLAC_DECODER)),true)
    LOCAL_CFLAGS += -DQTI_FLAC_DECODER
endif
ifeq ($(strip $(AUDIO_FEATURE_ENABLED_FLAC_OFFLOAD)),true)
    LOCAL_CFLAGS += -DFLAC_OFFLOAD_ENABLED
endif
ifeq ($(TARGET_BOARD_PLATFORM),apq8098_latv)
    LOCAL_CFLAGS +=  -DENABLE_MP2_AUDIO_CODEC
endif
endif

LOCAL_SRC_FILES:=                 \
    src/MMParserExtractor.cpp     \
    src/QComExtractorFactory.cpp  \
    src/SourcePort.cpp            \

LOCAL_C_INCLUDES:=                                    \
    $(LOCAL_PATH)/inc                                 \
    $(LOCAL_PATH)/../../common/inc                    \
    $(TARGET_OUT_HEADERS)/mm-core/omxcore

LOCAL_HEADER_LIBRARIES :=       \
    libstagefright_headers      \
    libmmosal_headers           \
    libmmparser_headers

LOCAL_SHARED_LIBRARIES +=       \
    libstagefright              \
    libmmosal                   \
    libutils                    \
    libcutils                   \
    libmmparser_lite            \
    libmediaextractor           \
    libstagefright_foundation   \
    liblog

LOCAL_MODULE:= libmmparser

LOCAL_MODULE_RELATIVE_PATH := extractors

LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
