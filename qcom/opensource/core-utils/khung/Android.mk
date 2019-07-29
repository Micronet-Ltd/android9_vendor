LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_BUILD_VARIANT),userdebug)
# init.qti.khung.sh
include $(CLEAR_VARS)
LOCAL_MODULE       := init.qti.khung.sh
LOCAL_MODULE_TAGS  := debug
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES    := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
include $(BUILD_PREBUILT)
#######################################
# init-khung.rc
include $(CLEAR_VARS)

LOCAL_MODULE := khung.rc
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/init

include $(BUILD_PREBUILT)
#######################################
endif