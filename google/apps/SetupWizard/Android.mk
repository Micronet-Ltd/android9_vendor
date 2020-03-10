###############################################################################
# SetupWizard
LOCAL_PATH := $(call my-dir)

my_archs := arm arm64 x86 x86_64
my_src_arch := $(call get-prebuilt-src-arch, $(my_archs))

include $(CLEAR_VARS)
LOCAL_MODULE := SetupWizard
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true
LOCAL_OVERRIDES_PACKAGES := Provision
LOCAL_SRC_FILES := $(LOCAL_MODULE)_default.apk
#LOCAL_REQUIRED_MODULES :=
LOCAL_PREBUILT_JNI_LIBS := lib/$(my_src_arch)/libbarhopper.so
LOCAL_MODULE_TARGET_ARCH := $(my_src_arch)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := SearchSelector
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
include $(BUILD_PREBUILT)
