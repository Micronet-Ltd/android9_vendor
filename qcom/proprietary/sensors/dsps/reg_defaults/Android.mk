LOCAL_PATH := $(call my-dir)

# Only include this conf file in QCOM internal builds
ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
include $(CLEAR_VARS)
LOCAL_MODULE := sensor_def_qcomdev.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_CLASS = ETC
LOCAL_MODULE_PATH := $(TARGET_OUT_VENDOR_ETC)/sensors
ifneq (, $(filter msm8953_64_c801, $(TARGET_PRODUCT)))
LOCAL_SRC_FILES := sensor_def_qcomdev_c801.conf
else
LOCAL_SRC_FILES := sensor_def_qcomdev.conf
endif
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := sensors_settings
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := qcom
LOCAL_MODULE_CLASS = ETC
LOCAL_MODULE_PATH := $(PRODUCT_OUT)/persist/sensors
LOCAL_SRC_FILES := $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

endif
