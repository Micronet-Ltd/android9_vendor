ifneq ($(TARGET_HAS_LOW_RAM), true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

ifeq (0,1)
LOCAL_MODULE_TAGS := debug

LOCAL_SRC_FILES := \
        $(call all-java-files-under, src)

LOCAL_PACKAGE_NAME := HiddTestApp
LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
endif
endif
