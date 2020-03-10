#############################################################################
# GalleryGo
LOCAL_PATH := $(my-dir)

my_archs := arm
my_src_arch := $(call get-prebuilt-src-arch, $(my_archs))

include $(CLEAR_VARS)
LOCAL_MODULE:= GalleryGo
LOCAL_MODULE_OWNER := google
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
#LOCAL_PRIVILEGED_MODULE :=
LOCAL_PRODUCT_MODULE := true
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true
LOCAL_CERTIFICATE := PRESIGNED
LOCAL_DPI_VARIANTS := xxxhdpi xxhdpi xhdpi hdpi mdpi
LOCAL_DPI_FILE_STEM := $(LOCAL_MODULE)_$(my_src_arch)_%.apk
LOCAL_SRC_FILES := $(LOCAL_MODULE)_$(my_src_arch).apk
LOCAL_OVERRIDES_PACKAGES := Gallery2
#LOCAL_REQUIRED_MODULES :=
LOCAL_MODULE_TARGET_ARCH := $(my_src_arch)
include $(BUILD_PREBUILT)
