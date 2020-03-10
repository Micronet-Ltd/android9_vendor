###############################################################################
# GmsCoreGo
LOCAL_PATH := $(call my-dir)

my_archs := arm arm64
my_src_arch := $(call get-prebuilt-src-arch, $(my_archs))

include $(CLEAR_VARS)
LOCAL_MODULE := GmsCoreGo
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true
LOCAL_OVERRIDES_PACKAGES := NetworkRecommendation
ifeq ($(my_src_arch),arm)
LOCAL_DPI_VARIANTS := xhdpi hdpi mdpi
LOCAL_DPI_FILE_STEM := $(LOCAL_MODULE)_$(my_src_arch)_%.apk
else ifeq ($(my_src_arch),arm64)
LOCAL_MULTILIB := both
endif
LOCAL_SRC_FILES := $(LOCAL_MODULE)_$(my_src_arch).apk
#LOCAL_REQUIRED_MODULES :=
LOCAL_MODULE_TARGET_ARCH := $(my_src_arch)
include $(BUILD_PREBUILT)
