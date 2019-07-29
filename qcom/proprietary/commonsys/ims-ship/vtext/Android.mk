LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := libimsvtext_headers
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)/inc/
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)

#Header LIBRARY
LOCAL_HEADER_LIBRARIES := libimsvtext_headers
LOCAL_HEADER_LIBRARIES += display_headers
LOCAL_HEADER_LIBRARIES += media_plugin_headers
LOCAL_HEADER_LIBRARIES += libcutils_headers
LOCAL_HEADER_LIBRARIES += liblog_headers


# Local library and header dependencies
LOCAL_SHARED_LIBRARIES := lib-imsvtutils
LOCAL_SHARED_LIBRARIES += lib-imsvtextutils
LOCAL_C_INCLUDES := $(LOCAL_PATH)/src/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/inc/

# Diag and log permission dependencies
LOCAL_SHARED_LIBRARIES += libcutils
LOCAL_SHARED_LIBRARIES += libutils
LOCAL_SHARED_LIBRARIES += liblog
LOCAL_SHARED_LIBRARIES += libdiag_system
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/diag/include
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/common/inc

# These are needed for nativeWindow and Surface
LOCAL_SHARED_LIBRARIES += libgui
LOCAL_SHARED_LIBRARIES += libandroid

#these are not needed for SDM845 onwards hence
#putting under flag
ifeq ($(call is-board-platform-in-list,msm8909 msm8953 msm8937 msm8952 msm8976 msm8996 msm8998 sdm660),true)
LOCAL_SHARED_LIBRARIES += libcamera_client
LOCAL_SHARED_LIBRARIES += libstagefright_foundation
LOCAL_SHARED_LIBRARIES += libqdMetaData.system
LOCAL_SHARED_LIBRARIES += libstagefright
LOCAL_SHARED_LIBRARIES += \
    libdl \
    libbase \
    libhardware_legacy \
    libhardware \
    libhidlbase \
    libhidlmemory \
    android.hidl.allocator@1.0 \
    android.hidl.memory@1.0 \
    android.hardware.media.omx@1.0 \
    android.hardware.camera.common@1.0 \
    android.hardware.camera.provider@2.4 \
    android.hardware.camera.device@1.0 \
    vendor.qti.hardware.camera.device@1.0 \
    android.hardware.camera.device@3.2
ifeq ($(DEFAULT_PLATFORM_VERSION),OPR1)
LOCAL_SHARED_LIBRARIES += android.hidl.manager@1.0
endif
LOCAL_SHARED_LIBRARIES += \
    libhwbinder \
    libhidlbase \
    libhidltransport

LOCAL_SHARED_LIBRARIES += libnativehelper
LOCAL_SHARED_LIBRARIES += libandroid_runtime
LOCAL_SHARED_LIBRARIES += libui
LOCAL_SHARED_LIBRARIES += libmedia
LOCAL_SHARED_LIBRARIES += libmedia_omx

# GPU libraries for rotation
LOCAL_SHARED_LIBRARIES += libbinder
LOCAL_SHARED_LIBRARIES += libGLESv2
LOCAL_SHARED_LIBRARIES += libEGL
#EGL
LOCAL_C_INCLUDES += vendor/qcom/proprietary/gles/adreno200/include/public/GLES2/
LOCAL_C_INCLUDES += vendor/qcom/proprietary/gles/adreno200/include/public/GLES3/
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/gui
LOCAL_C_INCLUDES += vendor/qcom/proprietary/gles/adreno200/opengl/esx/shared/include/public/EGL/
LOCAL_C_INCLUDES += $(TOP)/system/core/include/utils
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/opengl/include/EGL/


LOCAL_C_INCLUDES += $(TOP)/system/media/camera/include
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/media/hardware
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/media/openmax
LOCAL_C_INCLUDES +=  $(TARGET_OUT_HEADERS)/qcom/display
LOCAL_C_INCLUDES += $(TARGET_OUT_HEADERS)/mm-core/omxcore/
LOCAL_C_INCLUDES += $(TOP)/frameworks/base/include/android_runtime

# Src files speciic to IOMX
LOCAL_SRC_FILES += src/VideoOmx.cpp
LOCAL_SRC_FILES += src/VideoReceiver.cpp
LOCAL_SRC_FILES += src/VideoTransmitter.cpp

else #For mediaCodec interface

# MediaCodec Library
LOCAL_SHARED_LIBRARIES += libmediandk
LOCAL_C_INCLUDES += $(TOP)/frameworks/native/include/ndk

# Files that need to be compiled only for MEDIA_CODEC support
LOCAL_SRC_FILES += src/VideoCodecEncoder.cpp
LOCAL_SRC_FILES += src/VideoCodecDecoder.cpp
LOCAL_SRC_FILES += src/MediaCodecBase.cpp
endif #sdm845 ifdef

# Common files for all targets
LOCAL_SRC_FILES += src/VideoCodecBase.cpp
LOCAL_SRC_FILES += src/VTPlayer.cpp
LOCAL_SRC_FILES += src/VTRecorder.cpp
LOCAL_SRC_FILES += src/qpVideo.cpp

# AV Timer related dependencies
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/techpack/audio/include
LOCAL_C_INCLUDES += $(TARGET_OUT_INTERMEDIATES)/vendor/qcom/opensource/audio-kernel/include
LOCAL_ADDITIONAL_DEPENDENCIES += $(BOARD_VENDOR_KERNEL_MODULES)

ifeq ($(strip $(AUDIO_FEATURE_ENABLED_DLKM)),true)
  LOCAL_HEADER_LIBRARIES := audio_kernel_headers
endif

ifeq ($(call is-board-platform-in-list,msm8909 msm8953 msm8937 msm8952 msm8976  msm8996 msm8998 sdm660),true)
LOCAL_C_INCLUDES += $(TOP)/frameworks/av/services/camera/libcameraservice/
LOCAL_C_INCLUDES += $(TOP)/hardware/libhardware/include/hardware/
LOCAL_C_INCLUDES += $(QC_PROP_ROOT)/commonsys/ims-ship/imscamera/inc
LOCAL_C_INCLUDES += frameworks/av/include/camera/android/hardware
LOCAL_C_INCLUDES += frameworks/av/include/camera
LOCAL_C_INCLUDES += frameworks/av/services/camera/libcameraservice/
LOCAL_C_INCLUDES += frameworks/av/services/camera/libcameraservice/device1/
LOCAL_C_INCLUDES += hardware/libhardware/include/hardware/
#HAL1 linkage is commented
LOCAL_SHARED_LIBRARIES += lib-imscamera
endif

ifeq ($(call is-board-platform-in-list,msm8909),true)
LOCAL_CPPFLAGS :=  -DHAL1
LOCAL_CFLAGS :=  -DHAL1
endif

ifneq ($(call is-board-platform-in-list,msm8909 msm8953 msm8937 msm8952 msm8976 msm8996),true)
LOCAL_CFLAGS +=  -DGPU_ROTATION
LOCAL_CPPFLAGS +=  -DGPU_ROTATION
endif

ifeq ($(DEFAULT_PLATFORM_VERSION),OPR1)
LOCAL_CFLAGS += -DANDROIDOPR1
LOCAL_CPPFLAGS += -DANDROIDOPR1
endif

ifeq ($(call is-board-platform-in-list,msm8909 msm8909_512),true)
LOCAL_CFLAGS +=  -DMSM8909
LOCAL_CPPFLAGS +=  -DMSM8909
endif

ifeq ($(call is-board-platform,msm8937),true)
LOCAL_CFLAGS +=  -DMSM8937
LOCAL_CPPFLAGS +=  -DMSM8937
endif

ifneq ($(call is-board-platform-in-list,msm8909 msm8953 msm8937 msm8952 msm8976 msm8996 msm8998 sdm660),true)
LOCAL_CFLAGS +=  -DMEDIA_CODEC
LOCAL_CPPFLAGS +=  -DMEDIA_CODEC
endif

LOCAL_MODULE:= lib-imsvideocodec
LOCAL_CLANG := true

LOCAL_MODULE_TAGS:=optional

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)
