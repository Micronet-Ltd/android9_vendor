###############################################################################
# GoogleTTS
LOCAL_PATH := $(call my-dir)

my_archs := arm x86 arm64 x86_64
my_src_arch := $(call get-prebuilt-src-arch, $(my_archs))

include $(CLEAR_VARS)
LOCAL_MODULE := GoogleTTS
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
#LOCAL_PRIVILEGED_MODULE :=
LOCAL_CERTIFICATE := PRESIGNED
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true
LOCAL_OVERRIDES_PACKAGES := PicoTts
LOCAL_SRC_FILES := $(LOCAL_MODULE)_$(my_src_arch).apk
#LOCAL_REQUIRED_MODULES :=
LOCAL_MODULE_TARGET_ARCH := $(my_src_arch)
include $(BUILD_PREBUILT)
