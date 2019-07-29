LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:=oemcrypto_test
LOCAL_MODULE_TAGS := tests

LOCAL_MODULE_OWNER := widevine
LOCAL_PROPRIETARY_MODULE := true

# When built, explicitly put it in the DATA/bin directory.
LOCAL_MODULE_PATH := $(TARGET_OUT_DATA)/bin

ifneq ($(TARGET_ENABLE_MEDIADRM_64), true)
LOCAL_MODULE_TARGET_ARCH := arm x86 mips
endif

include $(LOCAL_PATH)/common.mk

include $(BUILD_EXECUTABLE)
