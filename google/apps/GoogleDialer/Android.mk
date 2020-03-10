###############################################################################
# GoogleDialer
LOCAL_PATH := $(call my-dir)

my_archs := arm arm64
my_src_arch := $(call get-prebuilt-src-arch, $(my_archs))

include $(CLEAR_VARS)
LOCAL_MODULE := GoogleDialer
LOCAL_MODULE_CLASS := APPS
LOCAL_MODULE_TAGS := optional
LOCAL_BUILT_MODULE_STEM := package.apk
LOCAL_MODULE_SUFFIX := $(COMMON_ANDROID_PACKAGE_SUFFIX)
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := PRESIGNED
DONT_UNCOMPRESS_PRIV_APPS_DEXS := true
LOCAL_OVERRIDES_PACKAGES := Dialer
LOCAL_DPI_VARIANTS := xxxhdpi xxhdpi xhdpi hdpi mdpi
LOCAL_DPI_FILE_STEM := $(LOCAL_MODULE)_$(my_src_arch)_%.apk
LOCAL_SRC_FILES := $(LOCAL_MODULE)_$(my_src_arch).apk
LOCAL_REQUIRED_MODULES := com.google.android.dialer.support.jar
#LOCAL_PREBUILT_JNI_LIBS :=
LOCAL_MODULE_TARGET_ARCH := $(my_src_arch)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := com.google.android.dialer.support.jar
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := JAVA_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_JAVA_LIBRARIES)
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_REQUIRED_MODULES := com.google.android.dialer.support.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := com.google.android.dialer.support.xml
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)
