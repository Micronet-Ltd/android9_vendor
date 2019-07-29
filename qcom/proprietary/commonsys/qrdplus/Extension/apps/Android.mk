# Disable include all subdir makefiles, If any one module can normal compile, please include module makefile in here.
#include $(call all-subdir-makefiles)

BASE_PATH := $(call my-dir)

#include $(BASE_PATH)/CarrierSwitch/Android.mk
include $(BASE_PATH)/WapPush/Android.mk
include $(BASE_PATH)/CameraExtension/Android.mk
