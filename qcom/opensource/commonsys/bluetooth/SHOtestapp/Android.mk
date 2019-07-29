LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/res
LOCAL_RESOURCE_DIR += frameworks/support/v7/appcompat/res
LOCAL_RESOURCE_DIR += frameworks/support/v7/recyclerview/res
LOCAL_RESOURCE_DIR += frameworks/support/v7/cardview/res


LOCAL_AAPT_FLAGS := --auto-add-overlay

LOCAL_SRC_FILES := \
        $(call all-java-files-under, java)
        
LOCAL_PACKAGE_NAME := BTAudioTestApp
LOCAL_CERTIFICATE := platform

LOCAL_MODULE_OWNER := qti

LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)

