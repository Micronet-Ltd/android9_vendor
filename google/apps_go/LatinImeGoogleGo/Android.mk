###############################################################################
# LatinImeGoogleGo
LOCAL_PATH := $(call my-dir)

my_archs := arm
my_src_arch := $(call get-prebuilt-src-arch,$(my_archs))

include $(CLEAR_VARS)
LOCAL_MODULE := LatinImeGoogleGo
LOCAL_MODULE_OWNER := google
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_PACKAGES := LatinIME
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_SRC_FILES := $(LOCAL_MODULE)_$(my_src_arch).apk
LOCAL_REQUIRED_MODULES := en_us_d3_20180105.dict
LOCAL_MODULE_TARGET_ARCH = $(my_src_arch)
LOCAL_DEX_PREOPT := true
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := en_us_d3_20180105.dict
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/usr/share/ime/google/d3_lms/
LOCAL_SRC_FILES := lms/main_en_us_d3_20180105.dict
include $(BUILD_PREBUILT)
