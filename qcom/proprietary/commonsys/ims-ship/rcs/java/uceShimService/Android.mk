LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := whitelist_com.qualcomm.qti.uceShimService.xml
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc/sysconfig
LOCAL_SRC_FILES := config/whitelist_com.qualcomm.qti.uceShimService.xml
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional debug
LOCAL_PROGUARD_ENABLED := disabled

LOCAL_SRC_FILES := $(call all-subdir-java-files)


LOCAL_PRIVATE_PLATFORM_APIS := true
LOCAL_PACKAGE_NAME := uceShimService
LOCAL_CERTIFICATE := platform
LOCAL_MODULE_OWNER := qti
LOCAL_JAVA_LIBRARIES := android.hidl.base-V1.0-java android.hidl.manager-V1.0-java

LOCAL_STATIC_JAVA_LIBRARIES := com.qualcomm.qti.uceservice-V2.0-java

include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))
