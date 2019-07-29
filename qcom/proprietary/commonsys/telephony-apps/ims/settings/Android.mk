ifeq ($(TARGET_USES_QCOM_BSP_ATEL), true)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_JAVA_LIBRARIES := telephony-common ims-common ims-ext-common

LOCAL_PACKAGE_NAME := imssettings
LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled
LOCAL_PRIVATE_PLATFORM_APIS := true

LOCAL_SRC_FILES := $(call all-java-files-under, src)
include $(BUILD_PACKAGE)
endif
