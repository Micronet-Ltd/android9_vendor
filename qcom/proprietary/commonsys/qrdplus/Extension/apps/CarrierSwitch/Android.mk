ifneq ($(TARGET_NO_QTI_APP),true)
ifeq (1,1)
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

# Only compile source java files in this apk.
LOCAL_SRC_FILES := $(call all-java-files-under, src) \
       src/com/qualcomm/qti/carrierswitch/ICarrierSwitchDeamonService.aidl

LOCAL_AIDL_INCLUDES := src/com/qualcomm/qti/carrierswitch/CarrierInfo.aidl

LOCAL_PACKAGE_NAME := CarrierSwitch

LOCAL_PRIVILEGED_MODULE := true
LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_CERTIFICATE := platform

LOCAL_PROGUARD_ENABLED := disabled

include $(BUILD_PACKAGE)
endif
endif
