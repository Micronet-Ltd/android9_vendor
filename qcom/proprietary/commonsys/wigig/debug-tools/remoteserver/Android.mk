LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := wigig_remoteserver

LOCAL_MODULE_TAGS := optional

LOCAL_CPPFLAGS := -Wall -lpthread -fexceptions -Wunused-parameter

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../lib/WlctPciAcss \
	$(LOCAL_PATH)/../lib/inc \
	$(LOCAL_PATH)/../lib/utils

LOCAL_SHARED_LIBRARIES := \
	libwigig_utils \
	libwigig_pciaccess

LOCAL_SRC_FILES := $(shell find $(LOCAL_PATH) -name '*.cpp' | sed s:^$(LOCAL_PATH)::g )
LOCAL_SRC_FILES += parser.l


include $(BUILD_EXECUTABLE)

