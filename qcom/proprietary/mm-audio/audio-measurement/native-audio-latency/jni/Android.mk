ifneq ($(BUILD_TINY_ANDROID),true)
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE    := libnative_audio_latency_jni

LOCAL_SRC_FILES := native-audio-latency-jni.c

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_GCOV)),true)
LOCAL_CFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_CPPFLAGS += --coverage -fprofile-arcs -ftest-coverage
LOCAL_STATIC_LIBRARIES += libprofile_rt
endif

LOCAL_C_INCLUDES := $(TOP)/system/media/wilhelm/include/
LOCAL_C_INCLUDES += $(TOP)/frameworks/wilhelm/include/
LOCAL_C_INCLUDES += $(TOP)/system/media/opensles/include/

LOCAL_SHARED_LIBRARIES := \
       libutils \
       libOpenSLES \
       libandroid \
       libcutils

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MULTILIB := 32

include $(BUILD_SHARED_LIBRARY)
endif #BUILD_TINY_ANDROID
