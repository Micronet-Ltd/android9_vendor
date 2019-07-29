LOCAL_PATH:= $(call my-dir)

############## config file ##########################

include $(CLEAR_VARS)
LOCAL_MODULE := ta_config.json
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := ta_config.json
LOCAL_MODULE_TAGS := optional debug
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_ETC)/ssg
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := tz_whitelist.json
LOCAL_MODULE_CLASS := ETC
LOCAL_SRC_FILES := tz_whitelist.json
LOCAL_MODULE_TAGS := optional debug
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_ETC)/ssg
LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PREBUILT)
