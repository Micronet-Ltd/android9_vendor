LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES:= \
    android_hardware_wipower.cpp \

LOCAL_C_INCLUDES += \
    $(JNI_H_INCLUDE) \
        packages/apps/Bluetooth/jni \
        vendor/qcom/opensource/bluetooth_ext/vhal/include \
        vendor/qcom/opensource/bluetooth/hal/include \
        system/bt/types/ \
        libnativehelper/include/nativehelper


LOCAL_REQUIRED_MODULES := bluetooth.default

LOCAL_PROGUARD_ENABLED := disabled


LOCAL_SHARED_LIBRARIES := \
        libnativehelper \
        libandroid_runtime \
        libcutils \
        liblog \
        libhardware

LOCAL_JNI_SHARED_LIBRARIES := libbluetooth_jni

LOCAL_MODULE := libwipower_jni
include $(BUILD_SHARED_LIBRARY)
