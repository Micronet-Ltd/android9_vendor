###############################################################################
# Drive
LOCAL_PATH := $(call my-dir)

my_archs := arm arm64 x86 x86_64
my_src_arch := $(call get-prebuilt-src-arch, $(my_archs))

include $(CLEAR_VARS)
LOCAL_MODULE := Drive
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
#LOCAL_PRIVILEGED_MODULE :=
LOCAL_CERTIFICATE := PRESIGNED
#LOCAL_OVERRIDES_PACKAGES :=
LOCAL_DPI_VARIANTS := xxxhdpi xxhdpi xhdpi hdpi mdpi
LOCAL_DPI_FILE_STEM := $(LOCAL_MODULE)_$(my_src_arch)_%.apk
LOCAL_SRC_FILES := $(LOCAL_MODULE)_$(my_src_arch).apk
#LOCAL_REQUIRED_MODULES :=
LOCAL_MODULE_TARGET_ARCH := $(my_src_arch)
include $(BUILD_PREBUILT)
