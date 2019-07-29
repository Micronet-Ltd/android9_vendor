LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := hal-server
LOCAL_INIT_RC := hal-server.rc
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := \
  hal-server.cpp \

LOCAL_SHARED_LIBRARIES := \
  libhidlbase \
  libhwbinder \
  libhidltransport \
  liblog \
  libutils \
  libhardware \
  libbinder

include $(BUILD_EXECUTABLE)
