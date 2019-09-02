OV9281_CHROMATIX_SW_POST_PROC_PATH := $(call my-dir)

# ---------------------------------------------------------------------------
#                      Make the shared library (libchromatix_ov9281_sw_post_proc)
# ---------------------------------------------------------------------------

include $(CLEAR_VARS)
LOCAL_PATH := $(OV9281_CHROMATIX_SW_POST_PROC_PATH)
LOCAL_MODULE_TAGS := optional

LOCAL_LDFLAGS := $(mmcamera_debug_lflags)

LOCAL_CFLAGS := -DAMSS_VERSION=$(AMSS_VERSION) \
        $(mmcamera_debug_defines) \
        $(mmcamera_debug_cflags) \
        $(CHROMATRIX_LOCAL_CFLAGS) \
        -include camera_defs_i.h

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../../../../../../../../mm-camerasdk/sensor/includes/$(CHROMATIX_VERSION)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../module/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../../../../../../../common/
LOCAL_C_INCLUDES += chromatix_ov9281_sw_post_proc.h

LOCAL_SRC_FILES:= chromatix_ov9281_sw_post_proc.c

LOCAL_MODULE           := libchromatix_ov9281_postproc
LOCAL_SHARED_LIBRARIES := libcutils
include $(LOCAL_PATH)/../../../../../../../../local_additional_dependency.mk

ifeq ($(MM_DEBUG),true)
LOCAL_SHARED_LIBRARIES += liblog
endif

LOCAL_MODULE_OWNER := qti
LOCAL_PROPRIETARY_MODULE := true

ifeq ($(32_BIT_FLAG), true)
LOCAL_32_BIT_ONLY := true
endif

include $(BUILD_SHARED_LIBRARY)
