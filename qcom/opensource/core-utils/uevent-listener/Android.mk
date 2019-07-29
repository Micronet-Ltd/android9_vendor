ifeq ($(call is-board-platform-in-list,trinket),true)
ifeq ($(TARGET_BUILD_VARIANT),userdebug)
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_SRC_FILES := uevent-listener.c
LOCAL_SHARED_LIBRARIES := libcutils liblog libutils
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_OWNER := qti
LOCAL_MODULE := uevent-listener
LOCAL_MODULE_TAGS := debug
include $(BUILD_EXECUTABLE)
#########init-uevent-listener
include $(CLEAR_VARS)
LOCAL_MODULE := uevent-listener.rc
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := debug
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_ETC)/init
include $(BUILD_PREBUILT)
endif
endif
