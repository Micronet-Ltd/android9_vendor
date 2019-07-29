ifneq ($(TARGET_NO_QTI_APP),true)
ifneq ($(TARGET_HAS_LOW_RAM), true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := debug
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/res
LOCAL_RESOURCE_DIR += frameworks/support/v7/appcompat/res
LOCAL_RESOURCE_DIR += frameworks/support/v7/recyclerview/res

LOCAL_AAPT_FLAGS := --auto-add-overlay

LOCAL_SRC_FILES := $(call all-java-files-under, src)
LOCAL_SRC_FILES += src/org/codeaurora/bluetooth/batestapp/IGattBroadcastServiceCallback.aidl
LOCAL_SRC_FILES += src/org/codeaurora/bluetooth/batestapp/IGattBroadcastService.aidl

LOCAL_AIDL_INCLUDES += system/bt/binder

LOCAL_STATIC_JAVA_LIBRARIES := \
    android-support-v4 \
    android-support-v7-recyclerview \
    android-support-v7-appcompat

LOCAL_PACKAGE_NAME := BATestApp
LOCAL_PRIVILEGED_MODULE := true
LOCAL_CERTIFICATE := platform

LOCAL_MODULE_OWNER := qti

LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PRIVATE_PLATFORM_APIS := true
include $(BUILD_PACKAGE)
endif
endif
