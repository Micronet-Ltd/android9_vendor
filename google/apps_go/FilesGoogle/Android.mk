###############################################################################
# FilesGoogle
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := FilesGoogle
LOCAL_MODULE_OWNER := google
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_CERTIFICATE := PRESIGNED
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_PRIVILEGED_MODULE := true
include $(BUILD_PREBUILT)

