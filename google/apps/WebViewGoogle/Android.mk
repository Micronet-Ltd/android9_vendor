###############################################################################
# WebView Chromium
LOCAL_PATH := $(call my-dir)

my_archs := arm arm64 x86 x86_64
my_src_arch := $(call get-prebuilt-src-arch, $(my_archs))

include $(CLEAR_VARS)
LOCAL_MODULE := WebViewGoogle
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
#LOCAL_PRIVILEGED_MODULE :=
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_OVERRIDES_PACKAGES := webview
LOCAL_SRC_FILES := $(LOCAL_MODULE)_$(my_src_arch).apk
LOCAL_MULTILIB := both
LOCAL_REQUIRED_MODULES := \
    libwebviewchromium_loader \
    libwebviewchromium_plat_support
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := WebViewStub
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
#LOCAL_PRIVILEGED_MODULE :=
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_OVERRIDES_PACKAGES := webview
LOCAL_SRC_FILES := $(LOCAL_MODULE).apk
LOCAL_MULTILIB := both
LOCAL_REQUIRED_MODULES := \
    libwebviewchromium_loader \
    libwebviewchromium_plat_support
include $(BUILD_PREBUILT)
