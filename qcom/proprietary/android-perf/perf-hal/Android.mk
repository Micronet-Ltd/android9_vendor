LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := vendor.qti.hardware.perf@1.0-service
LOCAL_INIT_RC := vendor.qti.hardware.perf@1.0-service.rc

LOCAL_SRC_FILES := \
        service.cpp Perf.cpp

LOCAL_C_INCLUDES := \
        vendor/qcom/proprietary/android-perf/mp-ctl

LOCAL_HEADER_LIBRARIES := libcutils_headers

LOCAL_SHARED_LIBRARIES := \
        liblog \
        libcutils \
        libdl \
        libbase \
        libutils \
        libhardware_legacy \
        libhardware \
        libhwbinder \
        libhidlbase \
        libhidltransport \
        vendor.qti.hardware.perf@1.0 \
        libperfgluelayer

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -DENABLE_BINDER_BUFFER_TUNING_FOR_32_BIT
endif

ifeq ($(TARGET_USES_LM),true)
    LOCAL_CFLAGS += -DTARGET_USES_LM
endif

include $(BUILD_EXECUTABLE)
