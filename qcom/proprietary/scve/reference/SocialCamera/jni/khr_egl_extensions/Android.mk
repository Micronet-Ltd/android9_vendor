LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := libsocialcamera_khr_egl_extensions
LOCAL_CFLAGS     := -Werror -fvisibility=hidden -fvisibility-inlines-hidden
LOCAL_SHARED_LIBRARIES := libEGL
LOCAL_SRC_FILES  := egl_khr_fence_sync.cpp

include vendor/qcom/proprietary/scve/reference/common.mk

include $(BUILD_SHARED_LIBRARY)
