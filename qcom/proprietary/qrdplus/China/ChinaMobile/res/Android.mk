# Disable include all subdir makefiles,  If any one module can normal compile, please include module makefile in here.
#include $(call all-subdir-makefiles)

BASE_PATH := $(call my-dir)
include $(BASE_PATH)/Mms/Android.mk
include $(BASE_PATH)/SystemUI/Android.mk
include $(BASE_PATH)/Settings/Android.mk