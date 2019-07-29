ifeq ($(strip $(TARGET_USES_QTIC_CU)),true)
GENERATED_PACKAGE_PATH := $(TARGET_OUT_VENDOR)/package
include $(call all-subdir-makefiles)
endif
