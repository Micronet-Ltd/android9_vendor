LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := wigig_logcollector

LOCAL_MODULE_TAGS := optional

LOCAL_CPPFLAGS := -Wall -fexceptions

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../lib/WlctPciAcss \
	$(LOCAL_PATH)/../lib/inc \
	$(LOCAL_PATH)/../lib/utils \

LOCAL_SHARED_LIBRARIES := \
	libwigig_utils \
	libwigig_pciaccess \

LOCAL_SRC_FILES := \
	LogCollector.cpp

include $(BUILD_EXECUTABLE)

# LogCollector configuration file

include $(CLEAR_VARS)
LOCAL_MODULE       := wigig_logcollector.ini
LOCAL_MODULE_TAGS  := optional
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_PATH  := $(TARGET_OUT_ETC)
LOCAL_SRC_FILES    := linux/wigig_logcollector.ini
include $(BUILD_PREBUILT)


