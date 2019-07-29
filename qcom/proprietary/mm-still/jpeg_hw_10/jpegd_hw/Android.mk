ifneq ($(BUILD_TINY_ANDROID),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

#===============================================================================
#             Deploy the headers that can be exposed
#===============================================================================

LOCAL_COPY_HEADERS_TO := mm-still/jpeg_hw_10
LOCAL_COPY_HEADERS += inc/jpegd_lib.h

#===============================================================================
#             Compile Shared library libjpegdhw.so
#===============================================================================

LOCAL_C_INCLUDES := $(LOCAL_PATH)/inc
LOCAL_C_INCLUDES += hardware/qcom/camera
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../jpeg2/inc

ifeq ($(TARGET_COMPILE_WITH_MSM_KERNEL),true)
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_ADDITIONAL_DEPENDENCIES := $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
else
LOCAL_C_INCLUDES += bionic/libc/kernel/common/media
endif

ifeq ($(strip $(TARGET_USES_ION)),true)
LOCAL_CFLAGS += -DUSE_ION
endif

LOCAL_CFLAGS += -D_ANDROID_

LOCAL_CFLAGS += -Werror

LOCAL_SRC_FILES := src/jpegd_lib.c
LOCAL_SRC_FILES +=  src/jpegd_lib_hw.c

LOCAL_SHARED_LIBRARIES := libcutils liblog
LOCAL_MODULE := libjpegdhw
LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_HEADER_LIBRARIES := libutils_headers

#LOCAL_CLANG := false

LOCAL_MODULE_OWNER := qcom
LOCAL_PROPRIETARY_MODULE := true

LOCAL_32_BIT_ONLY := true
include $(BUILD_SHARED_LIBRARY)

endif #BUILD_TINY_ANDROID
