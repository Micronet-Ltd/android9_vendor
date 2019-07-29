ifeq ($(TARGET_USES_QCOM_BSP_ATEL), true)
ifneq ($(TARGET_NO_TELEPHONY), true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional eng

LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_RESOURCE_DIR := $(LOCAL_PATH)/res
LOCAL_JAVA_LIBRARIES := telephony-common

LOCAL_PACKAGE_NAME := CallFeaturesSetting

LOCAL_CERTIFICATE := platform
LOCAL_PRIVATE_PLATFORM_APIS := true

include $(BUILD_PACKAGE)
endif
endif # TARGET_NO_TELEPHONY
