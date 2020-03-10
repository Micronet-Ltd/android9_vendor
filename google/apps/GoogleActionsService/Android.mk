###############################################################################
# GoogleActionsService
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := GoogleActionsService
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := google
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk

include $(BUILD_PREBUILT)
###############################################################################
