LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    fm_hci.cpp \
	../helium/radio_helium_hal.c \
	../helium/radio_helium_hal_cmds.c

LOCAL_SHARED_LIBRARIES := \
         libdl \
         libcutils \
         libbase \
         libhidlbase \
         libhidltransport \
         libhwbinder \
         liblog \
         libutils \
         android.hidl.base@1.0 \
         vendor.qti.hardware.fm@1.0 \

LOCAL_CFLAGS := -Wno-unused-parameter

LOCAL_C_INCLUDES += \
        $(LOCAL_PATH)/../helium \
        $(LOCAL_PATH)/fm_hci

LOCAL_MODULE := ftm_fm_lib
LOCAL_MODULE_TAGS := optional

LOCAL_MODULE_PATH_32 := $(TARGET_OUT_VENDOR)/lib
LOCAL_MODULE_PATH_64 := $(TARGET_OUT_VENDOR)/lib64
LOCAL_MODULE_TAGS := optional

include $(BUILD_SHARED_LIBRARY)
