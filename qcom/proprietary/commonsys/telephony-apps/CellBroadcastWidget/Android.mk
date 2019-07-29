ifeq (0,1)
ifneq ($(TARGET_NO_TELEPHONY), true)
ifneq ($(TARGET_HAS_LOW_RAM),true)

ifeq ($(TARGET_USES_QCOM_BSP_ATEL), true)
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_PACKAGE_NAME := CellBroadcastWidget
LOCAL_CERTIFICATE := platform

include $(BUILD_PACKAGE)
endif
endif
endif
endif # TARGET_NO_TELEPHONY
