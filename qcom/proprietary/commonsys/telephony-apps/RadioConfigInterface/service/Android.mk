#/******************************************************************************
#*@file Android.mk
#*brief Rules for compiling the source files
#*******************************************************************************/

LOCAL_PATH:= $(call my-dir)


ifneq ($(TARGET_HAS_LOW_RAM),true)

#  Build the interface files into a library
# for clients to use.

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := radioconfiginterfacelibrary

LOCAL_SRC_FILES := src/com/qualcomm/qti/radioconfiginterface/IRadioConfig.aidl
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/ILtedProvisioningListener.aidl
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/RadioConfigIntegerList.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/RadioConfigStringList.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/RadioConfigItem.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/RadioConfigParcelable.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/LtedConfig.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/LtedCategory.java
LOCAL_STATIC_JAVA_LIBRARIES += radioconfig-proto
LOCAL_PRIVATE_PLATFORM_APIS := true
include $(BUILD_JAVA_LIBRARY)


# ==========================================================================
#  Build the interface files into a library
# for clients to use.

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := radioconfiglibrary
LOCAL_JAVA_LIBRARIES := radioconfiginterfacelibrary

LOCAL_SRC_FILES := src/com/qualcomm/qti/radioconfiginterface/RadioConfig.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/RadioConfigItem.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/LtedConfig.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/LtedCategory.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/LtedProvisioningListener.java
LOCAL_PRIVATE_PLATFORM_APIS := true
include $(BUILD_JAVA_LIBRARY)

endif

# ==========================================================================

ifneq ($(TARGET_HAS_LOW_RAM),true)

# Build the service
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := debug
src_proto := src
LOCAL_SRC_FILES := src/com/qualcomm/qti/radioconfiginterface/RadioConfigService.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/RadioConfigSocket.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/RadioConfigMsgPacker.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/RadioConfigSyncRequest.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/RadioConfigBootReceiver.java
LOCAL_SRC_FILES += src/com/qualcomm/qti/radioconfiginterface/LtedAuthManager.java
LOCAL_SRC_FILES += $(call all-proto-files-under, $(src_proto))

LOCAL_PROTOC_OPTIMIZE_TYPE := micro

LOCAL_PACKAGE_NAME := radioconfig
LOCAL_CERTIFICATE := platform
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_JAVA_LIBRARIES := radioconfiglibrary
LOCAL_JAVA_LIBRARIES += radioconfiginterfacelibrary
LOCAL_PRIVATE_PLATFORM_APIS := true
include $(BUILD_PACKAGE)

# ==========================================================================

include $(CLEAR_VARS)

LOCAL_MODULE := radioconfig.xml
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE_CLASS := ETC

#this will install the file in /system/etc/permissions
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

# ==========================================================================

include $(CLEAR_VARS)

LOCAL_MODULE := radioconfiginterface.xml
LOCAL_MODULE_TAGS := debug

LOCAL_MODULE_CLASS := ETC

#this will install the file in /system/etc/permissions
LOCAL_MODULE_PATH := $(TARGET_OUT_ETC)/permissions

LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

endif
