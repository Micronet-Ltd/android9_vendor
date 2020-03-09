###############################################################################
# GmsCore
LOCAL_PATH := $(call my-dir)

my_archs := arm arm64 x86 x86_64
my_src_arch := $(call get-prebuilt-src-arch, $(my_archs))

include $(CLEAR_VARS)
LOCAL_MODULE := GmsCore
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true
LOCAL_OVERRIDES_PACKAGES := NetworkRecommendation
ifeq ($(my_src_arch),arm)
LOCAL_DPI_VARIANTS := xxhdpi xhdpi hdpi
LOCAL_DPI_FILE_STEM := $(LOCAL_MODULE)_$(my_src_arch)_%.apk
else ifeq ($(my_src_arch),arm64)
LOCAL_DPI_VARIANTS := xxhdpi xhdpi
LOCAL_DPI_FILE_STEM := $(LOCAL_MODULE)_$(my_src_arch)_%.apk
endif
LOCAL_SRC_FILES := $(LOCAL_MODULE)_$(my_src_arch).apk
#LOCAL_REQUIRED_MODULES :=
ifeq ($(my_src_arch),arm64)
LOCAL_MULTILIB := both
else ifeq ($(my_src_arch),x86_64)
LOCAL_MULTILIB := both
endif
LOCAL_MODULE_TARGET_ARCH := $(my_src_arch)
include $(BUILD_PREBUILT)
