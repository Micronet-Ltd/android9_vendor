#/******************************************************************************
#*@file Android.mk
#*brief Rules for compiling the source files
#*******************************************************************************/

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := $(call all-subdir-java-files)

LOCAL_STATIC_JAVA_LIBRARIES := iris

LOCAL_PACKAGE_NAME := IrisEnrollDemo
LOCAL_CERTIFICATE := platform
#LOCAL_JACK_ENABLED := disabled
LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_PACKAGE)

include $(call all-makefiles-under,$(LOCAL_PATH))

