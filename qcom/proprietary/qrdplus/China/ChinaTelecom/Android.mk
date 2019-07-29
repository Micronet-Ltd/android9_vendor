ifeq ($(strip $(TARGET_USES_QTIC_CT)),true)
GENERATED_PACKAGE_PATH := $(TARGET_OUT_VENDOR)/package
include $(call all-subdir-makefiles)
endif
