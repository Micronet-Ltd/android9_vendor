#############################################################################
# GoogleDialerGo
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= GoogleDialerGo
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_CERTIFICATE := PRESIGNED
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true
LOCAL_DPI_VARIANTS := xxxhdpi xxhdpi xhdpi hdpi mdpi
LOCAL_DPI_FILE_STEM := $(LOCAL_MODULE)_%.apk
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_REQUIRED_MODULES := googledialergo-sysconfig.xml
LOCAL_PRIVILEGED_MODULE := true
LOCAL_OVERRIDES_PACKAGES := Dialer
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := googledialergo-sysconfig.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/sysconfig/
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
