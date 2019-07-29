###############################################################################
# Velvet
LOCAL_PATH := $(call my-dir)

my_archs := arm arm64 x86
my_src_arch := $(call get-prebuilt-src-arch, $(my_archs))

OFFLINE_LANG_PACK_FILE_LIST := $(patsubst $(LOCAL_PATH)/OfflineVoiceRecognitionLanguagePacks/%,%,$(shell find $(LOCAL_PATH)/OfflineVoiceRecognitionLanguagePacks -type f))

define install-lang-pack-files
include $$(CLEAR_VARS)
LOCAL_MODULE := $(1)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $$(TARGET_OUT)/usr/srec
LOCAL_SRC_FILES := OfflineVoiceRecognitionLanguagePacks/$$(LOCAL_MODULE)
include $$(BUILD_PREBUILT)
endef

$(foreach fp, $(OFFLINE_LANG_PACK_FILE_LIST),\
  $(eval $(call install-lang-pack-files, $(fp))))

include $(CLEAR_VARS)
LOCAL_MODULE := Velvet
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_OVERRIDES_PACKAGES := QuickSearchBox
ifeq ($(my_src_arch),arm)
LOCAL_DPI_VARIANTS := xhdpi hdpi mdpi
LOCAL_DPI_FILE_STEM := $(LOCAL_MODULE)_$(my_src_arch)_%.apk
endif
LOCAL_SRC_FILES := $(LOCAL_MODULE)_$(my_src_arch).apk
LOCAL_REQUIRED_MODULES := $(OFFLINE_LANG_PACK_FILE_LIST)
LOCAL_MODULE_TARGET_ARCH := $(my_src_arch)
include $(BUILD_PREBUILT)
